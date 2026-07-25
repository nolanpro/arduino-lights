#!/bin/bash
# Run on the Pi (via SSH from run.sh). Expects the sketch already compiled into build/.
set -euo pipefail

# Non-interactive SSH sessions may not load the shell profile.
export PATH="$HOME/.local/bin:$PATH"

FQBN=${1:?usage: $0 <FQBN> <SketchName>}
NAME=${2:?usage: $0 <FQBN> <SketchName>}

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
SKETCH=$NAME/$NAME.ino
BUILD_PATH=$SCRIPT_DIR/build/$NAME
HEX=$BUILD_PATH/$NAME.ino.hex

cd "$SCRIPT_DIR"

if [[ ! -f "$HEX" ]]; then
  echo "error: build artifact missing on Pi: $HEX" >&2
  echo "is the SSHFS mount up? (systemctl status arduino-sshfs)" >&2
  exit 1
fi

BOARD_LIST=$(timeout 20 arduino-cli board list)
PORT=$(awk -v fqbn="$FQBN" '$0 ~ fqbn { print $1; exit }' <<<"$BOARD_LIST")
if [[ -z "${PORT:-}" ]]; then
  echo "error: no board found for FQBN $FQBN" >&2
  echo "plug in the Arduino and check: arduino-cli board list" >&2
  exit 1
fi
echo "using port $PORT"

echo "==> upload (timeout 60s)"
timeout 60 arduino-cli upload -v -p "$PORT" --fqbn "$FQBN" --build-path "$BUILD_PATH" "$SKETCH"

echo "==> monitor (Ctrl-C to quit)"
arduino-cli monitor -p "$PORT" -c baudrate=115200
