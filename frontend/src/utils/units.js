export const celsiusToFahrenheit = (c) => {
  if (c === null || c === undefined || isNaN(c)) return null;
  return (c * 9/ 5) + 32;
};

export const fahrenheitToCelsius = (f) => {
  if (f === null || f === undefined || isNaN(f)) return null;
  return (f - 32) * 5 / 9;
};

export const metersToFeet = (m) => {
  if (m === null || m === undefined || isNaN(m)) return null;
  return m * 3.28084;
};

export const feetToMeters = (ft) => {
  if (ft === null || ft === undefined || isNaN(ft)) return null;
  return ft / 3.28084;
};

export const hPaToInHg = (hPa) => {
  if (hPa === null || hPa === undefined || isNaN(hPa)) return null;
  return hPa / 33.8639;
};

export const inHgToHPa = (inHg) => {
  if (inHg === null || inHg === undefined || isNaN(inHg)) return null;
  return inHg * 33.8639;
};

export const hPaToPSI = (hPa) => {
  if (hPa === null || hPa === undefined || isNaN(hPa)) return null;
  return hPa * 0.0145038;
};

export const psiToHPa = (psi) => {
  if (psi === null || psi === undefined || isNaN(psi)) return null;
  return psi / 0.0145038;
};

export const formatTemperature = (value, unit = 'C') => {
  if (value === null || value === undefined || isNaN(value)) return '--';
  const formatted = unit === 'F' ? celsiusToFahrenheit(value) : value;
  return `${formatted.toFixed(1)}°${unit}`;
};

export const formatHumidity = (value) => {
  if (value === null || value === undefined || isNaN(value)) return '--';
  return `${value.toFixed(1)}%`;
};

export const formatPressure = (value, unit = 'hPa') => {
  if (value === null || value === undefined || isNaN(value)) return '--';
  if (unit === 'psi') {
    return `${hPaToPSI(value).toFixed(2)} psi`;
  }
  return `${value.toFixed(1)} hPa`;
};

export const formatAltitude = (value, unit = 'm') => {
  if (value === null || value === undefined || isNaN(value)) return '--';
  if (unit === 'ft') {
    return `${metersToFeet(value).toFixed(1)} ft`;
  }
  return `${value.toFixed(1)} m`;
};

export const formatSoilMoisture = (value) => {
  if (value === null || value === undefined || isNaN(value)) return '--';
  return `${value.toFixed(1)}%`;
};

export const formatRSSI = (value) => {
  if (value === null || value === undefined || isNaN(value)) return '--';
  return `${value} dBm`;
};

export const signalQuality = (rssi) => {
  if (rssi === null || rssi === undefined || isNaN(rssi)) return 'unknown';
  if (rssi >= -50) return 'excellent';
  if (rssi >= -60) return 'good';
  if (rssi >= -70) return 'fair';
  return 'weak';
};

export const signalColor = (rssi) => {
  const quality = signalQuality(rssi);
  switch (quality) {
    case 'excellent': return '#00E676';
    case 'good': return '#76FF03';
    case 'fair': return '#FFEB3B';
    case 'weak': return '#FF5722';
    default: return '#9E9E9E';
  }
};