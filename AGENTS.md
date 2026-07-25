# Repository Structure

- `arduino/` contains Arduino sketches, hardware-control code, and local Arduino tooling.
- `web/` is reserved for the future Vue frontend hosted with GitHub Pages.
- `api/openapi.yaml` is reserved for the OpenAPI description of the endpoints used by the frontend to communicate with the Arduino.

# Development Guidelines

- Keep device-specific code and scripts under `arduino/`.
- Keep frontend code under `web/`.
- Treat `api/openapi.yaml` as the contract between the frontend and the Arduino-facing API.
- Update this file when the repository structure or responsibilities change.

# Raspberry Pi workflow (`nolan-pi`)

Edit and compile on the Mac; upload/monitor on the Pi (board is plugged into the Pi). No GitHub push needed.

- **Build/upload:** from `arduino/`, run `./run.sh <SketchName>` — compiles locally, then SSHs to the Pi to upload and open the serial monitor.
- **SSH:** `ssh nolan@nolan-pi` (LAN or Tailscale). Override with `PI_HOST` if needed.
- **Live mount:** the Pi mounts this Mac folder at `/home/nolan/src/arduino` via SSHFS over Tailscale (`PI_REPO` in `run.sh`).
- **Boot service:** `arduino-sshfs.service` (enabled) remounts after `tailscaled` is up.
- **Requires:** Mac awake with Remote Login on, Tailscale connected on both machines (`nolans-macbook-air` ↔ `nolan-pi`), board connected to the Pi.

Useful commands on the Pi:

```bash
systemctl status arduino-sshfs
sudo systemctl restart arduino-sshfs
ls ~/src/arduino
```

Mount details live only on the Pi (`/etc/systemd/system/arduino-sshfs.service`, SSH host `mac-arduino` in `~/.ssh/config`).
