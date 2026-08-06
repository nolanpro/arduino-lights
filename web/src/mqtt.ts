import mqtt from 'mqtt'
import type { MqttClient, IClientPublishOptions } from 'mqtt'
import type { StripState, MqttCredentials } from './types'

const BROKER_URL = 'wss://157f4ec42861480695b95d99a47d8ae4.s1.eu.hivemq.cloud:8884/mqtt'

const COMMAND_TOPIC = 'arduino/in'
const STATE_TOPIC = 'arduino/out'

const CREDENTIALS_KEY = 'arduino-lights-mqtt-credentials'

let client: MqttClient | null = null
let stateCallbacks: Array<(state: StripState) => void> = []
let reconnectTimer: ReturnType<typeof setTimeout> | null = null

// ── Credentials storage ───────────────────────────────────────────────────

export function getStoredCredentials(): MqttCredentials | null {
  const raw = localStorage.getItem(CREDENTIALS_KEY)
  if (!raw) return null
  try {
    return JSON.parse(raw) as MqttCredentials
  } catch {
    return null
  }
}

export function setStoredCredentials(creds: MqttCredentials): void {
  localStorage.setItem(CREDENTIALS_KEY, JSON.stringify(creds))
}

export function clearStoredCredentials(): void {
  localStorage.removeItem(CREDENTIALS_KEY)
}

// ── Connection status ─────────────────────────────────────────────────────

export type MqttStatus = 'disconnected' | 'connecting' | 'connected' | 'error'

let connectionStatus: MqttStatus = 'disconnected'
let statusCallbacks: Array<(status: MqttStatus) => void> = []

function notifyStatus(newStatus: MqttStatus) {
  connectionStatus = newStatus
  for (const cb of statusCallbacks) {
    cb(newStatus)
  }
}

export function onStatusChange(cb: (status: MqttStatus) => void): () => void {
  statusCallbacks.push(cb)
  // Immediately notify with current status
  cb(connectionStatus)
  return () => {
    statusCallbacks = statusCallbacks.filter((fn) => fn !== cb)
  }
}

export function getMqttStatus(): MqttStatus {
  return connectionStatus
}

// ── Connection ────────────────────────────────────────────────────────────

export function connectMqtt(creds: MqttCredentials): Promise<void> {
  return new Promise((resolve, reject) => {
    if (client) {
      client.end(true)
      client = null
    }

    if (reconnectTimer) {
      clearTimeout(reconnectTimer)
      reconnectTimer = null
    }

    notifyStatus('connecting')

    client = mqtt.connect(BROKER_URL, {
      username: creds.username,
      password: creds.password,
      clean: true,
      reconnectPeriod: 5000,
      connectTimeout: 10000,
      keepalive: 60,
    })

    client.on('connect', () => {
      notifyStatus('connected')
      // Subscribe to state topic
      client?.subscribe(STATE_TOPIC, { qos: 1 }, (err: Error | null) => {
        if (err) {
          console.error('Failed to subscribe to state topic:', err)
        }
      })
      resolve()
    })

    client.on('message', (topic: string, payload: Buffer) => {
      if (topic === STATE_TOPIC) {
        try {
          const state = JSON.parse(payload.toString()) as StripState
          for (const cb of stateCallbacks) {
            cb(state)
          }
        } catch (e) {
          console.error('Failed to parse MQTT state message:', e)
        }
      }
    })

    client.on('error', (err: Error) => {
      console.error('MQTT error:', err)
      notifyStatus('error')
      reject(err)
    })

    client.on('close', () => {
      notifyStatus('disconnected')
    })

    client.on('reconnect', () => {
      notifyStatus('connecting')
    })
  })
}

export function disconnectMqtt(): void {
  if (reconnectTimer) {
    clearTimeout(reconnectTimer)
    reconnectTimer = null
  }
  if (client) {
    client.end(true)
    client = null
  }
  stateCallbacks = []
  notifyStatus('disconnected')
}

// ── State subscriptions ──────────────────────────────────────────────────

export function onStateUpdate(cb: (state: StripState) => void): () => void {
  stateCallbacks.push(cb)
  // Return unsubscribe function
  return () => {
    stateCallbacks = stateCallbacks.filter((fn) => fn !== cb)
  }
}

// ── Publishing commands ───────────────────────────────────────────────────

function publish(topic: string, payload: Record<string, unknown>): Promise<void> {
  if (!client || !client.connected) {
    return Promise.reject(new Error('MQTT not connected'))
  }
  const json = JSON.stringify(payload)
  const opts: IClientPublishOptions = { qos: 1 }
  return client.publishAsync(topic, json, opts).then(() => undefined)
}

// ── API commands ──────────────────────────────────────────────────────────

export function publishCommand(action: string, params?: Record<string, unknown>): Promise<void> {
  const payload: Record<string, unknown> = { action }
  if (params) {
    Object.assign(payload, params)
  }
  return publish(COMMAND_TOPIC, payload)
}

export function toggleStrip(): Promise<void> {
  return publishCommand('toggle')
}

export function setStripOn(): Promise<void> {
  return publishCommand('on')
}

export function setStripOff(): Promise<void> {
  return publishCommand('off')
}

export function setScene(name: string): Promise<void> {
  return publishCommand('set_scene', { scene: name })
}

export function setBrightness(value: number): Promise<void> {
  return publishCommand('set_brightness', { brightness: value })
}

export function setColor(r: number, g: number, b: number): Promise<void> {
  return publishCommand('set_color', { r, g, b })
}

export function setPixel(index: number): Promise<void> {
  return publishCommand('set_pixel', { index })
}

export function requestState(): Promise<void> {
  return publishCommand('get_state')
}