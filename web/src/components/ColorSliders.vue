<script setup lang="ts">
import { computed } from 'vue'

const props = defineProps<{
  r: number
  g: number
  b: number
}>()

const emit = defineEmits<{
  (e: 'update:color', color: { r: number; g: number; b: number }): void
}>()

const previewColor = computed(() => `rgb(${props.r}, ${props.g}, ${props.b})`)

function onRInput(e: Event) {
  const target = e.target as HTMLInputElement
  emit('update:color', { r: parseInt(target.value), g: props.g, b: props.b })
}

function onGInput(e: Event) {
  const target = e.target as HTMLInputElement
  emit('update:color', { r: props.r, g: parseInt(target.value), b: props.b })
}

function onBInput(e: Event) {
  const target = e.target as HTMLInputElement
  emit('update:color', { r: props.r, g: props.g, b: parseInt(target.value) })
}
</script>

<template>
  <div class="color-sliders">
    <div class="slider-row">
      <span class="slider-label">R</span>
      <span class="slider-value">{{ r }}</span>
      <input
        type="range"
        min="0"
        max="255"
        :value="r"
        @input="onRInput"
        class="color-slider slider-red"
      />
    </div>
    <div class="slider-row">
      <span class="slider-label">G</span>
      <span class="slider-value">{{ g }}</span>
      <input
        type="range"
        min="0"
        max="255"
        :value="g"
        @input="onGInput"
        class="color-slider slider-green"
      />
    </div>
    <div class="slider-row">
      <span class="slider-label">B</span>
      <span class="slider-value">{{ b }}</span>
      <input
        type="range"
        min="0"
        max="255"
        :value="b"
        @input="onBInput"
        class="color-slider slider-blue"
      />
    </div>
    <div class="preview-section">
      <div class="color-preview" :style="{ background: previewColor }" />
    </div>
  </div>
</template>

<style scoped>
.color-sliders {
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
  width: 100%;
}

.slider-row {
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.slider-label {
  min-width: 1.2rem;
  font-weight: 600;
  font-size: 0.85rem;
  font-variant-numeric: tabular-nums;
}

.slider-value {
  min-width: 2.2rem;
  font-size: 0.85rem;
  color: #aaa;
  font-variant-numeric: tabular-nums;
  text-align: right;
}

.color-slider {
  flex: 1;
  -webkit-appearance: none;
  appearance: none;
  height: 6px;
  border-radius: 3px;
  outline: none;
  cursor: pointer;
}

.slider-red {
  background: linear-gradient(to right, #000, #e53935);
}

.slider-green {
  background: linear-gradient(to right, #000, #43a047);
}

.slider-blue {
  background: linear-gradient(to right, #000, #1e88e5);
}

.color-slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 20px;
  height: 20px;
  border-radius: 50%;
  border: 2px solid #222;
  cursor: pointer;
  transition: background 0.2s;
}

.color-slider::-moz-range-thumb {
  width: 20px;
  height: 20px;
  border-radius: 50%;
  border: 2px solid #222;
  cursor: pointer;
}

.slider-red::-webkit-slider-thumb {
  background: #e53935;
}

.slider-red::-moz-range-thumb {
  background: #e53935;
}

.slider-red::-webkit-slider-thumb:hover {
  background: #ef5350;
}

.slider-green::-webkit-slider-thumb {
  background: #43a047;
}

.slider-green::-moz-range-thumb {
  background: #43a047;
}

.slider-green::-webkit-slider-thumb:hover {
  background: #66bb6a;
}

.slider-blue::-webkit-slider-thumb {
  background: #1e88e5;
}

.slider-blue::-moz-range-thumb {
  background: #1e88e5;
}

.slider-blue::-webkit-slider-thumb:hover {
  background: #42a5f5;
}

.preview-section {
  display: flex;
  justify-content: center;
  margin-top: 0.25rem;
}

.color-preview {
  width: 48px;
  height: 48px;
  border-radius: 8px;
  border: 2px solid #444;
  transition: background 0.1s ease;
}
</style>