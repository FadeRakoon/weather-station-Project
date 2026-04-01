import { defineStore } from 'pinia'
import { ref } from 'vue'

export const useAppStore = defineStore("app", () => {
    
    /*  
    The composition API way of defining a Pinia store
    ref() s become state properties
    computed() s become getters
    function() s become actions  
    */

    // STATES
    const useMetric = ref(true);
    const selectedStation = ref(null);
    const stations = ref([]);

    // ACTIONS
    const fetchWithTimeout = async (URL, timeoutMs = 60000) => {
        const controller = new AbortController();
        const signal = controller.signal;
        const timeoutId = setTimeout(() => controller.abort(), timeoutMs);
        try {
            const response = await fetch(URL, { method: "GET", signal });
            clearTimeout(timeoutId);
            if (response.ok) {
                return await response.json();
            } else {
                const text = await response.text();
                console.log(`Fetch error: ${text}`);
                return null;
            }
        } catch (err) {
            clearTimeout(timeoutId);
            console.error(`Fetch error: ${err.message}`);
            return null;
        }
    };

    const getStations = async () => {
        const URL = '/api/stations';
        console.log('API: Fetching stations list');
        const data = await fetchWithTimeout(URL);
        if (data && data.status === "success") {
            stations.value = data.data || [];
            if (!selectedStation.value && data.data && data.data.length > 0) {
                selectedStation.value = data.data[0];
            }
            console.log(`API: getStations returned ${data.data?.length || 0} stations`);
            return data.data;
        }
        console.log("API: getStations returned no data");
        return [];
    };

    const getAllInRange = async (start, end) => {
        let URL = `/api/climo/get/${start}/${end}`;
        if (selectedStation.value) {
            URL += `?station_id=${selectedStation.value}`;
        }
        console.log(`API: Fetching all data from ${start} to ${end}${selectedStation.value ? ` for station ${selectedStation.value}` : ''}`);
        const data = await fetchWithTimeout(URL);
        if (data && data.status === "found") {
            console.log(`API: getAllInRange returned ${data.data?.length || 0} records`);
            return data.data;
        }
        console.log("API: getAllInRange returned no data");
        return [];
    };

    const getTemperatureMMAR = async (start, end) => {
        let URL = `/api/mmar/temperature/${start}/${end}`;
        if (selectedStation.value) {
            URL += `?station_id=${selectedStation.value}`;
        }
        console.log(`API: Fetching temperature MMAR from ${start} to ${end}`);
        const data = await fetchWithTimeout(URL);
        if (data && data.status === "found") {
            console.log(`API: getTemperatureMMAR returned data`);
            return data.data;
        }
        console.log("API: getTemperatureMMAR returned no data");
        return [];
    };

    const getHumidityMMAR = async (start, end) => {
        let URL = `/api/mmar/humidity/${start}/${end}`;
        if (selectedStation.value) {
            URL += `?station_id=${selectedStation.value}`;
        }
        console.log(`API: Fetching humidity MMAR from ${start} to ${end}`);
        const data = await fetchWithTimeout(URL);
        if (data && data.status === "found") {
            console.log(`API: getHumidityMMAR returned data`);
            return data.data;
        }
        console.log("API: getHumidityMMAR returned no data");
        return [];
    };

    const getSoilMMAR = async (start, end) => {
        let URL = `/api/mmar/soil/${start}/${end}`;
        if (selectedStation.value) {
            URL += `?station_id=${selectedStation.value}`;
        }
        console.log(`API: Fetching soil moisture MMAR from ${start} to ${end}`);
        const data = await fetchWithTimeout(URL);
        if (data && data.status === "found") {
            console.log(`API: getSoilMMAR returned data`);
            return data.data;
        }
        console.log("API: getSoilMMAR returned no data");
        return [];
    };

    const getFreqDistro = async (variable, start, end) => {
        let URL = `/api/frequency/${variable}/${start}/${end}`;
        if (selectedStation.value) {
            URL += `?station_id=${selectedStation.value}`;
        }
        console.log(`API: Fetching ${variable} frequency distribution from ${start} to ${end}`);
        const data = await fetchWithTimeout(URL);
        if (data && data.status === "found") {
            console.log(`API: getFreqDistro returned data for ${variable}`);
            return data.data;
        }
        console.log(`API: getFreqDistro returned no data for ${variable}`);
        return [];
    };

    // Calculate MMAR client side for variables without backend support
    const calculateMMAR = (dataArray, field) => {
        if (!dataArray || dataArray.length === 0) return null;
        const values = dataArray
            .map(item => item[field])
            .filter(v => v !== null && v !== undefined && !isNaN(v));
        if (values.length === 0) return null;
        const min = Math.min(...values);
        const max = Math.max(...values);
        const avg = values.reduce((a, b) => a + b, 0) / values.length;
        const range = max - min;
        return [{ min, max, avg, range }];
    };

    return {
        // STATES
        useMetric,
        selectedStation,
        stations,
        // ACTIONS
        getStations,
        getAllInRange,
        getTemperatureMMAR,
        getHumidityMMAR,
        getSoilMMAR,
        getFreqDistro,
        calculateMMAR,
    }; 
}, { persist: true });