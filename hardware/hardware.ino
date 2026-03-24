#include <Arduino.h>
#include <DHT.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <string.h>
#include <ctype.h>
#include <rom/rtc.h> 
#include <math.h>
#include <PubSubClient.h>
#include <stdlib.h>
#include <stdio.h>
#include <ArduinoJson.h>

#include <Adafruit_GFX.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_ILI9341.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#include "NTP.h"



#define ARDUINOJSON_USE_DOUBLE      1

#define TFT_DC 17
#define TFT_CS 5
#define TFT_RST 16
#define TFT_CLK 18
#define TFT_MOSI 23
#define TFT_MISO 19

#define I2C_SDA 21
#define I2C_SCL 22
#define DHT_PIN 32
#define DHT_TYPE DHT22

#define SOIL_PIN 33
#define SOIL_DRY  2500  // Raw ADC value in dry air / dry soil
#define SOIL_WET  1100  // Raw ADC value in water / saturated soil

#define SEA_LEVEL_HPA   1013.25 //to maths altitude

#define BMP280_ADDR 0x76
#define SCREEN_W 240
#define SCREEN_H 320
#define HEADER_H 24
#define MARGIN 6
#define GUTTER 4
#define BAR_W (SCREEN_W - (MARGIN * 2))
#define BAR_H 44

#define REFRESH_MS 2000UL
#define HISTORY_SIZE 30
#define SPARKLINE_H 28
#define WIFI_RETRY_MS 10000UL
#define DHT_WAIT_MS 2000UL

// MQTT CLIENT CONFIG  
static const char* pubtopic      = "620171573";                    // Add your ID number here
static const char* subtopic[]    = {"620171573_sub","/elet2415"};  // Array of Topics(Strings) to subscribe to
static const char* mqtt_server   = "www.yanacreations.com";         // Broker IP address or Domain name as a String 
static uint16_t mqtt_port        = 1883;

static const char *ssid = "Rakoon";
static const char *password = "i_isARakoon";

// TASK HANDLES 
TaskHandle_t xMQTT_Connect          = NULL; 
TaskHandle_t xNTPHandle             = NULL;  
TaskHandle_t xLOOPHandle            = NULL;  
TaskHandle_t xUpdateHandle          = NULL;
TaskHandle_t xButtonCheckeHandle    = NULL;  

typedef struct {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
} Rect;

static const Rect BAR_1 = {MARGIN, HEADER_H + MARGIN, BAR_W, BAR_H};
static const Rect BAR_2 = {MARGIN, HEADER_H + MARGIN + (BAR_H + GUTTER) * 1, BAR_W, BAR_H};
static const Rect BAR_3 = {MARGIN, HEADER_H + MARGIN + (BAR_H + GUTTER) * 2, BAR_W, BAR_H};
static const Rect BAR_4 = {MARGIN, HEADER_H + MARGIN + (BAR_H + GUTTER) * 3, BAR_W, BAR_H};
static const Rect BAR_5 = {MARGIN, HEADER_H + MARGIN + (BAR_H + GUTTER) * 4, BAR_W, BAR_H};
static const Rect BAR_6 = {MARGIN, HEADER_H + MARGIN + (BAR_H + GUTTER) * 5, BAR_W, BAR_H};
static const Rect CLOCK_AREA = {SCREEN_W - 60, 0, 54, HEADER_H};

static Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
static Adafruit_BMP280 bmp;
static DHT dht(DHT_PIN, DHT_TYPE);

static uint16_t bg_color;
static uint16_t card_color;
static uint16_t label_color;
static uint16_t value_color;
static uint16_t good_color;
static uint16_t bad_color;

static unsigned long last_refresh = 0;
static unsigned long last_wifi_retry = 0;
static unsigned long last_dht_read = 0;

static float last_dht_temp = NAN;
static float last_dht_hum = NAN;
static int bmp_ready = 0;

static float current_temp = NAN;
static float current_hum = NAN;
static float current_pressure = NAN;
static float current_altitude = NAN;
static float current_soil = NAN;
static float current_heat_index = NAN;
static int current_online = 0;
static int current_rssi = 0;
static unsigned long current_uptime = 0;

static char shown_temp[16] = "";
static char shown_hum[16] = "";
static char shown_press[16] = "";
static char shown_alt[16] = "";
static char shown_soil[16] = "";
static char shown_rssi_bar[16] = "";
static char shown_clock[16] = "";
static char shown_rssi_hdr[8] = "";
static char shown_heat[16] = "";
static int shown_online = -1;
static int first_frame = 1;

static float temp_history[HISTORY_SIZE];
static float hum_history[HISTORY_SIZE];
static float press_history[HISTORY_SIZE];
static float alt_history[HISTORY_SIZE];
static float soil_history[HISTORY_SIZE];
static float rssi_history[HISTORY_SIZE];
static float heat_history[HISTORY_SIZE];
static uint8_t history_index = 0;

static uint16_t temp_spark_color;
static uint16_t hum_spark_color;
static uint16_t press_spark_color;
static uint16_t alt_spark_color;
static uint16_t soil_spark_color;
static uint16_t rssi_spark_color;
static uint16_t heat_spark_color;

static int bmp280_init(void);
static int bmp280_read(void);
static int dht22_read(void);
static int soil_read(void);
static void wifi_connect(void);
static void wifi_check(void);
static void draw_shell(void);
static void draw_label(Rect r, const char *text);
static void draw_bar(Rect r, const char *label, const char *value, const char *unit, float *history, uint16_t spark_color);
static void draw_sparkline(GFXcanvas16 *canvas, int16_t x, int16_t y, int16_t w, int16_t h, float *history, uint16_t color);
static void draw_diff(Rect r, const char *old_text, const char *new_text, uint16_t fg, uint16_t bg);
static void history_push(float *history, float value);
static void history_minmax(float *history, uint8_t count, float *min_out, float *max_out);
static void render_values(void);
static void log_values(void);

static double convert_Celsius_to_fahrenheit(double c) {
    return (9.0/5.0)*c+32.0;
}

static double convert_fahrenheit_to_Celsius(double f) {
    return (f-32.0)*5.0/9.0;
}

static double calcHeatIndex(double Temp, double Humid) {
    Temp = convert_Celsius_to_fahrenheit(Temp);
    double HumidIndex = -42.379 + (2.04901523 * Temp) + (10.14333127 * Humid) - (0.22475541 * Temp * Humid) - (0.00683783 * Temp * Temp) - (0.05481717 * Humid * Humid) + (0.00122874 * Temp * Temp * Humid) + (0.00085282 * Temp * Humid * Humid) - (0.00000199 * Temp * Temp * Humid * Humid);
    return convert_fahrenheit_to_Celsius(HumidIndex);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  pinMode(SOIL_PIN, INPUT);
  dht.begin();
  wifi_connect();

  if (WiFi.status() == WL_CONNECTED) {
    vNTPFunction();
  }

  tft.begin();
  tft.setRotation(0);
  tft.setTextWrap(false);

  bg_color = tft.color565(0, 102, 255);
  card_color = tft.color565(10, 16, 36);
  label_color = tft.color565(255, 59, 59);
  value_color = tft.color565(255, 255, 255);
  good_color = tft.color565(0, 230, 118);
  bad_color = tft.color565(255, 82, 82);

  temp_spark_color = tft.color565(255, 152, 0);
  hum_spark_color = tft.color565(0, 188, 212);
  press_spark_color = tft.color565(76, 175, 80);
  alt_spark_color = tft.color565(255, 235, 59);
  rssi_spark_color = tft.color565(156, 39, 176);
  soil_spark_color = tft.color565(205, 220, 57);
  heat_spark_color = tft.color565(255, 87, 34);

  bmp280_init();
  draw_shell();

  dht22_read();
  bmp280_read();
  soil_read();
  current_online = WiFi.status() == WL_CONNECTED;
  current_rssi = current_online ? WiFi.RSSI() : 0;
  current_uptime = millis() / 1000UL;
  if (!isnan(current_temp) && !isnan(current_hum)) {
    current_heat_index = (float)calcHeatIndex((double)current_temp, (double)current_hum);
  }
  render_values();
  log_values();
  last_refresh = millis();
}

void loop() {
  unsigned long now = millis();
  wifi_check();

  if (now - last_refresh < REFRESH_MS) {
    return;
  }

  dht22_read();
  bmp280_read();
  soil_read();
  current_online = WiFi.status() == WL_CONNECTED;
  current_rssi = current_online ? WiFi.RSSI() : 0;
  current_uptime = now / 1000UL;
  if (!isnan(current_temp) && !isnan(current_hum)) {
    current_heat_index = (float)calcHeatIndex((double)current_temp, (double)current_hum);
  }
  render_values();
  log_values();
  last_refresh = now;
}

static void wifi_connect(void) {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000UL) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nwifi ok");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nwifi fail");
    last_wifi_retry = millis();
  }
}

static void wifi_check(void) {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  if (millis() - last_wifi_retry < WIFI_RETRY_MS) {
    return;
  }
  Serial.println("wifi retry");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  last_wifi_retry = millis();

  // If NTP task was not created yet (e.g. initial connection failed), try to create it now
  if (WiFi.status() == WL_CONNECTED && xNTPHandle == NULL) {
    vNTPFunction();
  }
}

static int dht22_read(void) {
  unsigned long now = millis();

  if (now - last_dht_read < DHT_WAIT_MS && !isnan(last_dht_temp) && !isnan(last_dht_hum)) {
    current_temp = last_dht_temp;
    current_hum = last_dht_hum;
    return 1;
  }

  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  if (isnan(temp) || isnan(hum)) {
    Serial.println("dht fail");
    return 0;
  }

  last_dht_read = now;
  last_dht_temp = temp;
  last_dht_hum = hum;
  current_temp = temp;
  current_hum = hum;
  return 1;
}

static int bmp280_init(void) {
  bmp_ready = 0;
  if (!bmp.begin(BMP280_ADDR)) {
    Serial.println("bmp280 fail");
    return 0;
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X1,
                  Adafruit_BMP280::SAMPLING_X4,
                  Adafruit_BMP280::FILTER_OFF,
                  Adafruit_BMP280::STANDBY_MS_500);
  bmp_ready = 1;
  return 1;
}

static int bmp280_read(void) {
  if (!bmp_ready) {
    return 0;
  }
  current_pressure = bmp.readPressure() / 100.0f;
  current_altitude = bmp.readAltitude(SEA_LEVEL_HPA);
  if (isnan(current_pressure) || current_pressure <= 0.0f) {
    Serial.println("bmp280 fail");
    return 0;
  }
  return 1;
}

static int soil_read(void) {
  int raw = analogRead(SOIL_PIN);
  Serial.print("Soil: ");
  Serial.print(raw);
  // Map raw value to 0-100%. Note: Higher raw usually means drier (depending on sensor).
  // Formula: (raw - dry) * 100 / (wet - dry)
  float perc = (float)(raw - SOIL_DRY) * 100.0f / (float)(SOIL_WET - SOIL_DRY);
  current_soil = constrain(perc, 0.0f, 100.0f);
  return 1;
}

static void draw_shell(void) {
  tft.fillScreen(bg_color);
  tft.fillRoundRect(BAR_1.x, BAR_1.y, BAR_1.w, BAR_1.h, 4, card_color);
  tft.fillRoundRect(BAR_2.x, BAR_2.y, BAR_2.w, BAR_2.h, 4, card_color);
  tft.fillRoundRect(BAR_3.x, BAR_3.y, BAR_3.w, BAR_3.h, 4, card_color);
  tft.fillRoundRect(BAR_4.x, BAR_4.y, BAR_4.w, BAR_4.h, 4, card_color);
  tft.fillRoundRect(BAR_5.x, BAR_5.y, BAR_5.w, BAR_5.h, 4, card_color);
  tft.fillRoundRect(BAR_6.x, BAR_6.y, BAR_6.w, BAR_6.h, 4, card_color);

  tft.setFont(NULL);
  tft.setTextColor(label_color);
  tft.setTextSize(1);
  tft.setCursor(MARGIN + 12, 8);
  tft.print("WIFI");
}

static void draw_label(Rect r, const char *text) {
  tft.setFont(NULL);
  tft.setTextColor(label_color);
  tft.setTextSize(1);
  tft.setCursor(r.x + 6, r.y + 4);
  tft.print(text);
}

static void draw_bar(Rect r, const char *label, const char *value, const char *unit, float *history, uint16_t spark_color) {
  int16_t area_x = r.x + 2;
  int16_t area_y = r.y + 2;
  int16_t area_w = r.w - 4;
  int16_t area_h = r.h - 4;

  GFXcanvas16 canvas(area_w, area_h);
  canvas.fillScreen(card_color);

  // Label (Top Left)
  canvas.setFont(NULL);
  canvas.setTextColor(label_color);
  canvas.setTextSize(1);
  canvas.setCursor(4, 2);
  canvas.print(label);

  // Sparkline (Bottom / Right overlap)
  draw_sparkline(&canvas, 0, area_h - SPARKLINE_H, area_w, SPARKLINE_H, history, spark_color);

  // Value and Unit (Top Right)
  char full_val[32];
  snprintf(full_val, sizeof(full_val), "%s %s", value, unit);
  canvas.setFont(&FreeSansBold9pt7b);
  canvas.setTextColor(value_color);
  
  int16_t x1, y1;
  uint16_t w, h;
  canvas.getTextBounds(full_val, 0, 0, &x1, &y1, &w, &h);
  canvas.setCursor(area_w - w - 4, 14);
  canvas.print(full_val);

  tft.drawRGBBitmap(area_x, area_y, canvas.getBuffer(), canvas.width(), canvas.height());
}

static void draw_diff(Rect r, const char *old_text, const char *new_text, uint16_t fg, uint16_t bg) {
  size_t old_len = strlen(old_text);
  size_t new_len = strlen(new_text);
  size_t n = old_len > new_len ? old_len : new_len;

  tft.setFont(NULL);
  tft.setTextSize(1);

  for (size_t i = 0; i < n && i < (size_t)(r.w / 6); i++) {
    char a = i < old_len ? old_text[i] : ' ';
    char b = i < new_len ? new_text[i] : ' ';
    if (a == b) {
      continue;
    }
    tft.drawChar(r.x + (int16_t)(i * 6), r.y + ((r.h - 8) / 2), b, fg, bg, 1);
  }
}

static void history_push(float *history, float value) {
  history[history_index] = value;
}

static void history_minmax(float *history, uint8_t count, float *min_out, float *max_out) {
  *min_out = history[0];
  *max_out = history[0];
  for (uint8_t i = 1; i < count; i++) {
    if (history[i] < *min_out) *min_out = history[i];
    if (history[i] > *max_out) *max_out = history[i];
  }
  if (*max_out == *min_out) {
    *max_out += 0.1f;
    *min_out -= 0.1f;
  }
}

static void draw_sparkline(GFXcanvas16 *canvas, int16_t x, int16_t y, int16_t w, int16_t h, float *history, uint16_t color) {
  uint8_t count = (history_index < HISTORY_SIZE) ? history_index : HISTORY_SIZE;
  if (count < 2) return;

  float min_val, max_val;
  history_minmax(history, count, &min_val, &max_val);

  float range = max_val - min_val;
  if (range < 0.001f) range = 1.0f;

  uint16_t baseline_color = ((uint16_t)(30 & 0xF8) << 8) | ((uint16_t)(30 & 0xFC) << 3) | (30 >> 3);
  canvas->drawFastHLine(x, y + h - 1, w, baseline_color);

  int16_t prev_x = -1;
  int16_t prev_y = -1;

  for (uint8_t i = 0; i < count; i++) {
    int16_t px = x + (int16_t)((i * (w - 1)) / (HISTORY_SIZE - 1));
    int16_t py = y + h - (int16_t)(((history[i] - min_val) / range) * (h - 2)) - 1;

    if (prev_x >= 0) {
      canvas->drawLine(prev_x, prev_y, px, py, color);
    }
    prev_x = px;
    prev_y = py;
  }
}


static void render_values(void) {
  if (!isnan(current_temp)) history_push(temp_history, current_temp);
  if (!isnan(current_hum)) history_push(hum_history, current_hum);
  if (!isnan(current_pressure)) history_push(press_history, current_pressure);
  if (!isnan(current_altitude)) history_push(alt_history, current_altitude);
  history_push(rssi_history, (float)current_rssi);
  if (!isnan(current_soil)) history_push(soil_history, current_soil);
  if (!isnan(current_heat_index)) history_push(heat_history, current_heat_index);

  uint8_t next_index = history_index + 1;
  if (next_index >= HISTORY_SIZE) next_index = 0;
  history_index = next_index;

  char temp[16], hum[16], press[16], alt[16], rssi_bar[16], soil[16], clock_text[16], heat[16];

  if (isnan(current_temp)) snprintf(temp, sizeof(temp), "--");
  else snprintf(temp, sizeof(temp), "%.1f", current_temp);
  
  if (isnan(current_hum)) snprintf(hum, sizeof(hum), "--");
  else snprintf(hum, sizeof(hum), "%.1f", current_hum);
  
  if (isnan(current_pressure)) snprintf(press, sizeof(press), "--");
  else snprintf(press, sizeof(press), "%.1f", current_pressure);

  if (isnan(current_altitude)) snprintf(alt, sizeof(alt), "--");
  else snprintf(alt, sizeof(alt), "%.1f", current_altitude);
  
  snprintf(rssi_bar, sizeof(rssi_bar), "%d", current_rssi);
  
  if (isnan(current_soil)) snprintf(soil, sizeof(soil), "--");
  else snprintf(soil, sizeof(soil), "%.1f", current_soil);

  if (isnan(current_heat_index)) snprintf(heat, sizeof(heat), "--");
  else snprintf(heat, sizeof(heat), "%.1f", current_heat_index);

  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 0)) {
    strftime(clock_text, sizeof(clock_text), "%H:%M:%S", &timeinfo);
  } else {
    snprintf(clock_text, sizeof(clock_text), "%02lu:%02lu:%02lu",
             (current_uptime / 3600UL) % 24UL,
             (current_uptime / 60UL) % 60UL,
             current_uptime % 60UL);
  }

  if (first_frame || current_online != shown_online) {
    tft.fillCircle(MARGIN + 4, 11, 3, current_online ? good_color : bad_color);
    shown_online = current_online;
  }

  char rssi_hdr[8];
  if (current_online) {
    snprintf(rssi_hdr, sizeof(rssi_hdr), "%ddB", current_rssi);
  } else {
    snprintf(rssi_hdr, sizeof(rssi_hdr), "--");
  }

  if (first_frame || strcmp(rssi_hdr, shown_rssi_hdr) != 0) {
    tft.setFont(NULL);
    tft.setTextColor(current_online ? label_color : bad_color, bg_color);
    tft.setTextSize(1);
    tft.setCursor(MARGIN + 40, 8);
    tft.print(rssi_hdr);
    strncpy(shown_rssi_hdr, rssi_hdr, sizeof(shown_rssi_hdr) - 1);
  }

  if (first_frame || strcmp(clock_text, shown_clock) != 0) {
    draw_diff(CLOCK_AREA, shown_clock, clock_text, value_color, bg_color);
    strncpy(shown_clock, clock_text, sizeof(shown_clock) - 1);
  }

  if (first_frame || strcmp(temp, shown_temp) != 0) {
    draw_bar(BAR_1, "TEMPERATURE", temp, "C", temp_history, temp_spark_color);
    strncpy(shown_temp, temp, sizeof(shown_temp) - 1);
  }
  if (first_frame || strcmp(hum, shown_hum) != 0) {
    draw_bar(BAR_2, "HUMIDITY", hum, "%", hum_history, hum_spark_color);
    strncpy(shown_hum, hum, sizeof(shown_hum) - 1);
  }
  if (first_frame || strcmp(press, shown_press) != 0) {
    draw_bar(BAR_3, "PRESSURE", press, "hPa", press_history, press_spark_color);
    strncpy(shown_press, press, sizeof(shown_press) - 1);
  }
  if (first_frame || strcmp(alt, shown_alt) != 0) {
    draw_bar(BAR_4, "ALTITUDE", alt, "m", alt_history, alt_spark_color);
    strncpy(shown_alt, alt, sizeof(shown_alt) - 1);
  }
  if (first_frame || strcmp(heat, shown_heat) != 0) {
    draw_bar(BAR_5, "HEAT INDEX", heat, "C", heat_history, heat_spark_color);
    strncpy(shown_heat, heat, sizeof(shown_heat) - 1);
  }
  if (first_frame || strcmp(soil, shown_soil) != 0) {
    draw_bar(BAR_6, "SOIL MOISTURE", soil, "%", soil_history, soil_spark_color);
    strncpy(shown_soil, soil, sizeof(shown_soil) - 1);
  }

  first_frame = 0;
}

static void log_values(void) {
  Serial.print("Temp: ");
  Serial.print(current_temp, 1);
  Serial.print(" C  Hum: ");
  Serial.print(current_hum, 1);
  Serial.print(" %  Pres: ");
  Serial.print(current_pressure, 1);
  Serial.print(" hPa  Alt: ");
  Serial.print(current_altitude, 1);
  Serial.print(" m  Heat: ");
  Serial.print(current_heat_index, 1);
  Serial.print(" C  RSSI: ");
  Serial.print(current_rssi);
  Serial.print(" dB  Soil: ");
  Serial.print(current_soil, 1);
  Serial.println(" %");
}