#!/usr/bin/env python3
"""Performance benchmark: record + play on an emulated Atari TT, CPU-profiled.

Scenario (fixed, so numbers stay comparable across runs):
  boot at fast-forward -> normal speed -> record on (Ctrl+R) -> play
  (Ctrl+Space) -> 5 s settle -> profile a fixed 4200-VBL window (~59 s at
  the TT mono 71 Hz frame rate) -> dump profiler data -> report.

Hatari quirks this works around:
  - Profiler data is only processed on a *real* debugger entry; control
    socket commands don't count and reset the sample buffer on resume.
    The window is therefore closed by a one-shot breakpoint (VBL > target)
    armed *before* "profile on"; on hit, Hatari runs build/bench_dump.ini
    (profile stats + a full per-address cycles table, then continue)
    inside a genuine debugger entry.
  - "symbols prg" doesn't work for this program ("no program loaded"), so
    the program's symbols are generated with nm and relocated by the TEXT
    base from "info basepage" (runtime address = TEXT base + ELF vaddr,
    verified by disassembly comparison), then loaded as an absolute
    nm-style file.
  - "profile save" writes no per-address data lines on this build, so the
    report instead parses the "profile cycles" table from the log and
    aggregates per function offline (program symbols via nm, EmuTOS ROM
    symbols via the etos1024k.sym file next to the TOS image).

Usage: tools/bench.py [TAG]        (TAG names the result files)

Results:
  build/bench-TAG.log       full Hatari log incl. the per-address table
  build/profile-TAG.txt     raw "profile save" (call-graph; reference only)
  build/hatari-shots/bench-TAG.png
  and a top-functions-by-cycles report on stdout.

The SCSI image from the user's saved Hatari config is detached and the
GEMDOS drive forced to C: so the program finds its files on the TT.
"""

import argparse
import bisect
import os
import re
import shutil
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hatari_control as hc

VBL_WINDOW = int(os.environ.get("BENCH_VBL_WINDOW", "4200"))  # ~59 s TT mono
BOOT_SECS = 25               # EmuTOS boot + program start, at fast-forward
SETTLE_SECS = 5              # transport running before measurement starts
NM = "/opt/cross-mint/bin/m68k-atari-mintelf-nm"

DUMP_INI = """\
profile stats
profile symbols 30
profile cycles 20000
profile save {profile_raw}
c
"""


def log_since(path, pos):
    with open(path, "rb") as f:
        f.seek(pos)
        data = f.read()
    return data.decode("utf-8", "replace"), pos + len(data)


def read_elf_symbols(prg):
    """[(vaddr, name)] for all code symbols in the ELF, sorted."""
    out = subprocess.run([NM, "-n", prg], capture_output=True, text=True,
                         check=True).stdout
    syms = []
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[1] in ("T", "t"):
            syms.append((int(parts[0], 16), parts[2]))
    return syms


def read_sym_file(path):
    """[(addr, name)] from an nm-style symbol file (e.g. EmuTOS .sym)."""
    syms = []
    if not os.path.exists(path):
        return syms
    for line in open(path, encoding="utf-8", errors="replace"):
        parts = line.split()
        if len(parts) == 3 and parts[1] in ("T", "t"):
            try:
                syms.append((int(parts[0], 16), parts[2]))
            except ValueError:
                pass
    return syms


def report(log_path, prog_syms, text_base, text_size, rom_syms):
    """Aggregate the log's "profile cycles" per-address table per function."""
    table = sorted([(a, n, "program") for a, n in prog_syms] +
                   [(a, n, "rom") for a, n in rom_syms])
    addrs = [t[0] for t in table]
    text_end = text_base + text_size

    def lookup(addr):
        if text_base <= addr < text_end or addr >= 0xe00000:
            i = bisect.bisect_right(addrs, addr) - 1
            if i >= 0:
                return table[i][1], table[i][2]
        return ("<other RAM: MidiShare kernel etc.>", "ram")

    in_table = False
    secs = None
    funcs = {}
    for line in open(log_path, encoding="utf-8", errors="replace"):
        m = re.match(r"^= ([0-9.]+)s$", line.strip())
        if m:
            secs = max(secs or 0, float(m.group(1)))
        if line.startswith("addr:") and "cycles:" in line:
            in_table = True
            continue
        if not in_table:
            continue
        m = re.match(r"^0x([0-9a-f]+)\t\s*[\d.]+%\t(\d+)\t.*\((\d+),", line)
        if not m:
            if line.strip().endswith("CPU addresses listed."):
                in_table = False
            continue
        addr, cycles, instr = int(m.group(1), 16), int(m.group(2)), int(m.group(3))
        name, where = lookup(addr)
        r = funcs.setdefault((name, where), [0, 0])
        r[0] += cycles
        r[1] += instr
    total = sum(r[0] for r in funcs.values())
    if not total:
        print("report: no 'profile cycles' table found in", log_path)
        return
    print("\nmeasured: %.2fs CPU time attributed over the window" % (secs or 0))
    print("%-8s %6s %12s %12s  %s" %
          ("where", "cyc-%", "cycles", "instructions", "function"))
    ranked = sorted(funcs.items(), key=lambda kv: -kv[1][0])
    for (name, where), (cycles, instr) in ranked[:30]:
        print("%-8s %5.1f%% %12d %12d  %s" %
              (where, 100.0 * cycles / total, cycles, instr, name))
    for area in ("program", "rom", "ram"):
        sub = sum(r[0] for (n, w), r in funcs.items() if w == area)
        print("%-8s %5.1f%% %12d" % (area + ":", 100.0 * sub / total, sub))
    print("total attributed cycles: %d" % total)


def main():
    tag = sys.argv[1] if len(sys.argv) > 1 else "baseline"
    workspace = hc.WORKSPACE
    build = os.path.join(workspace, "build")
    os.makedirs(build, exist_ok=True)
    profile_raw = os.path.join(build, "profile-raw.txt")
    dump_ini = os.path.join(build, "bench_dump.ini")
    sym_file = os.path.join(build, "rt5abs.sym")
    if os.path.exists(profile_raw):
        os.remove(profile_raw)
    with open(dump_ini, "w") as f:
        f.write(DUMP_INI.format(profile_raw=profile_raw))

    prg = os.path.join(workspace, "realtim5.prg")
    elf_syms = read_elf_symbols(prg)
    rom_syms = read_sym_file(os.path.splitext(hc.TT_TOS)[0] + ".sym")
    print("bench: %d program / %d EmuTOS code symbols" %
          (len(elf_syms), len(rom_syms)))

    args = argparse.Namespace(
        prg="realtim5.prg", sdk=hc.DEFAULT_SDK, hatari=None, tos=None,
        machine="tt", color=False, zoom=None, fast=True, debugger=False,
        hatari_opts="--fpu 68882 --gemdos-drive C --scsi 0=none",
        screenshot_dir=os.path.join(build, "hatari-shots"),
        log=os.path.join(build, "hatari.log"),
        socket="/tmp/realtimer-bench-%d.sock" % os.getpid(),
        connect_timeout=30, verbose=False)

    s = hc.Session(args)
    s.start()
    try:
        print("bench: booting (%ds at fast-forward)" % BOOT_SECS)
        time.sleep(BOOT_SECS)

        # Program TEXT base -> absolute symbol file -> load into Hatari.
        pos = os.path.getsize(args.log)
        s.run_command("debug info basepage")
        time.sleep(0.5)
        text, pos = log_since(args.log, pos)
        m = re.search(r"Text segment\s*:\s*0x([0-9a-f]+)", text)
        ms = re.search(r"Text size\s*:\s*0x([0-9a-f]+)", text)
        if not (m and ms):
            sys.exit("bench: no basepage info in log:\n" + text)
        text_base, text_size = int(m.group(1), 16), int(ms.group(1), 16)
        prog_syms = [(text_base + v, n) for v, n in elf_syms]
        with open(sym_file, "w") as f:
            for addr, name in prog_syms:
                f.write("%08x T %s\n" % (addr, name))
        s.run_command("debug symbols %s" % sym_file)
        print("bench: TEXT at 0x%x (size 0x%x), symbols loaded"
              % (text_base, text_size))

        s.run_command("option --fast-forward no")
        time.sleep(2)
        print("bench: starting record + play")
        s.run_command("key ctrl+r")
        time.sleep(1)
        s.run_command("key ctrl+space")
        time.sleep(SETTLE_SECS)

        # Read the current VBL count to position the end-of-window breakpoint.
        pos = os.path.getsize(args.log)
        s.run_command("debug evaluate VBL")
        time.sleep(0.5)
        text, pos = log_since(args.log, pos)
        m = None
        for m in re.finditer(r"#(\d+) \(dec\)", text):
            pass
        if not m:
            sys.exit("bench: could not read VBL counter from log:\n" + text)
        vbl = int(m.group(1))
        target = vbl + VBL_WINDOW
        print("bench: VBL now %d, breakpoint at %d (%d frames)"
              % (vbl, target, VBL_WINDOW))

        s.run_command('debug b VBL > %d :once :quiet :file %s'
                      % (target, dump_ini))
        s.run_command("debug profile on")
        print("bench: measuring ~%ds at normal speed..." % (VBL_WINDOW // 71))
        time.sleep(VBL_WINDOW / 71.0)
        # The emulator may lag behind real time under load and the dump
        # itself takes a while: poll for the save file instead of guessing.
        waited = 0
        while not os.path.exists(profile_raw) and waited < 180:
            time.sleep(2)
            waited += 2
        if os.path.exists(profile_raw):
            print("bench: dump complete %ds after the nominal window" % waited)
        else:
            print("bench: WARNING: no profile dump after %ds" % waited)
        time.sleep(2)
        s.run_command("screenshot bench-%s" % tag)
        s.run_command("key ctrl+space")
        time.sleep(1)
        s.run_command("quit")
        if s.hatari:
            try:
                s.hatari.wait(timeout=15)
            except subprocess.TimeoutExpired:
                s.hatari.terminate()
    finally:
        s.close()

    log_copy = os.path.join(build, "bench-%s.log" % tag)
    shutil.copy(args.log, log_copy)
    out = os.path.join(build, "profile-%s.txt" % tag)
    if os.path.exists(profile_raw):
        os.replace(profile_raw, out)
    else:
        print("bench: WARNING: no profile save file (call-graph reference)")

    print("\n=== bench '%s' done ===" % tag)
    print("log:     build/bench-%s.log" % tag)
    print("profile: build/profile-%s.txt" % tag)
    report(log_copy, prog_syms, text_base, text_size, rom_syms)


if __name__ == "__main__":
    main()
