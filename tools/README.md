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

### Driving the transport

The transport bar buttons need the mouse, so the TRA window also reacts
to **Ctrl+Space** (play / stop) and **Ctrl+R** (record on / off), which
the tool can send: `key ctrl+space`. The internal MIDI kernel supplies the
playback clock; a resident MidiShare program is not required.

The transport's **C** shortcut toggles the Atari speaker click. It sounds
while playing at 120 BPM in 4/4, with a higher first beat. The click uses
the system PSG sound sequence, so it can interrupt other PSG sounds. Modal
dialogs that stop timer events pause the click; missed beats are not replayed.

### CMO position overview

Open **Output > CMO Cybermove Output** or the CMO desktop icon. Each window
shows the 32 inputs of the selected system; use **Optionen > CM System 1/2**
to choose inputs 1-32 or 33-64. The first window defaults to System 1 and
the second to System 2; each title identifies its current input range.
The line below each cube shows volume; zero volume appears as a dot.
MIDI position updates are coalesced on the 100 ms window timer, displaying
the latest values. Ordinary updates redraw only changed display objects;
resizing, changing views, and resets still request an immediate full refresh.
The display follows the MIDI channel and port configured in CMI. If both
systems use the same channel and port, their events are indistinguishable
and appear in both views. A window opened during playback receives a fresh
position update on the next PUF tick. This refresh also resends the current
PUF positions to its connected MIDI outputs once.

KOO lists the current CMI assignment for signals 1-8, while playback uses
the assignments recorded in PUF. Signal 0 is the lead position and has no
CMI output. Select **Optionen > nur Innenraum** to toggle the zoomed-in view.
CMO resets its positions when the transport stops; KOO retains its last values.

CMO is an overview only: the original ROM-port mixer output is disabled,
as are its master controls and unfinished preset actions. Numbered setups
require an existing CMO setup file; without one, the default setup is still
available but numbered setup operations report a missing file.

### Limits

Hatari's control socket cannot move the mouse pointer, only press buttons
where it is, so dialogs are best driven with keyboard shortcuts and
function keys. The `debug` command needs no special Hatari build, but
its output appears only in the log, not in the Hatari window.

### Checkbox and radio-button regression checks

`fix_objs()` converts resource `G_BOX` objects marked `CROSSED` to
`G_USERDEF`, including in GCC builds. The callbacks use the original
`USERIMG` checkbox and round radio-button bitmaps; `SELECTED` remains
the stored value. Do not replace this with drawing over AES-selected boxes:
that changes the artwork and draws outside AES's mouse-cursor protection.

In Hatari, open Settings with Ctrl+C and a module setup dialog with radio
buttons (for example A3D). Check box clicks, label clicks, Alt shortcuts,
and selecting another radio button. Click a box and move the pointer away:
there should be no stale pixels. Also check press-drag-release and redraw
after moving or uncovering the dialog. Repeat in monochrome and colour
modes to exercise both high- and low-height bitmaps.

### Setup-dialog refresh

Changing a setup redraws changed fields and combines adjacent dirty regions,
while preserving the drawing order of overlapping borders and backgrounds.
Changes to the object tree's layout fall back to a full redraw.

### Module help

Module help requires the optional PC_HELP accessory. Without it, a help
request displays one unavailable-help message. The bundled module help
trees contain placeholder text and are not used as a fallback.

## Regression tests

Run `make test TOOLCHAIN=/path/to/cross-mint` to build and execute the
variable-dispatch, synchronized PUF position, transport-click, CMI and LFO
setup, CMO routing, 3D projection, initialization-window lifetime,
help-dispatch/message, and MIDI timer keyboard-interrupt checks in a
headless Hatari ST. The timer check requests an IKBD clock response during
a scheduled callback and verifies that the reply arrives before it ends.
CMO checks cover redraw coalescing, urgent redraws, and updates received
while a redraw is in progress. Setup checks cover changed and unchanged
fields, adjacent regions, button release, and layout-change fallback.
The tests need Python 3, Hatari, and the SDK's ST EmuTOS image; set
`ATARIST_TOOLS` if the SDK cannot be found automatically.

Run `make test-tt TOOLCHAIN=/path/to/cross-mint` to run the same regressions
with the TT build and test floating-point state across MIDI timer interrupts.

## bench.py — TT performance benchmark with CPU profile

`tools/bench.py [TAG]` runs the fixed benchmark scenario (record + play
for a 4200-VBL window, ~59 s, on an emulated TT at normal speed) with
Hatari's cycle-exact CPU profiler, and prints the top functions by
exclusive cycles (program, EmuTOS ROM and other-RAM shares separated).
Results land in `build/bench-TAG.log` and `build/profile-TAG.txt`.
Compare two runs by tag to judge an optimization. See the module
docstring for the Hatari quirks the script works around.
