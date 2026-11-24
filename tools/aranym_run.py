#!/usr/bin/env python3
"""Run Realtimer in MacAranym (ARAnyM for macOS).

ARAnyM boots EmuTOS from a small FAT16 IDE image, build/aranym/drive_c.img,
which this script creates and fills with the program, its resources and
setups. The image is mounted on the Mac only while the script copies files,
never while ARAnyM runs. Each launch refreshes the program files; SETUPS and
INFOS are copied when the image is created or with --reset, so changes made
inside ARAnyM survive later launches.

    tools/aranym_run.py                 build, sync and run
    tools/aranym_run.py --no-build      run the current realtim5.prg
    tools/aranym_run.py --reset         recreate the image from the working tree
    tools/aranym_run.py --sync-only     update the image without starting ARAnyM

ARAnyM never runs MFP Timer A and has no MIDI input or ACIA transmit
interrupt. midikern.c detects ARAnyM through its _MCH cookie and switches to
a Timer C clock and polled MIDI output, which ARAnyM writes to
build/aranym/midi-out.bin.

EmuTOS only offers Videl modes (640x480 at most on VGA). For a larger GEM
screen the image also gets fVDI with ARAnyM's NatFeat driver (aranym.sys),
which supports any size in 8, 16 or 32 bits. By default the screen fills the
host's usable screen area; --screen vga keeps EmuTOS's own 640x480 mono mode.

With --hostfs the image only boots EmuTOS: BetaDOS and ARAnyM's hostfs.dos
in its AUTO folder mount the workspace itself as drive H:, and the desktop
starts H:\\REALTIM5.PRG. No copying is needed after a build, but Realtimer
then saves setups directly into the working tree, as under Hatari's GEMDOS
drive.
"""

import argparse
import glob
import os
import platform
import plistlib
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
WORKSPACE = os.path.dirname(HERE)
OUT = os.path.join(WORKSPACE, "build", "aranym")
IMAGE = os.path.join(OUT, "drive_c.img")
CONFIG = os.path.join(OUT, "config")
MIDI_OUT = os.path.join(OUT, "midi-out.bin")
LOG = os.path.join(OUT, "aranym.log")

# IDE geometry for a 64 MB disk; ARAnyM needs it to match the image size.
CYLINDERS, HEADS, SECTORS = 130, 16, 63
IMAGE_SIZE = CYLINDERS * HEADS * SECTORS * 512

DEFAULT_EMUTOS = os.path.expanduser("~/Documents/aranym_files/emutos-aranym.img")
DEFAULT_DRIVERS = os.path.expanduser("~/Documents/aranym_files/drive_c")
FVDI_FILES = {"AUTO/FVDI.PRG": "auto/fvdi.prg", "GEMSYS/ARANYM.SYS": "gemsys/aranym.sys"}
HOSTFS_FILES = {"AUTO/BETADOS.PRG": "auto/betados.prg", "AUTO/HOSTFS.DOS": "auto/hostfs.dos"}
HOSTFS_DRIVE = "H"
BDCONFIG = f"*DOS, C:\\AUTO\\HOSTFS.DOS, {HOSTFS_DRIVE}:{HOSTFS_DRIVE}\r\n".encode()
# Height of the macOS title bar above ARAnyM's screen, in points.
TITLE_BAR = 28
PROGRAM_FILES = ["realtim5.rsc", "realtim5.err", "*.RSC", "INTRFACE.INF"]
DATA_DIRS = ["SETUPS", "INFOS", "CLIPBRD"]
MAC_JUNK = [".fseventsd", ".Spotlight-V100", ".Trashes", ".TemporaryItems"]


def find_app():
    """The JIT build on Intel Macs, the universal interpreter on Apple Silicon
    (the JIT app is x86_64-only)."""
    names = ["MacAranym JIT.app", "MacAranym.app"]
    if platform.machine() == "arm64":
        names.reverse()
    for name in names:
        for path in sorted(glob.glob("/Applications/MacAranym*/" + name)) + \
                ["/Applications/" + name]:
            if "_orig" not in path and os.path.isdir(path):
                return path
    return None


def app_binary(app):
    with open(os.path.join(app, "Contents", "Info.plist"), "rb") as f:
        exe = plistlib.load(f)["CFBundleExecutable"]
    return os.path.join(app, "Contents", "MacOS", exe)


def run(cmd, **kw):
    return subprocess.run(cmd, check=True, capture_output=True, text=True, **kw).stdout


def image_in_use():
    r = subprocess.run(["lsof", "-t", IMAGE], capture_output=True, text=True)
    return bool(r.stdout.strip())


def attach(mount):
    args = ["hdiutil", "attach", "-imagekey", "diskimage-class=CRawDiskImage", "-plist"]
    args += ["-nobrowse", "-noverify", "-mountrandom", "/tmp"] if mount else ["-nomount"]
    info = plistlib.loads(run(args + [IMAGE]).encode())
    ents = info["system-entities"]
    dev = min((e["dev-entry"] for e in ents), key=len)
    points = [e["mount-point"] for e in ents if "mount-point" in e]
    return dev, (points[0] if points else None)


def create_image():
    os.makedirs(OUT, exist_ok=True)
    with open(IMAGE, "wb") as f:
        f.truncate(IMAGE_SIZE)
    dev, _ = attach(mount=False)
    try:
        run(["diskutil", "partitionDisk", dev, "1", "MBR", "MS-DOS FAT16", "RTM", "100%"])
    finally:
        subprocess.run(["hdiutil", "detach", dev], capture_output=True)


def emudesk_inf(autorun):
    """The working tree's EMUDESK.INF with an autorun entry. The #E line is
    dropped: its saved resolution is an ST one, ARAnyM boots in VGA modes.
    The AES only scans the first 300 bytes for #Z, so it goes first."""
    with open(os.path.join(WORKSPACE, "EMUDESK.INF"), "rb") as f:
        lines = f.read().replace(b"\r\n", b"\n").split(b"\n")
    lines = [l for l in lines if l and not l.startswith((b"#E", b"#Z"))]
    lines.insert(0, b"#Z 01 " + autorun.encode() + b"@")
    return b"\r\n".join(lines) + b"\r\n"


def hostfs_path(prg):
    """Atari path of prg on the workspace drive."""
    rel = os.path.relpath(os.path.abspath(os.path.join(WORKSPACE, prg)), WORKSPACE)
    if rel.startswith(".."):
        sys.exit(f"--prg {prg} is outside the workspace, which --hostfs mounts as "
                 f"{HOSTFS_DRIVE}:")
    return f"{HOSTFS_DRIVE}:\\" + rel.replace("/", "\\").upper()


def host_screen():
    """Usable area of the Mac's main screen in points (no menu bar or Dock)."""
    js = ("ObjC.import('AppKit'); var f = $.NSScreen.mainScreen.visibleFrame;"
          "f.size.width + 'x' + f.size.height")
    try:
        w, h = run(["osascript", "-l", "JavaScript", "-e", js]).strip().split("x")
        return int(float(w)), int(float(h))
    except (subprocess.CalledProcessError, ValueError, OSError):
        return None


def screen_size(spec):
    """None for EmuTOS's VGA mode, else (width, height) for fVDI."""
    if spec == "vga":
        return None
    if spec == "auto":
        host = host_screen()
        if not host:
            print("Could not read the host screen size; using 1280x960")
            return 1280, 960
        w, h = host[0], host[1] - TITLE_BAR
    else:
        try:
            w, h = (int(v) for v in spec.lower().split("x"))
        except ValueError:
            sys.exit(f"--screen: expected auto, vga or WIDTHxHEIGHT, not {spec!r}")
    # Multiples of 16 keep lines word-aligned.
    w, h = w // 16 * 16, h // 16 * 16
    if w < 640 or h < 400:
        sys.exit(f"--screen: {w}x{h} is smaller than 640x400")
    return w, h


def find_files(src, mapping):
    """Map image paths to files in src, matching names case-insensitively;
    None if any is missing."""
    found = {}
    for dst, rel in mapping.items():
        path = src
        for part in rel.split("/"):
            names = {n.lower(): n for n in os.listdir(path)} if os.path.isdir(path) else {}
            path = os.path.join(path, names.get(part, part))
        if not os.path.isfile(path):
            return None
        found[dst] = path
    return found


def fvdi_sys(size):
    w, h = size
    return (f"PATH = C:\\GEMSYS\r\nbooted\r\n"
            f"01r aranym.sys mode {w}x{h}x8@60 irq accelerate xxxxxxx\r\n").encode()


def copy_tree(src, dst):
    os.makedirs(dst, exist_ok=True)
    for name in os.listdir(src):
        if name.startswith("."):
            continue
        s, d = os.path.join(src, name), os.path.join(dst, name)
        if os.path.isdir(s):
            copy_tree(s, d)
        else:
            shutil.copyfile(s, d)


def install(mnt, files, names, extra):
    """Copy files (image path -> source) and write extra (image path -> bytes)
    to the mounted image; with files None, remove names and extra from it."""
    if files:
        for dst, src in files.items():
            os.makedirs(os.path.join(mnt, os.path.dirname(dst)), exist_ok=True)
            shutil.copyfile(src, os.path.join(mnt, dst))
        for dst, data in extra.items():
            with open(os.path.join(mnt, dst), "wb") as f:
                f.write(data)
        return
    for dst in list(names) + list(extra):
        if os.path.exists(os.path.join(mnt, dst)):
            os.remove(os.path.join(mnt, dst))


def sync(prg, with_data, fvdi, size, hostfs):
    dev, mnt = attach(mount=True)
    try:
        if hostfs:
            autorun = hostfs_path(prg)
            os.makedirs(os.path.join(mnt, "CLIPBRD"), exist_ok=True)
            # An old copy on C: must not be mistaken for the current build.
            if os.path.exists(os.path.join(mnt, "REALTIM5.PRG")):
                os.remove(os.path.join(mnt, "REALTIM5.PRG"))
        else:
            autorun = "C:\\REALTIM5.PRG"
            shutil.copyfile(os.path.join(WORKSPACE, prg), os.path.join(mnt, "REALTIM5.PRG"))
            for pattern in PROGRAM_FILES:
                for src in glob.glob(os.path.join(WORKSPACE, pattern)):
                    shutil.copyfile(src, os.path.join(mnt, os.path.basename(src).upper()))
        with open(os.path.join(mnt, "EMUDESK.INF"), "wb") as f:
            f.write(emudesk_inf(autorun))
        install(mnt, hostfs, HOSTFS_FILES, {"AUTO/BDCONFIG.SYS": BDCONFIG})
        install(mnt, fvdi if size else None, FVDI_FILES,
                {"FVDI.SYS": fvdi_sys(size) if size else b""})
        if with_data and not hostfs:
            for d in DATA_DIRS:
                src = os.path.join(WORKSPACE, d)
                if os.path.isdir(src):
                    shutil.rmtree(os.path.join(mnt, d), ignore_errors=True)
                    copy_tree(src, os.path.join(mnt, d))
        for junk in MAC_JUNK:
            shutil.rmtree(os.path.join(mnt, junk), ignore_errors=True)
        # AppleDouble files for source files with extended attributes; in
        # AUTO, EmuTOS would try to run ._*.PRG.
        for root, _, names in os.walk(mnt):
            for name in names:
                if name.startswith("._"):
                    os.remove(os.path.join(root, name))
    finally:
        run(["hdiutil", "detach", dev])


def write_config(emutos, fast_ram, size, hostfs):
    # A large window would grow off screen from ARAnyM's centred 640x480 start.
    window_pos = "0,0" if size else "center"
    hostfs_drive = f"{HOSTFS_DRIVE} = {WORKSPACE}" if hostfs else ""
    config = f"""\
# Generated by tools/aranym_run.py; changes are overwritten on each launch.
[GLOBAL]
FastRAM = {fast_ram}
Floppy =
TOS =
EmuTOS = {emutos}
BootDrive = C
GMTime = No

[STARTUP]
GrabMouse = Yes
Debugger = No

[HOTKEYS]
Setup = LM+,
Quit = LM+q
Reboot = LM+r
ColdReboot = LS+LM+r
Ungrab = LS+LM+m
Debug = LA+Pause
Screenshot = LM+s
Fullscreen = LM+f
Sound = RM+s

[JIT]
JIT = Yes
JITFPU = Yes

[VIDEO]
FullScreen = No
BootColorDepth = 1
VidelMonitor = 0
WindowPos = {window_pos}

[TOS]
Cookie_MCH = 50000
RedirConsole = No

[IDE0]
Present = Yes
IsCDROM = No
ByteSwap = Yes
ReadOnly = No
Path = {IMAGE}
Cylinders = {CYLINDERS}
Heads = {HEADS}
SectorsPerTrack = {SECTORS}
ModelName = Realtimer

[IDE1]
Present = No

[HOSTFS]
{hostfs_drive}

[MIDI]
Enabled = Yes
Type = file
File = {MIDI_OUT}

[ETH0]
Type = none
"""
    with open(CONFIG, "w") as f:
        f.write(config)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--no-build", action="store_true", help="skip make")
    ap.add_argument("--prg", default="realtim5.prg",
                    help="program to run, copied as C:\\REALTIM5.PRG, or run in place "
                         "with --hostfs (default: realtim5.prg)")
    ap.add_argument("--reset", action="store_true",
                    help="recreate the disk image, including SETUPS and INFOS")
    ap.add_argument("--sync-only", action="store_true", help="update the image, do not run")
    ap.add_argument("--emutos", default=DEFAULT_EMUTOS, help="EmuTOS image for ARAnyM")
    ap.add_argument("--fast-ram", type=int, default=64, help="FastRAM in MB (default: 64)")
    ap.add_argument("--app", help="MacAranym .app bundle (default: found in /Applications)")
    ap.add_argument("--screen", default="auto",
                    help="auto (fill the host screen, default), WIDTHxHEIGHT, "
                         "or vga (EmuTOS 640x480 mono, no fVDI)")
    ap.add_argument("--hostfs", action="store_true",
                    help=f"run Realtimer from the workspace, mounted as {HOSTFS_DRIVE}: "
                         "through BetaDOS and hostfs.dos, instead of from a copy")
    ap.add_argument("--drivers", default=DEFAULT_DRIVERS,
                    help="folder with auto/fvdi.prg, gemsys/aranym.sys and, for --hostfs, "
                         "auto/betados.prg and auto/hostfs.dos "
                         "(default: ~/Documents/aranym_files/drive_c)")
    a = ap.parse_args()

    size = screen_size(a.screen)
    fvdi = find_files(a.drivers, FVDI_FILES) if size else None
    if size and not fvdi:
        print(f"fVDI not found in {a.drivers} (auto/fvdi.prg, gemsys/aranym.sys); "
              "using 640x480 mono")
        size = None
    hostfs = find_files(a.drivers, HOSTFS_FILES) if a.hostfs else None
    if a.hostfs and not hostfs:
        sys.exit(f"--hostfs needs auto/betados.prg and auto/hostfs.dos in {a.drivers}")

    app = a.app or find_app()
    if not a.sync_only:
        if not app or not os.path.isdir(app):
            sys.exit("MacAranym not found in /Applications; pass --app")
        if not os.path.isfile(a.emutos):
            sys.exit(f"EmuTOS for ARAnyM not found: {a.emutos}\n"
                     "Get emutos-aranym-*.zip from https://emutos.sourceforge.io/ "
                     "and pass --emutos")
    if os.path.exists(IMAGE) and image_in_use():
        sys.exit("ARAnyM is still using the disk image; quit it first (Cmd+Q)")

    if not a.no_build:
        subprocess.run(["make", "-C", WORKSPACE], check=True)

    fresh = a.reset or not os.path.exists(IMAGE)
    if fresh:
        print("Creating", os.path.relpath(IMAGE, WORKSPACE))
        create_image()
    sync(a.prg, with_data=fresh, fvdi=fvdi, size=size, hostfs=hostfs)
    if hostfs:
        print(f"Boot image ready; Realtimer runs from the workspace as {hostfs_path(a.prg)}")
    else:
        print("Synced", a.prg, "and resources" + (", SETUPS and INFOS" if fresh else ""))
    print("Screen:", "%dx%d, 256 colours (fVDI)" % size if size else "640x480 mono")
    if a.sync_only:
        return

    write_config(os.path.abspath(a.emutos), a.fast_ram, size, hostfs)
    print("Starting", os.path.basename(app), "- log:", os.path.relpath(LOG, WORKSPACE))
    with open(LOG, "w") as log:
        subprocess.Popen([app_binary(app), "--config", CONFIG], cwd=OUT,
                         stdout=log, stderr=subprocess.STDOUT, start_new_session=True)


if __name__ == "__main__":
    main()
