# Arduino Lights — Frontend

Vue 3 SPA for controlling an Arduino LED strip over Wi-Fi.

## Local Development

```bash
# Install dependencies
npm install

# Start dev server (hot-reload on localhost:5173)
npm run dev

# Type-check and build for production
npm run build

# Preview production build
npm run preview
```

## Connecting to the Arduino

When you open the app, you'll be prompted to enter the Arduino's IP address
(shown on the Arduino's built-in LED matrix). The IP is saved in `localStorage`
so you only need to enter it once.

## Environment Variables

| Variable | Description |
|---|---|
| `VITE_API_URL` | Default Arduino API URL (e.g. `http://192.168.1.42`) |

## Deployment

The app is automatically deployed to GitHub Pages on every push to `main` that
changes files in `web/` or the deploy workflow.