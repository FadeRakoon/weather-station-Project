import '@mdi/font/css/materialdesignicons.css'
import 'vuetify/styles'

import { createVuetify } from 'vuetify'

const lightMode = {
  dark: false,
  colors: {
    primary: "#ff5500",
    onPrimary: "#ffffff",
    secondary: "#5200ff",
    onSecondary: "#ffffff",
    tertiary: "#00BCD4",
    onTertiary: "#ffffff",
    background: "#ffffff",
    onBackground: "#1c1c1c",
    surface: "#f5f5f5",
    onSurface: "#1c1c1c",
    surfaceVariant: "#e8e8e8",
    onSurfaceVariant: "#8e8e8e",
    error: "#ff5500",
    onError: "#ffffff",
    success: "#c0fe0c",
    "onSuccess": "#000000",
    warning: "#ff5500",
    info: "#5200ff",
    "onInfo": "#ffffff",
    outline: "#8e8e8e",
    outlineVariant: "#d4d4d4",
    shadow: "#000000",
    scrim: "#000000",
  },
}

const darkMode = {
  dark: true,
  colors: {
    primary: "#c0fe0c",
    onPrimary: "#000000",
    secondary: "#5200ff",
    onSecondary: "#ffffff",
    tertiary: "#00BCD4",
    onTertiary: "#ffffff",
    background: "#000000",
    onBackground: "#ffffff",
    surface: "#1c1c1c",
    onSurface: "#ffffff",
    surfaceVariant: "#2a2a2a",
    onSurfaceVariant: "#8e8e8e",
    error: "#ff5500",
    onError: "#000000",
    success: "#c0fe0c",
    "onSuccess": "#000000",
    warning: "#ff5500",
    info: "#5200ff",
    "onInfo": "#ffffff",
    outline: "#8e8e8e",
    outlineVariant: "#4a4a4a",
    shadow: "#000000",
    scrim: "#000000",
  },
}

export default createVuetify({
  theme: {
    defaultTheme: 'darkMode',
    themes: {
      lightMode,
      darkMode 
    },
  } 
})