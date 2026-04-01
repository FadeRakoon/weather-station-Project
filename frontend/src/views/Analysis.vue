<template>
  <v-container fluid class="pa-4">
    <v-row style="max-width: 1400px; margin: 0 auto;">
      <v-col cols="12">
        <v-card color="surface" class="mb-4">
          <v-card-item>
            <span class="text-h5 text-primary">Historical Analysis</span>
            <div class="text-caption text-onSurfaceVariant mt-1">
              Analyze sensor data over time
            </div>
          </v-card-item>
          <v-card-text>
            <v-row>
              <v-col cols="12" sm="6" md="4">
                <v-select
                  label="Station"
                  :items="stations"
                  v-model="selectedStation"
                  density="compact"
                  variant="outlined"
                  hide-details
                  clearable
                ></v-select>
              </v-col>
              <v-col cols="12" sm="6" md="4">
                <v-text-field
                  label="Start Date"
                  type="date"
                  density="compact"
                  variant="outlined"
                  v-model="start"
                  hide-details
                ></v-text-field>
              </v-col>
              <v-col cols="12" sm="6" md="4">
                <v-text-field
                  label="End Date"
                  type="date"
                  density="compact"
                  variant="outlined"
                  v-model="end"
                  hide-details
                ></v-text-field>
              </v-col>
            </v-row>
            <v-row class="mt-2">
              <v-col cols="12">
                <div class="text-caption mb-2">Select Sensors</div>
                <v-chip-group v-model="selectedSensors" multiple column mandatory="false">
                  <v-chip
                    v-for="sensor in availableSensors"
                    :key="sensor.key"
                    :value="sensor.key"
                    :color="sensor.chartColor"
                    variant="outlined"
                    filter
                  >
                    <v-icon start :icon="sensor.icon" size="18"></v-icon>
                    {{ sensor.label }}
                  </v-chip>
                </v-chip-group>
              </v-col>
            </v-row>
            <v-row class="mt-2">
              <v-col cols="12">
                <v-btn
                  color="primary"
                  variant="tonal"
                  @click="analyze"
                  :loading="loading"
                  :disabled="!start || !end || selectedSensors.length === 0"
                >
                  <v-icon start icon="mdi:chart-line"></v-icon>
                  Analyze
                </v-btn>
                <v-btn
                  class="ml-2"
                  variant="text"
                  @click="resetFilters"
                >
                  Reset
                </v-btn>
              </v-col>
            </v-row>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <v-row style="max-width: 1400px; margin: 0 auto;" v-if="hasData">
      <v-col cols="12" sm="6" md="3" v-for="stat in mmarCards" :key="stat.key">
        <v-card :color="stat.color" variant="tonal">
          <v-card-item>
            <div class="text-overline text-onSurfaceVariant">{{ stat.label }}</div>
            <div class="text-h4 text-onSurface font-weight-bold">{{ stat.avg }}</div>
            <div class="text-caption text-onSurfaceVariant">{{ stat.unit }}</div>
          </v-card-item>
          <v-card-text class="pt-0">
            <div class="d-flex justify-space-between">
              <div class="text-caption">
                <span class="font-weight-bold text-success">Min:</span> {{ stat.min }}
              </div>
              <div class="text-caption">
                <span class="font-weight-bold text-error">Max:</span> {{ stat.max }}
              </div>
              <div class="text-caption">
                <span class="font-weight-bold text-info">Range:</span> {{ stat.range }}
              </div>
            </div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <v-row style="max-width: 1400px; margin: 0 auto;" v-if="hasData">
      <v-col cols="12" v-if="showTemperature">
        <v-card color="surface">
          <v-card-title class="text-primary">Temperature Over Time</v-card-title>
          <v-card-text>
            <div id="tempChart" style="height: 300px;"></div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <v-row style="max-width: 1400px; margin: 0 auto;" v-if="hasData && showHumidity">
      <v-col cols="12">
<v-card color="surface">
            <v-card-title class="text-primary">Humidity Over Time</v-card-title>
          <v-card-text>
            <div id="humidityChart" style="height: 300px;"></div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <v-row style="max-width: 1400px; margin: 0 auto;" v-if="hasData">
      <v-col cols="12" md="6" v-if="showPressure">
<v-card color="surface">
            <v-card-title class="text-primary">Pressure Over Time</v-card-title>
          <v-card-text>
            <div id="pressureChart" style="height: 250px;"></div>
          </v-card-text>
        </v-card>
      </v-col>
      <v-col cols="12" md="6" v-if="showSoil">
<v-card color="surface">
            <v-card-title class="text-primary">Soil Moisture Over Time</v-card-title>
          <v-card-text>
            <div id="soilChart" style="height: 250px;"></div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <v-row style="max-width: 1400px; margin: 0 auto;" v-if="hasData">
      <v-col cols="12">
        <v-card color="surface">
          <v-card-title class="text-primary">Temperature vs Heat Index</v-card-title>
          <v-card-text>
            <div id="combinedTempHeatChart" style="height: 300px;"></div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <v-row style="max-width: 1400px; margin: 0 auto;" v-if="hasData">
      <v-col cols="12">
        <v-card color="surface">
          <v-card-title class="text-primary">Humidity vs Heat Index</v-card-title>
          <v-card-text>
            <div id="combinedHumidityHeatChart" style="height: 300px;"></div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <v-row style="max-width: 1400px; margin: 0 auto;" v-if="!hasData && analyzed">
      <v-col cols="12">
        <v-card color="surface">
          <v-card-text class="text-center py-8">
            <v-icon icon="mdi:database-off" size="48" color="onSurfaceVariant"></v-icon>
            <div class="text-h6 mt-4">No data found</div>
            <div class="text-caption text-onSurfaceVariant">
              Try selecting a different date range
            </div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<script setup>
import { ref, computed, onMounted, onBeforeUnmount, watch } from "vue";
import { useMqttStore } from "@/store/mqttStore";
import { useAppStore } from "@/store/appStore";
import { useTheme } from 'vuetify';
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
const theme = useTheme();
const { useMetric, selectedStation, stations } = storeToRefs(AppStore);

// REFS
const start = ref("");
const end = ref("");
const loading = ref(false);
const analyzed = ref(false);
const rawData = ref([]);

const selectedSensors = ref(['temperature', 'humidity', 'soil_moisture']);

const availableSensors = [
  { key: 'temperature', label: 'Temperature', icon: 'mdi:thermometer', chartColor: '#FF9800' },
  { key: 'humidity', label: 'Humidity', icon: 'mdi:water-percent', chartColor: '#00BCD4' },
  { key: 'pressure', label: 'Pressure', icon: 'mdi:gauge', chartColor: '#4CAF50' },
  { key: 'soil_moisture', label: 'Soil Moisture', icon: 'mdi:grass', chartColor: '#CDDC39' },
];

// COMPUTED
const showTemperature = computed(() => selectedSensors.value.includes('temperature'));
const showHumidity = computed(() => selectedSensors.value.includes('humidity'));
const showPressure = computed(() => selectedSensors.value.includes('pressure'));
const showSoil = computed(() => selectedSensors.value.includes('soil_moisture'));
const hasData = computed(() => rawData.value && rawData.value.length > 0);

const mmarCards = computed(() => {
  const cards = [];
  
  if (selectedSensors.value.includes('temperature') && temperatureMMAR.value) {
    const mmar = temperatureMMAR.value[0];
    cards.push({
      key: 'temperature',
      label: 'Temperature',
      avg: useMetric.value ? `${mmar.avg.toFixed(1)}°C` : `${celsiusToFahrenheit(mmar.avg).toFixed(1)}°F`,
      min: useMetric.value ? `${mmar.min.toFixed(1)}°C` : `${celsiusToFahrenheit(mmar.min).toFixed(1)}°F`,
      max: useMetric.value ? `${mmar.max.toFixed(1)}°C` : `${celsiusToFahrenheit(mmar.max).toFixed(1)}°F`,
      range: useMetric.value ? `${mmar.range.toFixed(1)}°C` : `${celsiusToFahrenheit(mmar.range).toFixed(1)}°F`,
      unit: useMetric.value ? '°C' : '°F',
      color: 'rgba(255, 152, 0, 0.15)'
    });
  }
  
  if (selectedSensors.value.includes('humidity') && humidityMMAR.value) {
    const mmar = humidityMMAR.value[0];
    cards.push({
      key: 'humidity',
      label: 'Humidity',
      avg: `${mmar.avg.toFixed(1)}%`,
      min: `${mmar.min.toFixed(1)}%`,
      max: `${mmar.max.toFixed(1)}%`,
      range: `${mmar.range.toFixed(1)}%`,
      unit: '%',
      color: 'rgba(0, 188, 212, 0.15)'
    });
  }
  
  if (selectedSensors.value.includes('soil_moisture') && soilMMAR.value) {
    const mmar = soilMMAR.value[0];
    cards.push({
      key: 'soil_moisture',
      label: 'Soil Moisture',
      avg: `${mmar.avg.toFixed(1)}%`,
      min: `${mmar.min.toFixed(1)}%`,
      max: `${mmar.max.toFixed(1)}%`,
      range: `${mmar.range.toFixed(1)}%`,
      unit: '%',
      color: 'rgba(205, 220, 57, 0.15)'
    });
  }
  
  if (selectedSensors.value.includes('pressure') && pressureMMAR.value) {
    const mmar = pressureMMAR.value[0];
    cards.push({
      key: 'pressure',
      label: 'Pressure',
      avg: useMetric.value ? `${mmar.avg.toFixed(1)} hPa` : `${hPaToPSI(mmar.avg).toFixed(2)} psi`,
      min: useMetric.value ? `${mmar.min.toFixed(1)} hPa` : `${hPaToPSI(mmar.min).toFixed(2)} psi`,
      max: useMetric.value ? `${mmar.max.toFixed(1)} hPa` : `${hPaToPSI(mmar.max).toFixed(2)} psi`,
      range: useMetric.value ? `${mmar.range.toFixed(1)} hPa` : `${hPaToPSI(mmar.range).toFixed(2)} psi`,
      unit: useMetric.value ? 'hPa' : 'psi',
      color: 'rgba(76, 175, 80, 0.15)'
    });
  }
  
  return cards;
});

// CHART THEME OPTIONS
const chartThemeOptions = computed(() => {
  const isDark = theme.global.current.value.dark;
  return {
    chart: {
      style: {
        fontFamily: 'inherit'
      }
    },
    yAxis: {
      labels: {
        style: {
          color: isDark ? '#ffffff' : '#1c1c1c'
        }
      },
      title: {
        style: {
          color: isDark ? '#ffffff' : '#1c1c1c'
        }
      },
      gridLineColor: isDark ? '#333333' : '#e0e0e0',
      lineColor: isDark ? '#333333' : '#cccccc'
    },
    xAxis: {
      labels: {
        style: {
          color: isDark ? '#ffffff' : '#1c1c1c'
        }
      },
      lineColor: isDark ? '#333333' : '#cccccc',
      tickColor: isDark ? '#333333' : '#cccccc'
    },
    legend: {
      itemStyle: {
        color: isDark ? '#ffffff' : '#1c1c1c'
      },
      itemHoverStyle: {
        color: isDark ? '#c0fe0c' : '#ff5500'
      }
    },
    tooltip: {
      backgroundColor: isDark ? '#1c1c1c' : '#ffffff',
      borderColor: isDark ? '#333333' : '#cccccc',
      style: {
        color: isDark ? '#ffffff' : '#1c1c1c'
      }
    }
  };
});

const temperatureMMAR = ref(null);
const humidityMMAR = ref(null);
const soilMMAR = ref(null);
const pressureMMAR = ref(null);

// CHART REFS
const tempChart = ref(null);
const humidityChart = ref(null);
const pressureChart = ref(null);
const soilChart = ref(null);
const combinedTempHeatChart = ref(null);
const combinedHumidityHeatChart = ref(null);

// ANALYZE FUNCTION
const analyze = async () => {
  if (!start.value || !end.value || selectedSensors.value.length === 0) return;
  
  loading.value = true;
  analyzed.value = false;
  console.log(`Analysis: Analyzing data from ${start.value} to ${end.value}`);
  
  try {
    const startDate = new Date(start.value).getTime() / 1000;
    const endDate = new Date(end.value).getTime() / 1000;
    
    // Fetch all data
    rawData.value = await AppStore.getAllInRange(startDate, endDate);
    console.log(`Analysis: Fetched ${rawData.value?.length || 0} records`);
    
    if (rawData.value && rawData.value.length > 0) {
      // Fetch MMAR data
      if (selectedSensors.value.includes('temperature')) {
        temperatureMMAR.value = await AppStore.getTemperatureMMAR(startDate, endDate);
        console.log('Analysis: Temperature MMAR', temperatureMMAR.value);
      }
      if (selectedSensors.value.includes('humidity')) {
        humidityMMAR.value = await AppStore.getHumidityMMAR(startDate, endDate);
        console.log('Analysis: Humidity MMAR', humidityMMAR.value);
      }
      if (selectedSensors.value.includes('soil_moisture')) {
        soilMMAR.value = await AppStore.getSoilMMAR(startDate, endDate);
        console.log('Analysis: Soil MMAR', soilMMAR.value);
      }
      if (selectedSensors.value.includes('pressure')) {
        pressureMMAR.value = AppStore.calculateMMAR(rawData.value, 'pressure');
        console.log('Analysis: Pressure MMAR (client-side)', pressureMMAR.value);
      }
      
      // Create charts after DOM update
      setTimeout(() => {
        createCharts();
        analyzed.value = true;
      }, 100);
    } else {
      analyzed.value = true;
    }
  } catch (error) {
    console.error('Analysis: Error analyzing data', error);
  } finally {
    loading.value = false;
  }
};

const createCharts = () => {
  destroyCharts();
  
  const themeOptions = chartThemeOptions.value;
  const tempContainer = document.getElementById('tempChart');
  const humidityContainer = document.getElementById('humidityChart');
  const pressureContainer = document.getElementById('pressureChart');
  const soilContainer = document.getElementById('soilChart');
  
  // Temperature Chart
  if (tempContainer && showTemperature.value) {
    const tempData = [];
    rawData.value.forEach(row => {
      const ts = row.timestamp * 1000;
      if (row.temperature !== null && row.temperature !== undefined) {
        const tempVal = useMetric.value ? row.temperature : celsiusToFahrenheit(row.temperature);
        tempData.push({ x: ts, y: parseFloat(tempVal.toFixed(2)) });
      }
    });
    
    tempChart.value = Highcharts.chart(tempContainer, {
      ...themeOptions,
      chart: { type: 'spline', backgroundColor: 'transparent', zoomType: 'x' },
      title: { text: null },
      yAxis: { ...themeOptions.yAxis, title: { text: useMetric.value ? 'Temperature (°C)' : 'Temperature (°F)', style: themeOptions.yAxis.title.style } },
      xAxis: { ...themeOptions.xAxis, type: 'datetime', labels: { ...themeOptions.xAxis.labels, format: '{value:%m/%d %H:%M}' } },
      tooltip: { ...themeOptions.tooltip, shared: true, crosshairs: true, xDateFormat: '%m/%d %Y %H:%M' },
      legend: { enabled: false },
      plotOptions: { spline: { marker: { enabled: false }, lineWidth: 2 } },
      credits: { enabled: false },
      series: [
        { name: 'Temperature', data: tempData, color: '#FF9800', turboThreshold: 0 }
      ]
    });
  }
  
  // Humidity Chart
  if (humidityContainer && showHumidity.value) {
    const humidityData = rawData.value.map(row => ({
      x: row.timestamp * 1000,
      y: parseFloat(row.humidity?.toFixed(2) || 0)
    }));
    
    humidityChart.value = Highcharts.chart(humidityContainer, {
      ...themeOptions,
      chart: { type: 'spline', backgroundColor: 'transparent', zoomType: 'x' },
      title: { text: null },
      yAxis: { ...themeOptions.yAxis, title: { text: 'Humidity (%)', style: themeOptions.yAxis.title.style }, labels: { ...themeOptions.yAxis.labels, format: '{value}%' } },
      xAxis: { ...themeOptions.xAxis, type: 'datetime', labels: { ...themeOptions.xAxis.labels, format: '{value:%m/%d %H:%M}' } },
      tooltip: { ...themeOptions.tooltip, shared: true, crosshairs: true, xDateFormat: '%m/%d %Y %H:%M' },
      legend: { enabled: false },
      plotOptions: { spline: { marker: { enabled: false }, lineWidth: 2 } },
      credits: { enabled: false },
      series: [{ name: 'Humidity', data: humidityData, color: '#00BCD4', turboThreshold: 0 }]
    });
  }
  
  // Pressure Chart
  if (pressureContainer && showPressure.value) {
    const pressureData = rawData.value
      .filter(row => row.pressure !== null && row.pressure !== undefined)
      .map(row => {
        const pressureVal = useMetric.value ? row.pressure : hPaToPSI(row.pressure);
        return { x: row.timestamp * 1000, y: parseFloat(pressureVal.toFixed(2)) };
      });
    
    pressureChart.value = Highcharts.chart(pressureContainer, {
      ...themeOptions,
      chart: { type: 'spline', backgroundColor: 'transparent', zoomType: 'x' },
      title: { text: null },
      yAxis: { ...themeOptions.yAxis, title: { text: useMetric.value ? 'Pressure (hPa)' : 'Pressure (psi)', style: themeOptions.yAxis.title.style } },
      xAxis: { ...themeOptions.xAxis, type: 'datetime', labels: { ...themeOptions.xAxis.labels, format: '{value:%m/%d %H:%M}' } },
      tooltip: { ...themeOptions.tooltip, shared: true, crosshairs: true, xDateFormat: '%m/%d %Y %H:%M' },
      legend: { enabled: false },
      plotOptions: { spline: { marker: { enabled: false }, lineWidth: 2 } },
      credits: { enabled: false },
      series: [{ name: 'Pressure', data: pressureData, color: '#4CAF50', turboThreshold: 0 }]
    });
  }
  
  // Soil Chart
  if (soilContainer && showSoil.value) {
    const soilData = rawData.value
      .filter(row => row.soil_moisture !== null && row.soil_moisture !== undefined)
      .map(row => ({ x: row.timestamp * 1000, y: parseFloat(row.soil_moisture.toFixed(2)) }));
    
    soilChart.value = Highcharts.chart(soilContainer, {
      ...themeOptions,
      chart: { type: 'spline', backgroundColor: 'transparent', zoomType: 'x' },
      title: { text: null },
      yAxis: { ...themeOptions.yAxis, title: { text: 'Soil Moisture (%)', style: themeOptions.yAxis.title.style }, labels: { ...themeOptions.yAxis.labels, format: '{value}%' } },
      xAxis: { ...themeOptions.xAxis, type: 'datetime', labels: { ...themeOptions.xAxis.labels, format: '{value:%m/%d %H:%M}' } },
      tooltip: { ...themeOptions.tooltip, shared: true, crosshairs: true, xDateFormat: '%m/%d %Y %H:%M' },
      legend: { enabled: false },
      plotOptions: { spline: { marker: { enabled: false }, lineWidth: 2 } },
      credits: { enabled: false },
      series: [{ name: 'Soil Moisture', data: soilData, color: '#CDDC39', turboThreshold: 0 }]
    });
  }
  
  // Combined Temperature vs Heat Index Chart
  const combinedTempHeatContainer = document.getElementById('combinedTempHeatChart');
  if (combinedTempHeatContainer && hasData.value) {
    const tempData = [];
    const heatData = [];
    rawData.value.forEach(row => {
      const ts = row.timestamp * 1000;
      if (row.temperature !== null && row.temperature !== undefined) {
        const tempVal = useMetric.value ? row.temperature : celsiusToFahrenheit(row.temperature);
        tempData.push({ x: ts, y: parseFloat(tempVal.toFixed(2)) });
      }
      if (row.heat_index !== null && row.heat_index !== undefined) {
        const heatVal = useMetric.value ? row.heat_index : celsiusToFahrenheit(row.heat_index);
        heatData.push({ x: ts, y: parseFloat(heatVal.toFixed(2)) });
      }
    });
    
    combinedTempHeatChart.value = Highcharts.chart(combinedTempHeatContainer, {
      ...themeOptions,
      chart: { type: 'spline', backgroundColor: 'transparent', zoomType: 'x' },
      title: { text: null },
      yAxis: { ...themeOptions.yAxis, title: { text: useMetric.value ? 'Temperature (°C)' : 'Temperature (°F)', style: themeOptions.yAxis.title.style } },
      xAxis: { ...themeOptions.xAxis, type: 'datetime', labels: { ...themeOptions.xAxis.labels, format: '{value:%m/%d %H:%M}' } },
      tooltip: { ...themeOptions.tooltip, shared: true, crosshairs: true, xDateFormat: '%m/%d %Y %H:%M' },
      legend: { ...themeOptions.legend, align: 'center', verticalAlign: 'bottom' },
      plotOptions: { spline: { marker: { enabled: false }, lineWidth: 2 } },
      credits: { enabled: false },
      series: [
        { name: 'Temperature', data: tempData, color: '#FF9800', turboThreshold: 0 },
        { name: 'Heat Index', data: heatData, color: '#FF5722', turboThreshold: 0 }
      ]
    });
  }
  
  // Combined Humidity vs Heat Index Chart
  const combinedHumidityHeatContainer = document.getElementById('combinedHumidityHeatChart');
  if (combinedHumidityHeatContainer && hasData.value) {
    const humidityData = rawData.value.map(row => ({
      x: row.timestamp * 1000,
      y: parseFloat(row.humidity?.toFixed(2) || 0)
    }));
    const heatData = [];
    rawData.value.forEach(row => {
      const ts = row.timestamp * 1000;
      if (row.heat_index !== null && row.heat_index !== undefined) {
        const heatVal = useMetric.value ? row.heat_index : celsiusToFahrenheit(row.heat_index);
        heatData.push({ x: ts, y: parseFloat(heatVal.toFixed(2)) });
      }
    });
    
    combinedHumidityHeatChart.value = Highcharts.chart(combinedHumidityHeatContainer, {
      ...themeOptions,
      chart: { type: 'spline', backgroundColor: 'transparent', zoomType: 'x' },
      title: { text: null },
      yAxis: [
        { ...themeOptions.yAxis, title: { text: 'Humidity (%)', style: themeOptions.yAxis.title.style }, labels: { ...themeOptions.yAxis.labels, format: '{value}%' } },
        { ...themeOptions.yAxis, title: { text: useMetric.value ? 'Heat Index (°C)' : 'Heat Index (°F)', style: { ...themeOptions.yAxis.title.style } }, opposite: true }
      ],
      xAxis: { ...themeOptions.xAxis, type: 'datetime', labels: { ...themeOptions.xAxis.labels, format: '{value:%m/%d %H:%M}' } },
      tooltip: { ...themeOptions.tooltip, shared: true, crosshairs: true, xDateFormat: '%m/%d %Y %H:%M' },
      legend: { ...themeOptions.legend, align: 'center', verticalAlign: 'bottom' },
      plotOptions: { spline: { marker: { enabled: false }, lineWidth: 2 } },
      credits: { enabled: false },
      series: [
        { name: 'Humidity', data: humidityData, color: '#00BCD4', turboThreshold: 0 },
        { name: 'Heat Index', data: heatData, color: '#FF5722', yAxis: 1, turboThreshold: 0 }
      ]
    });
  }
};

const destroyCharts = () => {
  if (tempChart.value) { try { tempChart.value.destroy(); } catch (e) {} tempChart.value = null; }
  if (humidityChart.value) { try { humidityChart.value.destroy(); } catch (e) {} humidityChart.value = null; }
  if (pressureChart.value) { try { pressureChart.value.destroy(); } catch (e) {} pressureChart.value = null; }
  if (soilChart.value) { try { soilChart.value.destroy(); } catch (e) {} soilChart.value = null; }
  if (combinedTempHeatChart.value) { try { combinedTempHeatChart.value.destroy(); } catch (e) {} combinedTempHeatChart.value = null; }
  if (combinedHumidityHeatChart.value) { try { combinedHumidityHeatChart.value.destroy(); } catch (e) {} combinedHumidityHeatChart.value = null; }
};

const resetFilters = () => {
  start.value = "";
  end.value = "";
  selectedSensors.value = ['temperature', 'humidity', 'soil_moisture'];
  rawData.value = [];
  analyzed.value = false;
  temperatureMMAR.value = null;
  humidityMMAR.value = null;
  soilMMAR.value = null;
  pressureMMAR.value = null;
  destroyCharts();
};

// WATCH THEME CHANGES
watch(() => theme.global.current.value.dark, () => {
  if (hasData.value) {
    destroyCharts();
    setTimeout(() => createCharts(), 100);
  }
});

// AUTO-UPDATE ON SENSOR/STATION CHANGE
watch([selectedSensors, selectedStation], () => {
  if (start.value && end.value && selectedSensors.value.length > 0) {
    analyze();
  }
}, { deep: true });

// LIFECYCLE
onMounted(async () => {
  console.log('Analysis: Component mounted');
  await AppStore.getStations();
});

onBeforeUnmount(() => {
  console.log('Analysis: Component unmounting');
  destroyCharts();
});
</script>

<style scoped>
.v-card {
  transition: transform 0.2s ease-in-out;
}

/* Futuristic MMAR label colors */
.v-theme--lightMode .text-success {
  color: #c0fe0c !important;
}

.v-theme--lightMode .text-error {
  color: #ff5500 !important;
}

.v-theme--lightMode .text-info {
  color: #5200ff !important;
}

.v-theme--darkMode .text-success {
  color: #c0fe0c !important;
}

.v-theme--darkMode .text-error {
  color: #ff5500 !important;
}

.v-theme--darkMode .text-info {
  color: #5200ff !important;
}
</style>