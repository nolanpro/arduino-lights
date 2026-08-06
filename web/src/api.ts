import type { StripStatus, SceneStatus, SceneList, PixelStatus, BrightnessStatus, HueStatus, ColorStatus } from './types'

const STORAGE_KEY = 'arduino-lights-api-url'

function getBaseUrl(): string {
  const envUrl = import.meta.env.VITE_API_URL
  if (envUrl) return envUrl

  const stored = localStorage.getItem(STORAGE_KEY)
  if (stored) return stored

  return ''
}

export function getStoredApiUrl(): string {
  return localStorage.getItem(STORAGE_KEY) ?? ''
}

export function setStoredApiUrl(url: string): void {
  if (url) {
    localStorage.setItem(STORAGE_KEY, url)
  } else {
    localStorage.removeItem(STORAGE_KEY)
  }
}

async function apiFetch<T>(path: string, options?: RequestInit): Promise<T> {
  const base = getBaseUrl()
  if (!base) {
    throw new Error('Arduino IP address not configured')
  }

  // Normalise: strip trailing slash from base, add slash before path
  const url = `${base.replace(/\/+$/, '')}/${path.replace(/^\/+/, '')}`
  const res = await fetch(url, {
    ...options,
    headers: {
      'Content-Type': 'application/json',
      ...options?.headers,
    },
  })

  if (!res.ok) {
    const body = await res.json().catch(() => null)
    throw new Error(
      body?.error ?? `Request failed with status ${res.status}`,
    )
  }

  return res.json()
}

export function getStripStatus(): Promise<StripStatus> {
  return apiFetch<StripStatus>('/api/strip')
}

export function toggleStrip(): Promise<StripStatus> {
  return apiFetch<StripStatus>('/api/strip/toggle', { method: 'POST' })
}

export function getCurrentScene(): Promise<SceneStatus> {
  return apiFetch<SceneStatus>('/api/strip/scene')
}

export function getScenes(): Promise<SceneList> {
  return apiFetch<SceneList>('/api/strip/scenes')
}

export function setScene(name: string): Promise<SceneStatus> {
  return apiFetch<SceneStatus>(`/api/strip/scene/${encodeURIComponent(name)}`, { method: 'POST' })
}

export function setPixel(index: number): Promise<PixelStatus> {
  return apiFetch<PixelStatus>(`/api/strip/pixel/${index}`, { method: 'POST' })
}

export function getBrightness(): Promise<BrightnessStatus> {
  return apiFetch<BrightnessStatus>('/api/strip/brightness')
}

export function setBrightness(value: number): Promise<BrightnessStatus> {
  return apiFetch<BrightnessStatus>('/api/strip/brightness', {
    method: 'POST',
    body: JSON.stringify({ brightness: value }),
  })
}

export function getHue(): Promise<HueStatus> {
  return apiFetch<HueStatus>('/api/strip/hue')
}

export function setHue(value: number): Promise<HueStatus> {
  return apiFetch<HueStatus>('/api/strip/hue', {
    method: 'POST',
    body: JSON.stringify({ hue: value }),
  })
}

export function getColor(): Promise<ColorStatus> {
  return apiFetch<ColorStatus>('/api/strip/color')
}

export function setColor(r: number, g: number, b: number): Promise<ColorStatus> {
  return apiFetch<ColorStatus>('/api/strip/color', {
    method: 'POST',
    body: JSON.stringify({ r, g, b }),
  })
}