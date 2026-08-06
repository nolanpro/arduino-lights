<script setup lang="ts">
import { ref, computed } from 'vue'

const props = defineProps<{
  modelValue: number
}>()

const emit = defineEmits<{
  (e: 'update:modelValue', value: number): void
}>()

const SIZE = 240
const RING_MID_RATIO = 0.9
const RING_MID = SIZE / 2 * RING_MID_RATIO

const wheelRef = ref<HTMLElement | null>(null)
const dragging = ref(false)

const angle = computed(() => props.modelValue * Math.PI / 180)

const dotStyle = computed(() => ({
  transform: `translate(-50%, -50%) translate(${RING_MID * Math.sin(angle.value)}px, ${-RING_MID * Math.cos(angle.value)}px)`,
}))

const selectedColor = computed(() => `hsl(${props.modelValue}, 100%, 50%)`)

function updateHue(e: PointerEvent) {
  const el = wheelRef.value
  if (!el) return
  const rect = el.getBoundingClientRect()
  const cx = rect.left + rect.width / 2
  const cy = rect.top + rect.height / 2
  const dx = e.clientX - cx
  const dy = e.clientY - cy
  const angle = Math.atan2(dx, -dy) // 0 at 12 o'clock, clockwise
  let hue = ((angle * 180 / Math.PI) % 360 + 360) % 360
  emit('update:modelValue', Math.round(hue))
}

function onPointerDown(e: PointerEvent) {
  dragging.value = true
  ;(e.target as HTMLElement).setPointerCapture(e.pointerId)
  updateHue(e)
}

function onPointerMove(e: PointerEvent) {
  if (!dragging.value) return
  updateHue(e)
}

function onPointerUp(_e: PointerEvent) {
  dragging.value = false
}

function onKeyDown(e: KeyboardEvent) {
  let step = 0
  if (e.key === 'ArrowRight' || e.key === 'ArrowUp') step = 1
  else if (e.key === 'ArrowLeft' || e.key === 'ArrowDown') step = -1
  else return

  e.preventDefault()
  const newVal = ((props.modelValue + step) % 360 + 360) % 360
  emit('update:modelValue', newVal)
}
</script>

<template>
  <div
    ref="wheelRef"
    class="hue-wheel"
    :class="{ dragging }"
    :style="{ width: SIZE + 'px', height: SIZE + 'px' }"
    role="slider"
    aria-label="Hue"
    :aria-valuemin="0"
    :aria-valuemax="360"
    :aria-valuenow="modelValue"
    tabindex="0"
    @pointerdown="onPointerDown"
    @pointermove="onPointerMove"
    @pointerup="onPointerUp"
    @pointercancel="onPointerUp"
    @keydown="onKeyDown"
  >
    <div class="hue-wheel-ring" />
    <div class="hue-wheel-dot" :style="dotStyle" />
    <div class="hue-wheel-center">
      <span class="hue-wheel-value">{{ modelValue }}°</span>
    </div>
  </div>
</template>

<style scoped>
.hue-wheel {
  position: relative;
  border-radius: 50%;
  cursor: crosshair;
  user-select: none;
  touch-action: none;
  outline: none;
}

.hue-wheel:focus-visible {
  box-shadow: 0 0 0 3px rgba(76, 175, 80, 0.6);
}

.hue-wheel-ring {
  position: absolute;
  inset: 0;
  border-radius: 50%;
  background: conic-gradient(
    hsl(0, 100%, 50%),
    hsl(30, 100%, 50%),
    hsl(60, 100%, 50%),
    hsl(90, 100%, 50%),
    hsl(120, 100%, 50%),
    hsl(150, 100%, 50%),
    hsl(180, 100%, 50%),
    hsl(210, 100%, 50%),
    hsl(240, 100%, 50%),
    hsl(270, 100%, 50%),
    hsl(300, 100%, 50%),
    hsl(330, 100%, 50%),
    hsl(360, 100%, 50%)
  );
  -webkit-mask: radial-gradient(circle closest-side, transparent calc(80% - 0.5px), #000 80%);
  mask: radial-gradient(circle closest-side, transparent calc(80% - 0.5px), #000 80%);
}

.hue-wheel-dot {
  position: absolute;
  top: 50%;
  left: 50%;
  width: 16px;
  height: 16px;
  border-radius: 50%;
  background: #fff;
  border: 2px solid #222;
  box-shadow: 0 1px 4px rgba(0, 0, 0, 0.5);
  pointer-events: none;
  transition: transform 0.05s ease;
}

.hue-wheel.dragging .hue-wheel-dot {
  transition: none;
}

.hue-wheel-center {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  width: 60%;
  height: 60%;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #1a1a1a;
  pointer-events: none;
}

.hue-wheel-value {
  font-family: 'SF Mono', 'Fira Code', 'Cascadia Code', monospace;
  font-size: 1.1rem;
  font-weight: 600;
  color: v-bind(selectedColor);
  font-variant-numeric: tabular-nums;
}
</style>