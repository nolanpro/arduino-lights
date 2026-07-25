#!/bin/bash
set -euo pipefail

FQBN=arduino:renesas_uno:unor4wifi
SKETCH=${1:?usage: $0 <SketchName>}/$1.ino

TIMEOUT_BIN=$(command -v timeout || command -v gtimeout || true)
if [[ -z "$TIMEOUT_BIN" ]]; then
  echo "error: need timeout or gtimeout (brew install coreutils)" >&2
  exit 1
fi

run() {
  local label=$1 secs=$2
  shift 2
  echo "==> $label (timeout ${secs}s)" >&2
  if ! "$TIMEOUT_BIN" "$secs" "$@"; then
    local status=$?
    if [[ $status -eq 124 ]]; then
      echo "error: $label timed out after ${secs}s" >&2
    else
      echo "error: $label failed (exit $status)" >&2
    fi
    exit "$status"
  fi
}

BOARD_LIST=$(run "board list" 20 arduino-cli board list)
PORT=$(awk -v fqbn="$FQBN" '$0 ~ fqbn { print $1; exit }' <<<"$BOARD_LIST")
if [[ -z "${PORT:-}" ]]; then
  echo "error: no board found for FQBN $FQBN" >&2
  echo "plug in the Arduino and check: arduino-cli board list" >&2
  exit 1
fi
echo "using port $PORT"

run "compile" 120 arduino-cli compile --fqbn "$FQBN" "$SKETCH"
run "upload" 60 arduino-cli upload -p "$PORT" --fqbn "$FQBN" "$SKETCH"

echo "==> monitor (Ctrl-C to quit)"
arduino-cli monitor -p "$PORT" -c baudrate=115200
