<template>
  <v-container fluid class="pa-4">
    <v-row style="max-width: 1400px; margin: 0 auto;">
      <v-col cols="12">
        <v-card class="mb-4" color="surface">
          <v-card-item>
            <div class="d-flex align-center justify-space-between">
              <div>
                <span class="text-h5 text-primary">Weather Station Dashboard</span>
                <div class="text-caption text-onSurfaceVariant mt-1">
                  Real-time sensor data from ESP32
                </div>
              </div>
              <div class="d-flex align-center ga-2">
                <v-chip 
                  :color="connectionStatus === 'connected' ? 'success' : (connectionStatus === 'connecting' ? 'warning' : 'error')" 
                  size="small"
                  variant="flat"
                >
                  <v-icon start icon="mdi:circle" size="8"></v-icon>
                  {{ connectionStatus === 'connected' ? 'Connected' : (connectionStatus === 'connecting' ? 'Connecting...' : 'Disconnected') }}
                </v-chip>
                <span class="text-caption text-onSurfaceVariant">
                  {{ lastUpdate }}
                </span>
              </div>
            </div>
          </v-card-item>
        </v-card>
      </v-col>
    </v-row>

    <v-row style="max-width: 1400px; margin: 0 auto;">
      <v-col cols="12" sm="6" md="4" lg="3" v-for="sensor in sensors" :key="sensor.id">
        <v-card 
          :color="sensor.color" 
          variant="tonal"
          class="h-100"
        >
          <v-card-item>
            <div class="d-flex justify-space-between align-start">
              <div>
                <div class="text-overline text-onSurfaceVariant">{{ sensor.label }}</div>
                <div class="text-h4 text-onSurface font-weight-bold">
                  {{ formatValue(sensor) }}
                </div>
                <div class="text-caption text-onSurfaceVariant">{{ sensor.unit }}</div>
              </div>
              <v-icon :icon="sensor.icon" size="32" class="text-onSurfaceVariant"></v-icon>
            </div>
          </v-card-item>
          <v-card-text class="pa-0 pt-2">
            <div :id="'chart-' + sensor.id" style="height: 60px;"></div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <v-row style="max-width: 1400px; margin: 0 auto;" class="mt-4">
      <v-col cols="12">
        <v-card color="surface" class="mb-4">
          <v-card-item>
            <div class="d-flex align-center justify-space-between flex-wrap ga-2">
              <div class="flex-grow-1">
                <v-select
                  label="Select Station"
                  :items="stations.length > 0 ? stations : stationList"
                  v-model="selectedStation"
                  density="compact"
                  variant="outlined"
                  hide-details
                  clearable
                  placeholder="Select a station"
                ></v-select>
              </div>
              <div class="text-caption text-onSurfaceVariant">
                <span v-if="selectedStation">
                  Viewing: {{ selectedStation }}
                </span>
                <span v-else>
                  No station selected
                </span>
              </div>
            </div>
          </v-card-item>
          <v-card-text v-if="stationList.length > 0 || stations.length > 0">
            <div class="text-caption text-onSurfaceVariant mb-1">Connected Stations:</div>
            <div class="d-flex flex-wrap ga-2">
              <v-chip
                v-for="station in (stations.length > 0 ? stations : stationList)"
                :key="station"
                size="small"
                :variant="selectedStation === station ? 'tonal' : 'outlined'"
                :color="selectedStation === station ? 'primary' : 'default'"
                @click="selectedStation = station === selectedStation ? null : station"
                style="cursor: pointer;"
              >
                {{ station }}
              </v-chip>
            </div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<script setup>
import { ref, reactive, watch, onMounted, onBeforeUnmount, computed } from "vue";
import { useMqttStore } from "@/store/mqttStore";
import { useAppStore } from "@/store/appStore";
import { storeToRefs } from "pinia";
import Highcharts from "highcharts";
import more from "highcharts/highcharts-more";
import Exporting from "highcharts/modules/exporting";
import { celsiusToFahrenheit, hPaToPSI } from "@/utils/units";

Exporting(Highcharts);
more(Highcharts);

// STORES
const Mqtt = useMqttStore();
const AppStore = useAppStore();
const { payloads, isConnected } = storeToRefs(Mqtt);
const { useMetric, selectedStation, stations } = storeToRefs(AppStore);

// REFS
const lastUpdate = ref("Waiting for data...");
const connectionStatus = ref("disconnected");
const maxPoints = ref(50);
const chartsInitialized = ref(false);

// CHART REFS
const sparklineCharts = ref({});

// SENSOR CONFIG
const sensors = computed(() => [
  { 
    id: 'temperature', 
    label: 'Temperature', 
    key: 'temperature',
    icon: 'mdi:thermometer',
    unit: useMetric.value ? '°C' : '°F',
    color: 'rgba(255, 152, 0, 0.15)',
    chartColor: '#FF9800'
  },
  { 
    id: 'humidity', 
    label: 'Humidity', 
    key: 'humidity',
    icon: 'mdi:water-percent',
    unit: '%',
    color: 'rgba(0, 188, 212, 0.15)',
    chartColor: '#00BCD4'
  },
  { 
    id: 'pressure', 
    label: 'Pressure', 
    key: 'pressure',
    icon: 'mdi:gauge',
    unit: useMetric.value ? 'hPa' : 'psi',
    color: 'rgba(76, 175, 80, 0.15)',
    chartColor: '#4CAF50'
  },
  { 
    id: 'altitude', 
    label: 'Altitude', 
    key: 'altitude',
    icon: 'mdi:elevation-rise',
    unit: useMetric.value ? 'm' : 'ft',
    color: 'rgba(255, 235, 59, 0.15)',
    chartColor: '#FFEB3B'
  },
  { 
    id: 'soil_moisture', 
    label: 'Soil Moisture', 
    key: 'soil_moisture',
    icon: 'mdi:grass',
    unit: '%',
    color: 'rgba(205, 220, 57, 0.15)',
    chartColor: '#CDDC39'
  },
  { 
    id: 'heat_index', 
    label: 'Heat Index', 
    key: 'heat_index',
    icon: 'mdi:thermometer-alert',
    unit: useMetric.value ? '°C' : '°F',
    color: 'rgba(255, 87, 34, 0.15)',
    chartColor: '#FF5722'
  },
  { 
    id: 'rssi', 
    label: 'Signal', 
    key: 'rssi',
    icon: 'mdi:wifi',
    unit: 'dBm',
    color: 'rgba(156, 39, 176, 0.15)',
    chartColor: '#9C27B0'
  }
]);

// FORMATTER
const formatValue = (sensor) => {
  const value = getCurrentValue(sensor.key);
  switch (sensor.id) {
    case 'temperature':
    case 'heat_index':
      return useMetric.value 
        ? (value !== null && value !== undefined ? value.toFixed(1) : '--')
        : (value !== null && value !== undefined ? celsiusToFahrenheit(value).toFixed(1) : '--');
    case 'humidity':
    case 'soil_moisture':
      return value !== null && value !== undefined ? value.toFixed(1) : '--';
    case 'pressure':
      return useMetric.value 
        ? (value !== null && value !== undefined ? value.toFixed(1) : '--')
        : (value !== null && value !== undefined ? hPaToPSI(value).toFixed(2) : '--');
    case 'altitude':
      return useMetric.value 
        ? (value !== null && value !== undefined ? value.toFixed(1) : '--')
        : (value !== null && value !== undefined ? (value * 3.28084).toFixed(1) : '--');
    case 'rssi':
      return value !== null && value !== undefined ? value : '--';
    default:
      return '--';
  }
};

const getCurrentValue = (key) => {
  // Get payload for selected station
  const currentPayload = selectedStation.value 
    ? payloads.value[selectedStation.value] 
    : null;
  
  if (currentPayload && typeof currentPayload === 'object') {
    return currentPayload[key];
  }
  return null;
};

// Computed: list of stations from payloads
const stationList = computed(() => Object.keys(payloads.value));

// Computed: active payload for selected station
const activePayload = computed(() => {
  if (!selectedStation.value) return null;
  return payloads.value[selectedStation.value] || null;
});

// CHART OPTIONS
const createSparklineOptions = (color) => ({
  chart: {
    type: 'line',
    height: 60,
    backgroundColor: 'transparent',
    margin: [0, 0, 0, 0],
    style: { overflow: 'visible' },
    skipClone: true
  },
  title: { text: null },
  credits: { enabled: false },
  xAxis: {
    type: 'datetime',
    visible: false,
    minPadding: 0,
    maxPadding: 0
  },
  yAxis: {
    visible: false,
    minPadding: 0,
    maxPadding: 0
  },
  legend: { enabled: false },
  tooltip: { enabled: false },
  plotOptions: {
    line: {
      marker: { enabled: false },
      lineWidth: 2,
      color: color,
      states: { hover: { lineWidth: 2 } }
    },
    series: {
      animation: { duration: 300 },
      enableMouseTracking: false
    }
  },
  series: [{
    data: [],
    turboThreshold: 0
  }]
});

// INITIALIZE CHARTS
const initCharts = () => {
  console.log('Dashboard: Initializing charts...');
  
  // Create sparkline charts for each sensor
  sensors.value.forEach(sensor => {
    const container = document.getElementById(`chart-${sensor.id}`);
    if (container) {
      sparklineCharts.value[sensor.id] = Highcharts.chart(
        container,
        createSparklineOptions(sensor.chartColor)
      );
    }
  });
  
  chartsInitialized.value = true;
  console.log('Dashboard: Charts initialized');
};

// UPDATE CHARTS
const updateCharts = () => {
  if (!chartsInitialized.value) {
    console.log('Dashboard: Charts not initialized yet');
    return;
  }
  
  const currentPayload = activePayload.value;
  
  if (!currentPayload || typeof currentPayload !== 'object') {
    console.log('Dashboard: No valid payload for selected station');
    return;
  }
  
  const timestamp = currentPayload.timestamp ? currentPayload.timestamp * 1000 : Date.now();
  console.log('Dashboard: Updating charts with payload', currentPayload);
  
  // Update sparklines
  sensors.value.forEach(sensor => {
    const chart = sparklineCharts.value[sensor.id];
    const rawValue = currentPayload[sensor.key];
    
    if (chart && rawValue !== null && rawValue !== undefined && !isNaN(rawValue)) {
      let value = rawValue;
      if ((sensor.id === 'temperature' || sensor.id === 'heat_index') && !useMetric.value) {
        value = celsiusToFahrenheit(value);
      }
      if (sensor.id === 'altitude' && !useMetric.value) {
        value = value * 3.28084;
      }
      
      try {
        chart.series[0].addPoint([timestamp, value], true, chart.series[0].data.length >= maxPoints.value);
      } catch (e) {
        console.log('Dashboard: Sparkline update error', e);
      }
    }
  });

  // Update timestamp
  const date = new Date(timestamp);
  lastUpdate.value = `Last update: ${date.toLocaleTimeString()}`;
};

// DESTROY CHARTS
const destroyCharts = () => {
  Object.values(sparklineCharts.value).forEach(chart => {
    if (chart) {
      try { chart.destroy(); } catch (e) { console.log('Sparkline destroy error:', e); }
    }
  });
  sparklineCharts.value = {};
  
  chartsInitialized.value = false;
};

// WATCH CONNECTION STATUS
watch(isConnected, (connected) => {
  connectionStatus.value = connected ? 'connected' : 'disconnected';
  console.log('Dashboard: Connection status changed to', connectionStatus.value);
}, { immediate: true });

// WATCH PAYLOAD - Only update when payload for selected station changes
watch(activePayload, (newPayload) => {
  if (newPayload && typeof newPayload === 'object' && Object.keys(newPayload).length > 0) {
    console.log('Dashboard: Received payload for selected station', newPayload);
    updateCharts();
  }
}, { deep: true });

// LIFECYCLE
onMounted(async () => {
  console.log('Dashboard: Component mounted');
  
  // Fetch available stations
  await AppStore.getStations();
  
  // Initialize charts
  setTimeout(() => {
    initCharts();
  }, 100);

  // Check connection status and subscribe
  setTimeout(() => {
    console.log('Dashboard: isConnected =', Mqtt.isConnected, 'mqtt =', !!Mqtt.mqtt);
    
    if (Mqtt.isConnected && Mqtt.mqtt) {
      console.log('Dashboard: Already connected, subscribing to topic');
      Mqtt.subscribe("620171573");
      connectionStatus.value = 'connected';
    } else {
      console.log('Dashboard: Not connected, connecting...');
      connectionStatus.value = 'connecting';
      Mqtt.connect();
    }
  }, 500);
});

onBeforeUnmount(() => {
  console.log('Dashboard: Component unmounting');
  // Only unsubscribe, don't disconnect (connection is global)
  Mqtt.unsubscribeAll();
  destroyCharts();
});
</script>

<style scoped>
.v-card {
  transition: transform 0.2s ease-in-out;
}
.v-card:hover {
  transform: translateY(-2px);
}
</style>