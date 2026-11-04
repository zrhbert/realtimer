#!/bin/sh
# Maintain a stable symlink to the Atari ST Dev extension's SDK.
#
# The extension installs under a directory whose name changes with every
# version and platform (dgis.atari-st-dev-<version>-<platform>), which kept
# breaking the absolute paths in .vscode. This script points the fixed
# symlink  ~/Documents/Development/Hatari/atari-sdk  at the most recently
# installed extension's SDK. It runs before every build via the
# "Atari ST: make" task, so the link self-heals after extension updates.
#
# Everything in .vscode (settings.json "realtimer.atariSdk", launch.json,
# tasks.json) and tools/hatari_control.py resolves the SDK through the link.

LINK="$HOME/Documents/Development/Hatari/atari-sdk"

case "$(uname -s)" in
    Darwin) plat=darwin ;;
    Linux)  plat=linux ;;
    *)      plat=win32 ;;
esac

# newest installation wins (by modification time, robust against version
# strings that do not sort lexically)
sdk=$(ls -td "$HOME"/.vscode/extensions/dgis.atari-st-dev-*/sdk/"$plat" 2>/dev/null | head -1)

if [ -z "$sdk" ]; then
    echo "atari_sdk_link: no dgis.atari-st-dev extension found; leaving $LINK alone" >&2
    exit 0
fi

current=$(readlink "$LINK" 2>/dev/null)
if [ "$current" != "$sdk" ]; then
    ln -sfn "$sdk" "$LINK"
    echo "atari_sdk_link: $LINK -> $sdk"
fi
exit 0
