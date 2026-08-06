# Repository Structure

- `arduino/` contains Arduino sketches, hardware-control code, and local Arduino tooling.
- `web/` contains the Vue 3 + Vite SPA frontend for controlling the LED strip. Compiled to static files and deployed to GitHub Pages.
- `api/openapi.yaml` describes the REST API endpoints used by the frontend to communicate with the Arduino.
- `.github/workflows/` contains CI/CD workflows: `deploy-pages.yml` (production deploy to GitHub Pages on main branch pushes) and `pr-check.yml` (type-check + build on PRs).

# Development Guidelines

- Keep device-specific code and scripts under `arduino/`.
- Keep frontend code under `web/`.
- Treat `api/openapi.yaml` as the contract between the frontend and the Arduino-facing API.
- The Arduino web server now serves only the REST API endpoints (`/api/strip`, `/api/strip/toggle`) plus a minimal landing page at `/` showing the device IP and linking to the GitHub Pages frontend.
- After changing files in `web/`, run `npm run build` from `web/` to verify the SPA compiles cleanly.
- Update this file when the repository structure or responsibilities change.

# Frontend deployment

The Vue SPA is deployed to GitHub Pages automatically via `.github/workflows/deploy-pages.yml`:
- Trigger: push to `main` that changes `web/**` or the workflow file.
- URL: https://nolanpro.github.io/arduino-lights/
- For local development, run `npm run dev` from `web/`.

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
