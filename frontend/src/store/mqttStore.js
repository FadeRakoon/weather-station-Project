import { defineStore } from 'pinia'
import { ref } from 'vue'

export const useMqttStore = defineStore('mqtt', () => {

    /*MQTT Paho Documentation:
        1. https://eclipse.dev/paho/index.php?page=clients/js/index.php 
        2. https://eclipse.dev/paho/files/jsdoc/Paho.MQTT.Client.html
    */ 

    // STATES 
    const mqtt = ref(null);
    const host = ref("www.yanacreations.com");
    const port = ref(9002);
    const isConnected = ref(false);
    const connectionError = ref(null);
    
    // Per-station payload storage
    const payloads = ref({});
    const latestStationId = ref(null);
    
    // Legacy single payload for backward compatibility
    const payload = ref({
        station_id: null,
        timestamp: null,
        temperature: null,
        humidity: null,
        pressure: null,
        altitude: null,
        soil_moisture: null,
        heat_index: null,
        rssi: null
    });
    
    const payloadTopic = ref("");
    const subTopics = ref({});

    // ACTIONS
    
    const onSuccess = () => { 
        console.log(`MQTT: Connection established to ${host.value}:${port.value}`);
        isConnected.value = true;
        connectionError.value = null;
    }

    const onConnected = (reconnect, URI) => {
        console.log(`MQTT: Connected to: ${URI}, Reconnect: ${reconnect}`);      
        isConnected.value = true;
        if(reconnect){
            const topics = Object.keys(subTopics.value);
            if(topics.length > 0){
                topics.forEach((topic) => {
                    subscribe(topic);
                });
            }
        }
    }
 
    const onConnectionLost = (response) => { 
        isConnected.value = false;
        if (response.errorCode !== 0) {
            console.log(`MQTT: Connection lost - ${response.errorMessage}`);
            connectionError.value = response.errorMessage;
        } else {
            console.log(`MQTT: Connection closed normally`);
        }
    }
    
    const onFailure = (response) => {
        isConnected.value = false;
        const h = response.invocationContext.host;   
        console.log(`MQTT: Connection to ${h} failed. Error: ${response.errorMessage}`);                  
        connectionError.value = response.errorMessage;
    };
    
    const onMessageArrived = (response) => {
        try {
            const parsed = JSON.parse(response.payloadString);
            const stationId = parsed.station_id || 'unknown';
            
            // Store payload per station
            payloads.value[stationId] = parsed;
            latestStationId.value = stationId;
            
            // Also update legacy payload for backward compatibility
            payload.value = parsed;
            payloadTopic.value = response.destinationName;
            
            console.log(`MQTT: Station ${stationId} | Topic: ${response.destinationName} | Payload:`, parsed);
        } catch (error) {
            console.log(`MQTT: onMessageArrived Error: ${error}`);
        }
    }
 
    const makeid = (length) => {
        var result = '';
        var characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
        var charactersLength = characters.length;
        for ( var i = 0; i < length; i++ ) {
            result += characters.charAt(Math.floor(Math.random() * charactersLength));
        }
        return "IOT_F_" + result;
    };

    // SUBSCRIBE UTIL FUNCTIONS
    const sub_onSuccess = (response) => {   
        const topic = response.invocationContext.topic;  
        console.log(`MQTT: Subscribed to - ${topic}`);  
        subTopics.value[topic] = "subscribed"; 
    }

    const sub_onFailure = (response) => {       
        const topic = response.invocationContext.topic;  
        console.log(`MQTT: Failed to subscribe to - ${topic}. Error: ${response.errorMessage}`);  
    }

    const subscribe = (topic) => {
        if (!isConnected.value || !mqtt.value) {
            console.log(`MQTT: Cannot subscribe - not connected (isConnected: ${isConnected.value}, mqtt: ${!!mqtt.value})`);
            return;
        }
        try {
            var subscribeOptions = { onSuccess: sub_onSuccess, onFailure: sub_onFailure, invocationContext: {"topic": topic} };
            mqtt.value.subscribe(topic, subscribeOptions);   
        } catch (error) {
            console.log(`MQTT: Unable to subscribe - ${error}`);
        }
    }

    // UNSUBSCRIBE UTIL FUNCTIONS
    const unSub_onSuccess = (response) => {    
        const topic = response.invocationContext.topic;  
        console.log(`MQTT: Unsubscribed from - ${topic}`);          
        delete subTopics.value[topic];
    }

    const unSub_onFailure = (response) => {   
        const topic = response.invocationContext.topic;  
        console.log(`MQTT: Failed to unsubscribe from - ${topic}. Error: ${response.errorMessage}`);
    }

    const unsubscribe = (topic) => {     
        if (!isConnected.value || !mqtt.value) {
            console.log(`MQTT: Cannot unsubscribe - not connected`);
            return;
        }
        var unsubscribeOptions = { onSuccess: unSub_onSuccess, onFailure: unSub_onFailure, invocationContext: {"topic": topic} };
        mqtt.value.unsubscribe(topic, unsubscribeOptions);         
    }
    
    const unsubscribeAll = () => {   
        const topics = Object.keys(subTopics.value);
        if(topics.length > 0) {
            topics.forEach((topic) => {
                var unsubscribeOptions = { onSuccess: unSub_onSuccess, onFailure: unSub_onFailure, invocationContext: {"topic": topic} };
                mqtt.value.unsubscribe(topic, unsubscribeOptions);
            });
        }  
    }

    // PUBLISH UTIL FUNCTION
    const publish = (topic, payload) => { 
        if (!isConnected.value || !mqtt.value) {
            console.log(`MQTT: Cannot publish - not connected`);
            return;
        }
        const message = new Paho.MQTT.Message(payload);
        message.destinationName = topic;
        mqtt.value.publish(message);                     
    }

    // DISCONNECT UTIL FUNCTION
    const disconnect = () => {  
        if (mqtt.value && isConnected.value) {
            mqtt.value.disconnect();
            isConnected.value = false;
            console.log(`MQTT: Disconnected`);
        }
    }

    const connect = () => {
        if (isConnected.value && mqtt.value) {
            console.log(`MQTT: Already connected`);
            return;
        }

        var IDstring = makeid(12);
        console.log(`MQTT: Connecting to Server: ${host.value} Port: ${port.value}`);
        mqtt.value = new Paho.MQTT.Client(host.value, port.value, "/mqtt", IDstring);   
    
        var options = { 
            timeout: 3, 
            onSuccess: onSuccess, 
            onFailure: onFailure, 
            invocationContext: {"host": host.value, "port": port.value }, 
            useSSL: false, 
            reconnect: true, 
            uris: [`ws://${host.value}:${port.value}/mqtt`]
        }; 
        
        mqtt.value.onConnectionLost = onConnectionLost;
        mqtt.value.onMessageArrived = onMessageArrived;
        mqtt.value.onConnected = onConnected;
        mqtt.value.connect(options);    
    };
 
    // GET PAYLOAD FOR SPECIFIC STATION
    const getPayload = (stationId) => {
        return payloads.value[stationId] || null;
    };
    
    // GET ALL STATION IDS
    const getStationIds = () => {
        return Object.keys(payloads.value);
    };

    return {  
        mqtt,
        payload,
        payloads,
        latestStationId,
        payloadTopic,
        isConnected,
        connectionError,
        getPayload,
        getStationIds,
        subscribe,
        unsubscribe,
        unsubscribeAll,
        publish,
        connect,
        disconnect,
    };
}, { 
    persist: {
        paths: ['payload', 'payloads', 'latestStationId', 'payloadTopic']
    }
});