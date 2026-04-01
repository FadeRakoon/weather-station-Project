<template>
  <VToolbar flat density="compact" color="surface" height="60">
    <VContainer fluid>
      <VRow align="center" justify="center">
        <VCol cols="2" align="left">
          <span class="text-h6 text-primary font-weight-bold">Weather Station</span>
        </VCol>

        <VCol cols="8" align="center"> 
          <RouterLink class="route" :to="{ name: 'Dashboard' }">
            <VBtn 
              class="text-subtitle-2" 
              density="compact" 
              color="primary" 
              :variant="(route.name === 'Dashboard') ? 'tonal' : 'text'"
            >
              Dashboard
            </VBtn>
          </RouterLink>

          <RouterLink class="route" to="/analysis">
            <VBtn 
              class="text-subtitle-2" 
              density="compact" 
              color="primary" 
              :variant="(route.name === 'Analysis') ? 'tonal' : 'text'"
            >
              Analysis
            </VBtn>
          </RouterLink>  
        </VCol>

<VCol cols="2" align="right">
          <VBtn 
            size="x-small" 
            variant="flat"
            color="primary"
            @click="toggleUnits"
            class="mr-2"
          >                  
            <img 
              :src="useMetric ? fahrenheitIcon : celsiusIcon" 
              :alt="useMetric ? 'Switch to Fahrenheit' : 'Switch to Celsius'"
              style="width: 20px; height: 20px;"
            />
            <VTooltip activator="parent">
              {{ useMetric ? 'Switch to Imperial (°F, ft)' : 'Switch to Metric (°C, m)' }}
            </VTooltip>
          </VBtn>
          
          <VBtn 
            size="x-small" 
            variant="flat"
            color="primary"
            icon
            @click="toggleTheme"
          >                  
            <VIcon :icon="darkmode ? 'mdi-weather-night' : 'mdi-white-balance-sunny'" color="black" />
            <VTooltip activator="parent">
              {{ darkmode ? 'Switch to Light Mode' : 'Switch to Dark Mode' }}
            </VTooltip>
          </VBtn>
        </VCol>
      </VRow>
    </VContainer>
  </VToolbar>
</template>

<script setup>
import { useRoute } from "vue-router";
import { useTheme } from 'vuetify';  
import { ref, watch, onBeforeMount } from 'vue';
import { useAppStore } from '@/store/appStore';
import celsiusIcon from '@/assets/celsius.png';
import fahrenheitIcon from '@/assets/fahrenheit.png';

// VARIABLES 
const theme = useTheme();
const darkmode = ref(false); 
const route = useRoute();  
const appStore = useAppStore();
const useMetric = ref(true);

// WATCHERS
watch(darkmode, (mode) => {
  theme.global.name.value = mode ? 'darkMode' : 'lightMode';
  localStorage.setItem("theme", mode ? 'darkMode' : 'lightMode');  
});

// FUNCTIONS
const toggleTheme = () => {
  darkmode.value = !darkmode.value;
};

const toggleUnits = () => {
  useMetric.value = !useMetric.value;
  appStore.useMetric = useMetric.value;
  localStorage.setItem("useMetric", useMetric.value);
  console.log(`Units: ${useMetric.value ? 'Metric (°C, m)' : 'Imperial (°F, ft)'}`);
};

onBeforeMount(() => {
  // RESTORE THEME FROM LOCALSTORAGE
  if (localStorage.getItem("theme") != null) {
    theme.global.name.value = localStorage.getItem("theme");
    darkmode.value = theme.global.current.value.dark;
  } else {
    localStorage.setItem("theme", theme.global.current.value.dark ? 'darkMode' : 'lightMode');
    darkmode.value = theme.global.current.value.dark;    
  }
  
  // RESTORE UNIT PREFERENCE FROM LOCALSTORAGE
  const savedMetric = localStorage.getItem("useMetric");
  if (savedMetric !== null) {
    useMetric.value = savedMetric === 'true';
    appStore.useMetric = useMetric.value;
  } else {
    appStore.useMetric = true;
  }
});  
</script>

<style>
.route {
  text-decoration: none;
  margin-left: 8px;
  margin-right: 8px; 
}
</style>