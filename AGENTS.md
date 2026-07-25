# Repository Structure

- `arduino/` contains Arduino sketches, hardware-control code, and local Arduino tooling.
- `web/` is reserved for the future Vue frontend hosted with GitHub Pages.
- `api/openapi.yaml` is reserved for the OpenAPI description of the endpoints used by the frontend to communicate with the Arduino.

# Development Guidelines

- Keep device-specific code and scripts under `arduino/`.
- Keep frontend code under `web/`.
- Treat `api/openapi.yaml` as the contract between the frontend and the Arduino-facing API.
- Update this file when the repository structure or responsibilities change.
