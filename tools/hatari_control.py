#!/usr/bin/env python3
"""Drive Realtimer inside Hatari from the command line.

Launches Hatari with the project's usual settings (workspace mounted as C:,
program auto-started) and keeps a control socket open so the running
emulator can be driven without touching its window: simulated key presses,
mouse buttons, screenshots, Hatari debugger commands, pause/continue.

Typical uses
    tools/hatari_control.py                      interactive session
    tools/hatari_control.py --script tools/smoke.hatari
    tools/hatari_control.py -c "wait 20; screenshot start; key F8; wait 2; screenshot koo; quit"

Commands (interactive, --script file or -c; '#' starts a comment)
    wait SECONDS            pause the script (emulation keeps running)
    screenshot [NAME]       save the emulated screen as PNG (optionally renamed to NAME.png)
    key NAME [NAME ...]     press and release keys, e.g. key F8 Return ctrl+c shift+F1
    keydown NAME / keyup NAME
    type TEXT               type TEXT (US keyboard positions, shift handled)
    doubleclick             left mouse double click at the current pointer position
    rightclick / rightdown / rightup
    debug COMMAND           Hatari debugger command, e.g. debug m $1000 or debug r
    shortcut NAME           Hatari shortcut: screenshot, warmreset, coldreset, pause, quit, ...
    option ARGS             change Hatari command line options at runtime
    stop / cont             stop / continue the emulation
    raw LINE                send LINE verbatim to the control socket
    log [N]                 show the last N lines of Hatari's output
    help                    list commands and key names
    quit                    quit Hatari and end the session

Only the mouse buttons can be simulated; Hatari's control socket has no
pointer movement, so dialogs are best driven through keyboard shortcuts.
"""

import argparse
import os
import shlex
import shutil
import socket
import subprocess
import sys
import time

# --------------------------------------------------------------------------
# Defaults, mirroring .vscode/launch.json and tasks.json
# --------------------------------------------------------------------------

HERE = os.path.dirname(os.path.abspath(__file__))
WORKSPACE = os.path.dirname(HERE)
DEFAULT_SDK = os.environ.get(
    "ATARIST_TOOLS",
    os.path.expanduser("~/.vscode/extensions/dgis.atari-st-dev-0.2.1/sdk/darwin"))
TT_TOS = os.path.expanduser(
    "~/Documents/Development/Hatari/ROM/EMUTOS/emutos-1024k-1.4/etos1024k.img")

# --------------------------------------------------------------------------
# Atari ST keyboard scancodes (positional, US layout as used by etos*us.img)
# --------------------------------------------------------------------------

SCANCODES = {
    "esc": 1, "escape": 1,
    "1": 2, "2": 3, "3": 4, "4": 5, "5": 6, "6": 7, "7": 8, "8": 9, "9": 10, "0": 11,
    "-": 12, "minus": 12, "=": 13, "equals": 13,
    "backspace": 14, "tab": 15,
    "q": 16, "w": 17, "e": 18, "r": 19, "t": 20, "y": 21, "u": 22, "i": 23, "o": 24, "p": 25,
    "[": 26, "]": 27, "return": 28, "enter": 28, "ctrl": 29, "control": 29,
    "a": 30, "s": 31, "d": 32, "f": 33, "g": 34, "h": 35, "j": 36, "k": 37, "l": 38,
    ";": 39, "'": 40, "`": 41, "shift": 42, "lshift": 42, "\\": 43,
    "z": 44, "x": 45, "c": 46, "v": 47, "b": 48, "n": 49, "m": 50,
    ",": 51, ".": 52, "/": 53, "rshift": 54, "alt": 56, "alternate": 56,
    "space": 57, " ": 57, "capslock": 58,
    "f1": 59, "f2": 60, "f3": 61, "f4": 62, "f5": 63, "f6": 64, "f7": 65, "f8": 66, "f9": 67, "f10": 68,
    "home": 71, "clrhome": 71, "up": 72, "kp-": 74, "left": 75, "right": 77, "kp+": 78, "down": 80,
    "insert": 82, "delete": 83, "del": 83, "undo": 97, "help": 98,
    "kp(": 99, "kp)": 100, "kp/": 101, "kp*": 102,
    "kp7": 103, "kp8": 104, "kp9": 105, "kp4": 106, "kp5": 107, "kp6": 108,
    "kp1": 109, "kp2": 110, "kp3": 111, "kp0": 112, "kp.": 113, "kpenter": 114,
}

# Characters that need Shift on the US layout, mapped to their unshifted key.
SHIFTED = {
    "!": "1", "@": "2", "#": "3", "$": "4", "%": "5", "^": "6", "&": "7", "*": "8",
    "(": "9", ")": "0", "_": "-", "+": "=", "{": "[", "}": "]", ":": ";", '"': "'",
    "~": "`", "|": "\\", "<": ",", ">": ".", "?": "/",
}

MODIFIERS = {"shift": 42, "lshift": 42, "rshift": 54, "ctrl": 29, "control": 29,
             "alt": 56, "alternate": 56}


def scancode(name):
    """Return the ST scancode for a key name or a bare number."""
    key = name.lower()
    if key.isdigit() and len(key) > 1:
        return int(key)
    if key in SCANCODES:
        return SCANCODES[key]
    raise ValueError("unknown key '%s' (try: help)" % name)


# --------------------------------------------------------------------------
# Session
# --------------------------------------------------------------------------

class Session:
    def __init__(self, args):
        self.args = args
        self.sock_path = args.socket
        self.log_path = args.log
        self.shots = args.screenshot_dir
        self.hatari = None
        self.conn = None
        self.log_pos = 0
        self.shot_count = 0

    # ---- lifecycle ------------------------------------------------------

    def hatari_binary(self):
        if self.args.hatari:
            return self.args.hatari
        mac = os.path.join(self.args.sdk, "hatari", "Hatari.app", "Contents", "MacOS", "Hatari")
        if os.path.exists(mac):
            return mac
        for cand in (os.path.join(self.args.sdk, "hatari", "hatari"), shutil.which("hatari")):
            if cand and os.path.exists(cand):
                return cand
        sys.exit("Hatari binary not found; use --hatari or --sdk")

    def hatari_args(self):
        a = self.args
        tos = a.tos or (TT_TOS if a.machine == "tt" else os.path.join(a.sdk, "hatari", "etos512us.img"))
        cmd = [self.hatari_binary(), "--tos", tos, "--confirm-quit", "no", "--sound", "off",
               "--fast-forward", "yes" if a.fast else "no",
               "--screenshot-dir", self.shots, "--screenshot-format", "png", "--crop", "yes",
               "--control-socket", self.sock_path]
        if a.machine == "tt":
            cmd += ["--machine", "tt", "--ttram", "0", "--memsize", "14"]
            cmd += ["--tos-res", "ttmed"] if a.color else ["--mono"]
        else:
            cmd += ["--memsize", "4"]
            cmd += [] if a.color else ["--mono"]
        if a.zoom:
            cmd += ["--zoom", str(a.zoom)]
        if a.debugger:
            cmd += ["--parse", os.path.join(a.sdk, "hatari", "db.ini")]
        cmd += shlex.split(a.hatari_opts) if a.hatari_opts else []
        cmd.append(os.path.join(WORKSPACE, a.prg))
        return cmd

    def start(self):
        os.makedirs(self.shots, exist_ok=True)
        os.makedirs(os.path.dirname(self.log_path) or ".", exist_ok=True)
        if os.path.exists(self.sock_path):
            os.remove(self.sock_path)
        server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        server.bind(self.sock_path)
        server.listen(1)
        cmd = self.hatari_args()
        if self.args.verbose:
            print("launch:", " ".join(shlex.quote(c) for c in cmd))
        self.hatari = subprocess.Popen(cmd, cwd=WORKSPACE,
                                       stdout=open(self.log_path, "w"), stderr=subprocess.STDOUT)
        server.settimeout(self.args.connect_timeout)
        try:
            self.conn, _ = server.accept()
        except socket.timeout:
            self.hatari.terminate()
            sys.exit("Hatari did not connect to the control socket within %ds (see %s)"
                     % (self.args.connect_timeout, self.log_path))
        finally:
            server.close()
        print("Hatari connected (pid %d); log: %s; screenshots: %s"
              % (self.hatari.pid, self.log_path, self.shots))

    def alive(self):
        return self.hatari is not None and self.hatari.poll() is None

    def close(self):
        if self.conn:
            try:
                self.conn.close()
            except OSError:
                pass
        if os.path.exists(self.sock_path):
            os.remove(self.sock_path)

    # ---- low level ------------------------------------------------------

    def send(self, line, settle=0.15):
        if not self.alive():
            raise RuntimeError("Hatari has exited")
        if self.args.verbose:
            print("  ->", line)
        self.conn.sendall((line + "\n").encode("latin-1", "replace"))
        time.sleep(settle)
        self.report_log()

    def report_log(self, show_all=False, tail=None):
        """Echo new ERROR/WARN lines from Hatari's output (or the last lines)."""
        try:
            with open(self.log_path, "rb") as f:
                if tail is not None:
                    lines = f.read().decode("utf-8", "replace").splitlines()
                    for l in lines[-tail:]:
                        print("  hatari:", l)
                    return
                f.seek(self.log_pos)
                data = f.read()
                self.log_pos = f.tell()
        except OSError:
            return
        for l in data.decode("utf-8", "replace").splitlines():
            if show_all or l.startswith(("ERROR", "- ")) or "Supported" in l or "shortcuts are" in l:
                print("  hatari:", l)

    # ---- key handling ---------------------------------------------------

    def press(self, spec):
        """Press one key spec such as 'F8', 'ctrl+c', 'shift+F1', '28'."""
        parts = spec.split("+") if len(spec) > 1 else [spec]
        mods, key = [], parts[-1]
        for m in parts[:-1]:
            if m.lower() not in MODIFIERS:
                raise ValueError("unknown modifier '%s'" % m)
            mods.append(MODIFIERS[m.lower()])
        code = scancode(key)
        for m in mods:
            self.send("hatari-event keydown %d" % m, 0.05)
        self.send("hatari-event keypress %d" % code, 0.05)
        for m in reversed(mods):
            self.send("hatari-event keyup %d" % m, 0.05)

    def type_text(self, text):
        for ch in text:
            shift = False
            if ch in SHIFTED:
                ch, shift = SHIFTED[ch], True
            elif ch.isalpha() and ch.isupper():
                ch, shift = ch.lower(), True
            if ch == "\n":
                ch = "return"
            code = scancode(ch)
            if shift:
                self.send("hatari-event keydown 42", 0.05)
            self.send("hatari-event keypress %d" % code, 0.08)
            if shift:
                self.send("hatari-event keyup 42", 0.05)

    # ---- screenshots ----------------------------------------------------

    def screenshot(self, name=None):
        before = set(os.listdir(self.shots))
        self.send("hatari-shortcut screenshot", 0.3)
        new = None
        for _ in range(20):
            added = [f for f in os.listdir(self.shots) if f not in before and f.lower().endswith(".png")]
            if added:
                new = os.path.join(self.shots, sorted(added)[-1])
                break
            time.sleep(0.1)
        if new is None:
            print("  screenshot: no file appeared in", self.shots)
            return
        if name:
            target = os.path.join(self.shots, name if name.lower().endswith(".png") else name + ".png")
            os.replace(new, target)
            new = target
        self.shot_count += 1
        print("  screenshot:", new)

    # ---- command interpreter -------------------------------------------

    def run_command(self, line):
        line = line.split("#", 1)[0].strip()
        if not line:
            return True
        try:
            words = shlex.split(line)
        except ValueError:
            words = line.split()
        cmd, rest = words[0].lower(), line[len(words[0]):].strip()
        try:
            if cmd == "wait":
                time.sleep(float(words[1]))
            elif cmd == "screenshot":
                self.screenshot(words[1] if len(words) > 1 else None)
            elif cmd == "key":
                for spec in words[1:]:
                    self.press(spec)
            elif cmd == "keydown":
                self.send("hatari-event keydown %d" % scancode(words[1]))
            elif cmd == "keyup":
                self.send("hatari-event keyup %d" % scancode(words[1]))
            elif cmd == "type":
                self.type_text(rest.strip('"') if rest.startswith('"') and rest.endswith('"') else rest)
            elif cmd == "doubleclick":
                self.send("hatari-event doubleclick")
            elif cmd == "rightclick":
                self.send("hatari-event rightdown", 0.1)
                self.send("hatari-event rightup")
            elif cmd in ("rightdown", "rightup"):
                self.send("hatari-event " + cmd)
            elif cmd == "debug":
                self.send("hatari-debug " + rest, 0.3)
                self.report_log(show_all=True)
            elif cmd == "shortcut":
                self.send("hatari-shortcut " + rest)
            elif cmd == "option":
                self.send("hatari-option " + rest)
            elif cmd in ("stop", "cont"):
                self.send("hatari-" + cmd)
            elif cmd == "raw":
                self.send(rest)
            elif cmd == "log":
                self.report_log(tail=int(words[1]) if len(words) > 1 else 20)
            elif cmd == "help":
                print(__doc__)
                print("Key names:", " ".join(sorted(k for k in SCANCODES if len(k) > 1)))
            elif cmd == "quit":
                self.send("hatari-shortcut quit", 0.5)
                return False
            else:
                print("  unknown command '%s' (try: help)" % cmd)
        except (ValueError, IndexError) as e:
            print("  error:", e)
        except RuntimeError as e:
            print(" ", e)
            return False
        return self.alive()

    def run_script(self, lines):
        for line in lines:
            if self.args.verbose and line.strip():
                print(">", line.rstrip())
            if not self.run_command(line):
                return
            if not self.alive():
                return

    def interactive(self):
        print("Type commands ('help' for a list, 'quit' to end). Hatari keeps running until quit.")
        while self.alive():
            try:
                line = input("hatari> ")
            except EOFError:
                print()
                break
            if not self.run_command(line):
                break


# --------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter,
                                 epilog=__doc__.split("Typical uses", 1)[1])
    ap.add_argument("--prg", default="realtim5.prg", help="program to auto-start, relative to the workspace")
    ap.add_argument("--sdk", default=DEFAULT_SDK, help="Atari ST dev SDK directory (Hatari, EmuTOS, db.ini)")
    ap.add_argument("--hatari", help="Hatari executable (default: the SDK's Hatari.app)")
    ap.add_argument("--tos", help="TOS image (default: SDK etos512us.img, or the TT EmuTOS for --machine tt)")
    ap.add_argument("--machine", choices=["st", "tt"], default="st")
    ap.add_argument("--color", action="store_true", help="colour mode instead of mono")
    ap.add_argument("--zoom", type=float, help="Hatari window zoom factor")
    ap.add_argument("--no-fast", dest="fast", action="store_false", help="run at normal speed")
    ap.add_argument("--debugger", action="store_true", help="also parse the SDK's db.ini (breakpoint at TEXT)")
    ap.add_argument("--hatari-opts", help="extra Hatari command line options, quoted as one string")
    ap.add_argument("--screenshot-dir", default=os.path.join(WORKSPACE, "build", "hatari-shots"))
    ap.add_argument("--log", default=os.path.join(WORKSPACE, "build", "hatari.log"))
    ap.add_argument("--socket", default="/tmp/realtimer-hatari-%d.sock" % os.getpid())
    ap.add_argument("--connect-timeout", type=int, default=30)
    ap.add_argument("--script", help="file with one command per line")
    ap.add_argument("-c", "--commands", help="commands separated by ';'")
    ap.add_argument("--keep-open", action="store_true",
                    help="after a script, drop into the interactive prompt instead of quitting")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()

    session = Session(args)
    session.start()
    try:
        scripted = False
        if args.script:
            with open(args.script, encoding="utf-8") as f:
                session.run_script(f.readlines())
            scripted = True
        if args.commands:
            session.run_script(args.commands.split(";"))
            scripted = True
        if session.alive() and (not scripted or args.keep_open):
            session.interactive()
        if session.alive():
            session.run_command("quit")
        if session.hatari:
            try:
                session.hatari.wait(timeout=10)
            except subprocess.TimeoutExpired:
                session.hatari.terminate()
        session.report_log()
        print("Hatari exited with code", session.hatari.returncode if session.hatari else "?")
    finally:
        session.close()


if __name__ == "__main__":
    main()
