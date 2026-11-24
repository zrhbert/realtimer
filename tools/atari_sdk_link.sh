#!/bin/sh
# Maintain a stable, per-machine symlink to the Atari ST Dev extension's SDK.
#
# The extension installs under a directory whose name changes with every
# version and platform (dgis.atari-st-dev-<version>-darwin-<x64|arm64>), and
# ~/.vscode/extensions is local to each Mac. This script points the fixed
# symlink  ~/.atari-sdk  at the newest installed SDK built for this Mac's
# CPU. The link lives outside ~/Documents on purpose: Documents is synced
# through iCloud, so a link there flipped between the Intel and the Apple
# Silicon SDK whenever the other Mac built.
#
# It runs before every build via the "Atari ST: make" task, and the Makefile
# calls it with --print to locate the compiler. Everything in .vscode
# (settings.json "realtimer.atariSdk", launch.json, tasks.json) and
# tools/hatari_control.py resolves the SDK through the link.
#
# Usage: atari_sdk_link.sh [--print]
#   --print  write the SDK path to stdout (empty if none is installed)

LINK="$HOME/.atari-sdk"

case "$(uname -s)" in
    Darwin) plat=darwin ;;
    Linux)  plat=linux ;;
    *)      plat=win32 ;;
esac

# Prefer the native SDK. A shell running under Rosetta reports x86_64 on
# Apple Silicon, but the arm64 tools run there natively.
arch=$(uname -m)
if [ "$arch" = x86_64 ] && [ "$(sysctl -n sysctl.proc_translated 2>/dev/null)" = 1 ]; then
    arch=arm64
fi
case "$arch" in
    x86_64|amd64)  arch=x64 ;;
    arm64|aarch64) arch=arm64 ;;
esac

# Newest installation wins (by modification time, robust against version
# strings that do not sort lexically). Fall back to any SDK for this OS,
# e.g. old releases without a platform suffix (x64, runs under Rosetta).
ext="$HOME/.vscode/extensions"
sdk=$(ls -td "$ext"/dgis.atari-st-dev-*-"$plat"-"$arch"/sdk/"$plat" 2>/dev/null | head -1)
[ -n "$sdk" ] || sdk=$(ls -td "$ext"/dgis.atari-st-dev-*/sdk/"$plat" 2>/dev/null | head -1)

if [ -z "$sdk" ]; then
    echo "atari_sdk_link: no dgis.atari-st-dev extension found; leaving $LINK alone" >&2
    [ "$1" = --print ] && [ -d "$LINK" ] && echo "$LINK"
    exit 0
fi

current=$(readlink "$LINK" 2>/dev/null)
if [ "$current" != "$sdk" ]; then
    ln -sfn "$sdk" "$LINK"
    echo "atari_sdk_link: $LINK -> $sdk" >&2
fi
[ "$1" = --print ] && echo "$LINK"
exit 0
