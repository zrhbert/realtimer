# tools

## hatari_control.py — drive Realtimer in Hatari without touching its window

An optional debugging aid. It launches Hatari the same way the VS Code
tasks do (workspace mounted as drive C:, `realtim5.prg` auto-started,
EmuTOS, mono, fast-forward) and keeps Hatari's *control socket* open, so
the running emulator can be fed simulated key presses and mouse buttons,
asked for screenshots, paused, or handed debugger commands. Useful for
reproducing a problem step by step, for before/after screenshots of a
fix, and for quick smoke tests after a build.

Requirements: Python 3 (no extra packages) and the Hatari bundled with the
*Atari ST Dev* VS Code extension. Nothing has to be installed in the
emulated system.

### Running it

```sh
tools/hatari_control.py                                   # interactive prompt
tools/hatari_control.py --script tools/smoke.hatari       # scripted session
tools/hatari_control.py -c "wait 20; screenshot a; key F8; wait 2; screenshot b; quit"
tools/hatari_control.py --machine tt --color              # TT medium, like "Debug in Hatari (TT-color)"
tools/hatari_control.py --prg rt5run.prg --no-fast        # stripped PRG, normal speed
```

From VS Code: task **Hatari: control session** (builds first, then opens
the prompt in the terminal) or **Hatari: smoke test** (runs
`tools/smoke.hatari`).

Screenshots land in `build/hatari-shots/`, Hatari's own output in
`build/hatari.log`; both are ignored by git.

### Commands

| Command | Effect |
|---|---|
| `wait SECONDS` | pause the script; the emulation keeps running |
| `screenshot [NAME]` | PNG of the emulated screen, optionally renamed to `NAME.png` |
| `key NAME ...` | press and release keys, e.g. `key F8`, `key ctrl+c`, `key shift+F1 Return` |
| `keydown NAME`, `keyup NAME` | hold / release a key |
| `type TEXT` | type text; upper case and US punctuation get Shift automatically |
| `doubleclick` | left double click at the current pointer position |
| `rightclick`, `rightdown`, `rightup` | right mouse button |
| `debug COMMAND` | Hatari debugger command, output goes to the log (`debug r`, `debug m $1000`) |
| `shortcut NAME` | Hatari shortcut: `screenshot`, `warmreset`, `coldreset`, `mousegrab`, `quit`, ... |
| `option ARGS` | change Hatari command line options at run time |
| `stop`, `cont` | freeze / resume the emulation |
| `raw LINE` | send a line verbatim to the control socket |
| `log [N]` | show the last N lines of Hatari's output |
| `help` | command list and all key names |
| `quit` | quit Hatari (no confirmation) and end the session |

Key names are the ST key cap names: letters, digits, `F1`..`F10`,
`Return`, `Esc`, `Tab`, `Space`, `Backspace`, `Delete`, `Insert`, `Home`,
`Undo`, `Help`, `Up`/`Down`/`Left`/`Right`, `kp0`..`kp9`, `kpenter`, and
the modifiers `shift`, `ctrl`, `alt`. A bare number is taken as a raw ST
scancode (`key 28` is Return). Positions follow the US layout of the
bundled `etos512us.img`.

### Limits

Hatari's control socket cannot move the mouse pointer, only press buttons
where it is, so dialogs are best driven with keyboard shortcuts and
function keys. The `debug` command needs no special Hatari build, but
its output appears only in the log, not in the Hatari window.
