export interface StripStatus {
  on: boolean
  brightness: number
}

export interface SceneStatus {
  scene: string
}

export interface SceneList {
  scenes: string[]
}

export interface PixelStatus {
  index: number
  scene: string
}

export interface BrightnessStatus {
  brightness: number
}

export interface HueStatus {
  hue: number
}

export interface ColorStatus {
  r: number
  g: number
  b: number
}

export interface ApiError {
  error: string
}

export type RGBColor = { r: number; g: number; b: number }