export interface StripState {
  on: boolean
  brightness: number
  scene: string
  r: number
  g: number
  b: number
}

export interface MqttCredentials {
  username: string
  password: string
}

export type MqttStatus = 'disconnected' | 'connecting' | 'connected' | 'error'

export type RGBColor = { r: number; g: number; b: number }