#!/bin/bash

# arduino-cli board list
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi Lights.ino
arduino-cli upload -p /dev/cu.usbmodemE072A1E212482 --fqbn arduino:renesas_uno:unor4wifi Lights.ino