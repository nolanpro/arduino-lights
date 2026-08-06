<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { getStripStatus, toggleStrip, getStoredApiUrl, setStoredApiUrl, getCurrentScene, getScenes, setScene, setPixel, getBrightness, setBrightness, getColor, setColor } from './api'
import type { StripStatus } from './types'
import ColorSliders from './components/ColorSliders.vue'

const on = ref(false)
const busy = ref(false)
const sceneBusy = ref(false)
const pixelBusy = ref(false)
const brightnessBusy = ref(false)
const error = ref('')
const apiUrl = ref(getStoredApiUrl())
const showConfig = ref(!apiUrl.value)
const scene = ref('')
const scenes = ref<string[]>([])
const pixelIndex = ref('')
const brightness = ref(200)
const brightnessBefore = ref(200)
const colorR = ref(255)
const colorG = ref(255)
const colorB = ref(255)
const colorBefore = ref({ r: 255, g: 255, b: 255 })

async function refresh() {
  try {
    error.value = ''
    const status: StripStatus = await getStripStatus()
    on.value = status.on
    if (status.brightness !== undefined) {
      brightness.value = status.brightness
      brightnessBefore.value = status.brightness
    }
  } catch (e: unknown) {
    on.value = false
    error.value = e instanceof Error ? e.message : 'Failed to connect'
    showConfig.value = true
  }
}

async function refreshScenes() {
  try {
    const result = await getCurrentScene()
    scene.value = result.scene
  } catch {
    // non-critical
  }
  try {
    const result = await getScenes()
    scenes.value = result.scenes
  } catch {
    // non-critical
  }
}

async function refreshColor() {
  try {
    const result = await getColor()
    colorR.value = result.r
    colorG.value = result.g
    colorB.value = result.b
    colorBefore.value = { r: result.r, g: result.g, b: result.b }
  } catch {
    // non-critical
  }
}

async function refreshBrightness() {
  try {
    const result = await getBrightness()
    brightness.value = result.brightness
    brightnessBefore.value = result.brightness
  } catch {
    // non-critical
  }
}

async function toggle() {
  busy.value = true
  try {
    error.value = ''
    const status: StripStatus = await toggleStrip()
    on.value = status.on
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
    // Refresh color from device when switching to solid-color
    if (name === 'solid-color') {
      await refreshColor()
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

function saveUrl() {
  const trimmed = apiUrl.value.trim()
  setStoredApiUrl(trimmed)
  if (trimmed) {
    showConfig.value = false
    refresh()
    refreshScenes()
    refreshBrightness()
  }
}

onMounted(() => {
  if (apiUrl.value) {
    refresh()
    refreshScenes()
    refreshBrightness()
    refreshColor()
  }
})
</script>

<template>
  <div class="container">
    <h1>Strip Lights</h1>

    <div v-if="showConfig" class="config-card">
      <h2>Arduino IP Address</h2>
      <p class="hint">Enter the IP shown on the Arduino's LED matrix (e.g. <code>192.168.1.42</code>).</p>
      <form @submit.prevent="saveUrl" class="config-form">
        <input
          v-model="apiUrl"
          type="text"
          placeholder="http://192.168.1.42"
          class="url-input"
          autofocus
        />
        <button type="submit" class="btn-primary" :disabled="!apiUrl.trim()">
          Connect
        </button>
      </form>
      <p v-if="error" class="error-text">{{ error }}</p>
    </div>

    <template v-else>
      <p class="status">
        LED strip: <strong :class="on ? 'on' : 'off'">{{ on ? 'ON' : 'OFF' }}</strong>
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

      <button class="link-btn" @click="showConfig = true">Change Arduino IP</button>
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