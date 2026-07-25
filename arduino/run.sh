#!/bin/bash
set -euo pipefail

# Compile on this Mac; upload/monitor on the Pi (board is plugged in there).
# The Pi mounts this repo at PI_REPO via SSHFS, so local build/ is visible remotely.

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
FQBN=arduino:renesas_uno:unor4wifi
PI_HOST=${PI_HOST:-nolan@nolan-pi}
PI_REPO=${PI_REPO:-/home/nolan/src/arduino}
PI_ARDUINO_DIR=$PI_REPO/arduino

NAME=${1:?usage: $0 <SketchName>}
SKETCH=$NAME/$NAME.ino
BUILD_PATH=$SCRIPT_DIR/build/$NAME

TIMEOUT_BIN=$(command -v timeout || command -v gtimeout || true)
if [[ -z "$TIMEOUT_BIN" ]]; then
  echo "error: need timeout or gtimeout (brew install coreutils)" >&2
  exit 1
fi

run() {
  local label=$1 secs=$2 status
  shift 2
  echo "==> $label (timeout ${secs}s)" >&2
  "$TIMEOUT_BIN" "$secs" "$@" && return 0
  status=$?
  if [[ $status -eq 124 ]]; then
    echo "error: $label timed out after ${secs}s" >&2
  else
    echo "error: $label failed (exit $status)" >&2
  fi
  exit "$status"
}

cd "$SCRIPT_DIR"
echo "build path $BUILD_PATH"
mkdir -p "$BUILD_PATH"

run "compile (local)" 1000 arduino-cli compile -v --fqbn "$FQBN" --build-path "$BUILD_PATH" "$SKETCH"
sync

echo "==> upload + monitor on $PI_HOST"
# -t: allocate a TTY so serial monitor is interactive
ssh -t "$PI_HOST" "$PI_ARDUINO_DIR/upload-on-pi.sh" "$FQBN" "$NAME"
