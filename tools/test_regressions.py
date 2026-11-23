#!/usr/bin/env python3
"""Run the Atari regression executable in a headless Hatari ST or TT."""

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

import hatari_control as hc


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("prg", help="68000 regression executable")
    parser.add_argument("--sdk", default=hc.DEFAULT_SDK)
    parser.add_argument("--hatari", help="Hatari executable")
    parser.add_argument("--tos", help="TOS image")
    parser.add_argument("--machine", choices=["st", "tt"], default="st")
    parser.add_argument("--timeout", type=int, default=30)
    options = parser.parse_args()

    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
    os.environ.setdefault("SDL_AUDIODRIVER", "dummy")
    # Hatari reads the user's saved config from $HOME (on macOS under
    # Library/Application Support); a saved SCSI boot image or hard disk
    # setup there keeps the regression program from starting.
    home = tempfile.mkdtemp(prefix="realtimer-hatari-home-")
    os.environ["HOME"] = home
    with tempfile.TemporaryDirectory(prefix="realtimer-tests-") as directory:
        root = Path(directory)
        program = root / "REGTEST.PRG"
        result = root / "RESULT.TXT"
        log = root / "hatari.log"
        shutil.copyfile(options.prg, program)
        machine_opts = ("--machine tt --cpulevel 3 --cpuclock 32 --fpu 68882"
                        if options.machine == "tt"
                        else "--machine st --cpulevel 0 --cpuclock 8 --tos-res high")
        args = argparse.Namespace(
            prg=str(program), sdk=options.sdk, hatari=options.hatari,
            tos=options.tos, machine=options.machine, color=False, zoom=None, fast=True,
            debugger=False,
            hatari_opts=machine_opts,
            screenshot_dir=str(root), log=str(log),
            socket="/tmp/realtimer-test-%d.sock" % os.getpid(),
            connect_timeout=30, verbose=False)
        session = hc.Session(args)
        session.start()
        report = ""
        complete = False
        try:
            deadline = time.monotonic() + options.timeout
            while time.monotonic() < deadline and session.alive():
                if result.exists():
                    report = result.read_text(encoding="ascii")
                    if report.endswith("\n") and report.splitlines()[-1].startswith("RESULT "):
                        complete = True
                        break
                time.sleep(0.1)
        finally:
            if session.alive():
                session.run_command("quit")
            try:
                session.hatari.wait(timeout=10)
            except subprocess.TimeoutExpired:
                session.hatari.terminate()
                session.hatari.wait(timeout=10)
            session.close()

        if not complete:
            print("Regression executable did not finish; Hatari log:", file=sys.stderr)
            if log.exists():
                print("\n".join(log.read_text(errors="replace").splitlines()[-40:]),
                      file=sys.stderr)
            return 1
        print(report, end="")
        return 0 if report.splitlines()[-1] == "RESULT 0" else 1


if __name__ == "__main__":
    original_home = os.environ.get("HOME")
    try:
        status = main()
    finally:
        if original_home is not None:
            temp_home = os.environ.get("HOME")
            os.environ["HOME"] = original_home
            if temp_home and temp_home != original_home:
                shutil.rmtree(temp_home, ignore_errors=True)
    sys.exit(status)
