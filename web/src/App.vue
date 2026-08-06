<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { connectMqtt, disconnectMqtt, getStoredCredentials, setStoredCredentials, clearStoredCredentials, onStateUpdate, onStatusChange, toggleStrip, setScene, setPixel, setBrightness, setColor, requestState } from './mqtt'
import type { StripState, MqttStatus } from './types'
import ColorSliders from './components/ColorSliders.vue'

const on = ref(false)
const busy = ref(false)
const sceneBusy = ref(false)
const pixelBusy = ref(false)
const brightnessBusy = ref(false)
const error = ref('')
const creds = ref(getStoredCredentials() ?? { username: '', password: '' })
const showConfig = ref(!getStoredCredentials())
const scene = ref('')
const scenes = ref<string[]>([])
const pixelIndex = ref('')
const brightness = ref(200)
const brightnessBefore = ref(200)
const colorR = ref(255)
const colorG = ref(255)
const colorB = ref(255)
const colorBefore = ref({ r: 255, g: 255, b: 255 })
const mqttStatus = ref<MqttStatus>('disconnected')

let unsubState: (() => void) | null = null
let unsubStatus: (() => void) | null = null

const availableScenes = [
  'blue-noise',
  'rainbow-chase',
  'single-led',
  'solid-color',
  'rainbow',
  'rainbow-with-glitter',
  'confetti',
  'sinelon',
  'juggle',
  'bpm',
]

function handleState(state: StripState) {
  on.value = state.on
  brightness.value = state.brightness
  brightnessBefore.value = state.brightness
  scene.value = state.scene
  colorR.value = state.r
  colorG.value = state.g
  colorB.value = state.b
  colorBefore.value = { r: state.r, g: state.g, b: state.b }
}

async function connect(creds: { username: string; password: string }) {
  try {
    error.value = ''
    mqttStatus.value = 'connecting'
    await connectMqtt(creds)
    setStoredCredentials(creds)
    showConfig.value = false
    scenes.value = availableScenes
    // Request initial state from the Arduino
    await requestState()
  } catch (e: unknown) {
    mqttStatus.value = 'error'
    error.value = e instanceof Error ? e.message : 'Failed to connect to MQTT broker'
    showConfig.value = true
  }
}

async function toggle() {
  busy.value = true
  try {
    error.value = ''
    await toggleStrip()
  } catch (e: unknown) {
    error.value = e instanceof Error ? e.message : 'Toggle failed'
  } finally {
    busy.value = false
  }
}

async function activateScene(name: string) {
  sceneBusy.value = true
  try {
    error.value = ''
    await setScene(name)
    scene.value = name
    // Clear pixel index when switching to non-single-led scenes
    if (name !== 'single-led') {
      pixelIndex.value = ''
    }
  } catch (e: unknown) {
    error.value = e instanceof Error ? e.message : `Failed to set scene`
  } finally {
    sceneBusy.value = false
  }
}

async function sendPixel() {
  const idx = parseInt(pixelIndex.value, 10)
  if (isNaN(idx)) {
    error.value = 'Enter a valid LED number'
    return
  }
  pixelBusy.value = true
  try {
    error.value = ''
    await setPixel(idx)
    scene.value = 'single-led'
  } catch (e: unknown) {
    error.value = e instanceof Error ? e.message : 'Failed to set pixel'
  } finally {
    pixelBusy.value = false
  }
}

let brightnessTimeout: ReturnType<typeof setTimeout> | null = null

function onBrightnessInput() {
  if (brightnessTimeout) clearTimeout(brightnessTimeout)
  brightnessTimeout = setTimeout(sendBrightness, 150)
}

async function sendBrightness() {
  brightnessBusy.value = true
  try {
    error.value = ''
    await setBrightness(brightness.value)
    brightnessBefore.value = brightness.value
  } catch (e: unknown) {
    error.value = e instanceof Error ? e.message : 'Failed to set brightness'
    // Revert to last known good value
    brightness.value = brightnessBefore.value
  } finally {
    brightnessBusy.value = false
  }
}

let colorTimeout: ReturnType<typeof setTimeout> | null = null

function onColorInput() {
  if (colorTimeout) clearTimeout(colorTimeout)
  colorTimeout = setTimeout(sendColor, 150)
}

async function sendColor() {
  try {
    error.value = ''
    await setColor(colorR.value, colorG.value, colorB.value)
    colorBefore.value = { r: colorR.value, g: colorG.value, b: colorB.value }
  } catch (e: unknown) {
    error.value = e instanceof Error ? e.message : 'Failed to set color'
    colorR.value = colorBefore.value.r
    colorG.value = colorBefore.value.g
    colorB.value = colorBefore.value.b
  }
}

function saveCredentials() {
  const trimmedUser = creds.value.username.trim()
  const trimmedPass = creds.value.password.trim()
  if (!trimmedUser || !trimmedPass) {
    error.value = 'Username and password are required'
    return
  }
  connect({ username: trimmedUser, password: trimmedPass })
}

function disconnect() {
  disconnectMqtt()
  clearStoredCredentials()
  showConfig.value = true
  error.value = ''
}

onMounted(() => {
  // Subscribe to state updates
  unsubState = onStateUpdate(handleState)

  // Subscribe to connection status changes
  unsubStatus = onStatusChange((status) => {
    mqttStatus.value = status
    if (status === 'error' || status === 'disconnected') {
      showConfig.value = true
    }
  })

  // Auto-connect if credentials are stored
  const stored = getStoredCredentials()
  if (stored) {
    connect(stored)
  }
})

onUnmounted(() => {
  if (unsubState) unsubState()
  if (unsubStatus) unsubStatus()
  disconnectMqtt()
})
</script>

<template>
  <div class="container">
    <h1>Strip Lights</h1>

    <div v-if="showConfig" class="config-card">
      <h2>MQTT Credentials</h2>
      <p class="hint">Enter your HiveMQ Cloud MQTT credentials to connect to the broker.</p>
      <form @submit.prevent="saveCredentials" class="config-form">
        <input
          v-model="creds.username"
          type="text"
          placeholder="MQTT username"
          class="url-input"
          autofocus
        />
        <input
          v-model="creds.password"
          type="password"
          placeholder="MQTT password"
          class="url-input"
        />
        <button type="submit" class="btn-primary" :disabled="!creds.username.trim() || !creds.password.trim()">
          Connect
        </button>
      </form>
      <p v-if="mqttStatus === 'connecting'" class="status-text">Connecting...</p>
      <p v-if="error" class="error-text">{{ error }}</p>
    </div>

    <template v-else>
      <p class="status">
        LED strip: <strong :class="on ? 'on' : 'off'">{{ on ? 'ON' : 'OFF' }}</strong>
        <span class="mqtt-indicator" :class="mqttStatus">MQTT {{ mqttStatus }}</span>
      </p>

      <button
        :class="['toggle-btn', on ? 'on' : 'off']"
        :disabled="busy"
        @click="toggle"
      >
        {{ on ? 'Turn Off' : 'Turn On' }}
      </button>

      <p v-if="error" class="error-text">{{ error }}</p>

      <div class="brightness-section">
        <h2>Brightness</h2>
        <div class="brightness-control">
          <span class="brightness-label">{{ brightness }}</span>
          <input
            type="range"
            min="0"
            max="255"
            v-model.number="brightness"
            @input="onBrightnessInput"
            class="brightness-slider"
            :disabled="brightnessBusy"
          />
        </div>
      </div>

      <div class="scenes-section" v-if="scenes.length">
        <h2>Scenes</h2>
        <div class="scenes-grid">
          <button
            v-for="s in scenes"
            :key="s"
            :class="['scene-btn', { active: scene === s }]"
            :disabled="sceneBusy"
            @click="activateScene(s)"
          >
            {{ s }}
          </button>
        </div>
      </div>

      <div v-if="scene === 'solid-color'" class="hue-section">
        <h2>Color</h2>
        <ColorSliders
          :r="colorR"
          :g="colorG"
          :b="colorB"
          @update:color="(c) => { colorR = c.r; colorG = c.g; colorB = c.b; onColorInput() }"
        />
        <p class="hue-value-display">
          RGB: <strong>{{ colorR }}, {{ colorG }}, {{ colorB }}</strong>
        </p>
      </div>

      <div v-if="scene === 'single-led'" class="pixel-section">
        <h2>Single LED</h2>
        <form @submit.prevent="sendPixel" class="pixel-form">
          <input
            v-model="pixelIndex"
            type="number"
            min="0"
            placeholder="LED number"
            class="pixel-input"
          />
          <button type="submit" class="btn-primary" :disabled="pixelBusy || pixelIndex === ''">
            Set LED
          </button>
        </form>
      </div>

      <button class="link-btn" @click="disconnect">Disconnect</button>
    </template>
  </div>
</template>

<style>
* {
  box-sizing: border-box;
}

body {
  font-family: system-ui, -apple-system, sans-serif;
  margin: 0;
  padding: 2rem 1rem;
  background: #111;
  color: #eee;
  min-height: 100vh;
}

.container {
  max-width: 400px;
  margin: 0 auto;
  text-align: center;
}

h1 {
  font-size: 1.5rem;
  margin: 0 0 1.5rem;
  font-weight: 600;
}

h2 {
  font-size: 1.1rem;
  margin: 0 0 0.75rem;
  font-weight: 500;
}

.status {
  margin: 0 0 1.5rem;
  color: #aaa;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 0.75rem;
  flex-wrap: wrap;
}

.status strong {
  font-weight: 600;
}

.status .on {
  color: #4caf50;
}

.status .off {
  color: #888;
}

.mqtt-indicator {
  font-size: 0.75rem;
  padding: 0.15em 0.5em;
  border-radius: 4px;
  background: #333;
  color: #888;
}

.mqtt-indicator.connected {
  background: #1b5e20;
  color: #a5d6a7;
}

.mqtt-indicator.connecting {
  background: #e65100;
  color: #ffcc80;
}

.mqtt-indicator.error {
  background: #b71c1c;
  color: #ef9a9a;
}

.status-text {
  color: #aaa;
  font-size: 0.85rem;
  margin: 0.5rem 0 0;
}

.config-card {
  background: #1e1e1e;
  border: 1px solid #333;
  border-radius: 12px;
  padding: 1.5rem;
  text-align: left;
}

.config-card .hint {
  color: #888;
  font-size: 0.85rem;
  margin: 0 0 1rem;
  line-height: 1.4;
}

.config-card .hint code {
  background: #2a2a2a;
  padding: 0.1em 0.4em;
  border-radius: 4px;
  font-size: 0.85em;
}

.config-form {
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
}

.url-input {
  width: 100%;
  padding: 0.75rem;
  border: 1px solid #444;
  border-radius: 8px;
  background: #2a2a2a;
  color: #eee;
  font-size: 1rem;
  font-family: monospace;
  outline: none;
  transition: border-color 0.2s;
}

.url-input:focus {
  border-color: #4caf50;
}

.btn-primary {
  padding: 0.75rem;
  border: none;
  border-radius: 8px;
  background: #2e7d32;
  color: #fff;
  font-size: 1rem;
  font-weight: 500;
  cursor: pointer;
  transition: background 0.2s;
}

.btn-primary:hover:not(:disabled) {
  background: #388e3c;
}

.btn-primary:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.toggle-btn {
  font-size: 1.1rem;
  padding: 0.75rem 2rem;
  border: none;
  border-radius: 8px;
  cursor: pointer;
  color: #fff;
  font-weight: 500;
  transition: background 0.2s, opacity 0.2s;
}

.toggle-btn:disabled {
  opacity: 0.5;
  cursor: wait;
}

.toggle-btn.on {
  background: #2e7d32;
}

.toggle-btn.on:hover:not(:disabled) {
  background: #388e3c;
}

.toggle-btn.off {
  background: #555;
}

.toggle-btn.off:hover:not(:disabled) {
  background: #666;
}

.error-text {
  color: #ef5350;
  margin: 1rem 0 0;
  font-size: 0.9rem;
}

.link-btn {
  display: inline-block;
  margin-top: 1.5rem;
  padding: 0;
  background: none;
  border: none;
  color: #888;
  font-size: 0.85rem;
  cursor: pointer;
  text-decoration: underline;
}

.link-btn:hover {
  color: #aaa;
}

.scenes-section {
  margin-top: 2rem;
  padding-top: 1.5rem;
  border-top: 1px solid #333;
}

.brightness-section {
  margin-top: 1.5rem;
}

.brightness-control {
  display: flex;
  align-items: center;
  gap: 0.75rem;
}

.brightness-label {
  min-width: 2.2rem;
  font-size: 0.9rem;
  color: #aaa;
  font-variant-numeric: tabular-nums;
  text-align: right;
}

.brightness-slider {
  flex: 1;
  -webkit-appearance: none;
  appearance: none;
  height: 6px;
  border-radius: 3px;
  background: #333;
  outline: none;
  cursor: pointer;
}

.brightness-slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 20px;
  height: 20px;
  border-radius: 50%;
  background: #4caf50;
  border: 2px solid #2e7d32;
  cursor: pointer;
  transition: background 0.2s;
}

.brightness-slider::-webkit-slider-thumb:hover {
  background: #66bb6a;
}

.brightness-slider::-moz-range-thumb {
  width: 20px;
  height: 20px;
  border-radius: 50%;
  background: #4caf50;
  border: 2px solid #2e7d32;
  cursor: pointer;
}

.brightness-slider:disabled {
  opacity: 0.5;
  cursor: wait;
}

.brightness-slider:disabled::-webkit-slider-thumb {
  cursor: wait;
}

.hue-section {
  margin-top: 2rem;
  padding-top: 1.5rem;
  border-top: 1px solid #333;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.75rem;
}

.hue-value-display {
  margin: 0;
  font-size: 0.9rem;
  color: #aaa;
}

.hue-value-display strong {
  font-variant-numeric: tabular-nums;
  color: #eee;
}

.scenes-grid {
  display: flex;
  flex-wrap: wrap;
  gap: 0.5rem;
  justify-content: center;
}

.scene-btn {
  padding: 0.5rem 1rem;
  border: 1px solid #555;
  border-radius: 8px;
  background: #2a2a2a;
  color: #ccc;
  font-size: 0.9rem;
  cursor: pointer;
  transition: all 0.2s;
  text-transform: capitalize;
}

.scene-btn:hover:not(:disabled) {
  background: #3a3a3a;
  border-color: #888;
}

.scene-btn.active {
  background: #1b5e20;
  border-color: #4caf50;
  color: #fff;
}

.scene-btn:disabled {
  opacity: 0.5;
  cursor: wait;
}

.pixel-section {
  margin-top: 2rem;
  padding-top: 1.5rem;
  border-top: 1px solid #333;
}

.pixel-form {
  display: flex;
  gap: 0.5rem;
  justify-content: center;
}

.pixel-input {
  width: 120px;
  padding: 0.75rem;
  border: 1px solid #444;
  border-radius: 8px;
  background: #2a2a2a;
  color: #eee;
  font-size: 1rem;
  font-family: monospace;
  outline: none;
  transition: border-color 0.2s;
}

.pixel-input:focus {
  border-color: #4caf50;
}
</style>