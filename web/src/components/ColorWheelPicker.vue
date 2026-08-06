<script setup lang="ts">
import { ref, watch } from 'vue'
import { VueColorWheel } from 'vue-color-wheel'
import type { Harmony } from 'vue-color-wheel'

const props = defineProps<{
  r: number
  g: number
  b: number
}>()

const emit = defineEmits<{
  (e: 'update:color', color: { r: number; g: number; b: number }): void
}>()

function rgbToHex(r: number, g: number, b: number): string {
  return '#' + [r, g, b].map(c => c.toString(16).padStart(2, '0')).join('')
}

const wheelColor = ref(rgbToHex(props.r, props.g, props.b))

watch(
  () => [props.r, props.g, props.b],
  ([nr, ng, nb]) => {
    wheelColor.value = rgbToHex(nr, ng, nb)
  },
)

function colorStringToRgb(input: string): { r: number; g: number; b: number } {
  // Try hex: #ff8040 or ff8040
  const hexMatch = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(input)
  if (hexMatch) {
    return {
      r: parseInt(hexMatch[1], 16),
      g: parseInt(hexMatch[2], 16),
      b: parseInt(hexMatch[3], 16),
    }
  }

  // Try rgb(r, g, b) format
  const rgbMatch = /^rgb\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)$/i.exec(input)
  if (rgbMatch) {
    return {
      r: Math.min(255, Math.max(0, parseInt(rgbMatch[1], 10))),
      g: Math.min(255, Math.max(0, parseInt(rgbMatch[2], 10))),
      b: Math.min(255, Math.max(0, parseInt(rgbMatch[3], 10))),
    }
  }

  return { r: 255, g: 255, b: 255 }
}

function onColorUpdate(hex: string) {
  const color = colorStringToRgb(hex)
  emit('update:color', color)
}

function onChange(colors: Harmony[]) {
  if (colors.length > 0) {
    const color = colorStringToRgb(colors[0].rgb)
    emit('update:color', color)
  }
}
</script>

<template>
  <div class="color-picker-wrapper">
    <VueColorWheel
      v-model:color="wheelColor"
      wheel="aurora"
      harmony="monochromatic"
      :radius="120"
      :showBrightness="false"
      @update:color="onColorUpdate"
      @change="onChange"
    />
  </div>
</template>

<style scoped>
.color-picker-wrapper {
  display: flex;
  justify-content: center;
  align-items: center;
}
</style>