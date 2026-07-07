#include "config.h"

#include "secrets.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_system.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================================================
// HP12 v1.6.0
// Clean UI + line gauge + advice button.
// This code is intentionally verbose and commented for future maintenance.
// ============================================================================

// =========================
// FALLBACK CONFIG
// =========================

#ifndef BUZZER_ACTIVE_LOW
#define BUZZER_ACTIVE_LOW 0
#endif

#ifndef LED_ACTIVE_LOW
#define LED_ACTIVE_LOW 0
#endif

#ifndef BUTTON_NAV_ACTIVE_LOW
#define BUTTON_NAV_ACTIVE_LOW 1
#endif

#ifndef BUTTON_ADVICE_ACTIVE_LOW
#define BUTTON_ADVICE_ACTIVE_LOW 1
#endif

#ifndef ONBOARD_BLUE_LED_ENABLED
#define ONBOARD_BLUE_LED_ENABLED 0
#endif

#ifndef ONBOARD_BLUE_LED_ACTIVE_LOW
#define ONBOARD_BLUE_LED_ACTIVE_LOW 0
#endif

#ifndef SCROLL_UPDATE_INTERVAL_MS
#define SCROLL_UPDATE_INTERVAL_MS 150UL
#endif

#ifndef BUZZER_PROFILE_START
#define BUZZER_PROFILE_START 1
#endif

#ifndef LIGHT_SENSOR_ENABLED
#define LIGHT_SENSOR_ENABLED 0
#endif

#ifndef LIGHT_ADC_BRIGHT_IS_HIGH
#define LIGHT_ADC_BRIGHT_IS_HIGH 1
#endif


#ifndef FORCE_FIRST_SYNC_ENABLED
#define FORCE_FIRST_SYNC_ENABLED 1
#endif

#ifndef ATTRIBUTES_RESEND_MS
#define ATTRIBUTES_RESEND_MS 300000UL
#endif

#ifndef MQTT_BUFFER_SIZE
#define MQTT_BUFFER_SIZE 768
#endif

#ifndef MQTT_CLIENT_ID_PREFIX
#define MQTT_CLIENT_ID_PREFIX "HP12_"
#endif


#ifndef OLED_SAFE_X
#define OLED_SAFE_X 0
#endif

#ifndef OLED_SAFE_W
#define OLED_SAFE_W 120
#endif

#ifndef OLED_SAFE_RIGHT
#define OLED_SAFE_RIGHT (OLED_SAFE_X + OLED_SAFE_W - 1)
#endif

#ifndef OLED_GAUGE_X
#define OLED_GAUGE_X 4
#endif

#ifndef OLED_GAUGE_W
#define OLED_GAUGE_W 112
#endif

// =========================
// OBJECTS
// =========================

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// =========================
// STATE TYPES
// =========================

enum ComfortState {
  STATE_STARTUP,
  STATE_SENSOR_ERROR,
  STATE_SENSOR_SUSPECT,
  STATE_GOOD,
  STATE_WARN,
  STATE_DANGER
};

enum ThermalState {
  THERMAL_UNKNOWN,
  THERMAL_GOOD,
  THERMAL_WARN,
  THERMAL_DANGER
};

enum LightState {
  LIGHT_UNKNOWN,
  LIGHT_DARK,
  LIGHT_GOOD,
  LIGHT_BRIGHT,
  LIGHT_GLARE
};

enum DisplayPage {
  PAGE_HOME,
  PAGE_FOCUS,
  PAGE_HEAT,
  PAGE_TEMP,
  PAGE_HUM,
  PAGE_LIGHT,
  PAGE_BASIS,
  PAGE_ADVICE,
  PAGE_HELP
};

enum AdviceTopic {
  ADVICE_NOW,
  ADVICE_REASON,
  ADVICE_ACTION,
  ADVICE_BASIS
};

enum AlarmProfile {
  ALARM_QUIET = 0,
  ALARM_NORMAL = 1,
  ALARM_STRONG = 2
};

enum ButtonEvent {
  BTN_NONE,
  BTN_SINGLE,
  BTN_DOUBLE,
  BTN_TRIPLE,
  BTN_LONG
};

struct ButtonRuntime {
  bool stablePressed;
  bool lastReadingPressed;
  unsigned long lastChangeMs;
  unsigned long pressedAtMs;
  bool longHandled;
  uint8_t clickCount;
  unsigned long lastClickMs;
};

// =========================
// GLOBAL STATE
// =========================

ComfortState comfortState = STATE_STARTUP;
ThermalState thermalState = THERMAL_UNKNOWN;
LightState lightState = LIGHT_UNKNOWN;

DisplayPage currentPage = PAGE_HOME;
AdviceTopic adviceTopic = ADVICE_NOW;
AlarmProfile alarmProfile = (AlarmProfile)BUZZER_PROFILE_START;

ButtonRuntime navButton = {false, false, 0, 0, false, 0, 0};
ButtonRuntime adviceButton = {false, false, 0, 0, false, 0, 0};

float rawTempC = NAN;
float rawHumidity = NAN;

float tempC = NAN;
float humidity = NAN;
float heatIndexC = NAN;

int lightRaw = 0;
float lightRawFiltered = NAN;
float lightPercent = NAN;
bool lightDigital = false;
bool lightReady = false;

float focusScore = 0.0;

bool oledReady = false;
bool sensorHasRawData = false;
bool sensorHasValidData = false;
bool lastSensorReadOk = false;

unsigned long lastSensorReadMs = 0;
unsigned long lastLightReadMs = 0;
unsigned long lastOledUpdateMs = 0;
unsigned long lastLedUpdateMs = 0;
unsigned long lastBlueLedUpdateMs = 0;
unsigned long lastScrollMs = 0;
unsigned long blueSelfTestUntilMs = 0;

bool blinkState = false;
int scrollX = OLED_SAFE_RIGHT;

unsigned long alarmMutedUntilMs = 0;
unsigned long helpPageUntilMs = 0;

// =========================
// CLOUD STATE
// =========================
//
// forceFirstTelemetry = true moi khi moi ket noi MQTT.
// Muc tieu: ThingsBoard co so ngay sau khi thiet bi online.
unsigned long lastWifiRetryMs = 0;
unsigned long lastMqttRetryMs = 0;
unsigned long lastTelemetryMs = 0;
unsigned long lastAttributesMs = 0;

bool forceFirstTelemetry = true;
bool attributesSentOnce = false;
bool cloudEverConnected = false;

// =========================
// BASIC IO HELPERS
// =========================

bool readActiveLowAware(int pin, bool activeLow) {
  bool level = digitalRead(pin);
  return activeLow ? (level == LOW) : (level == HIGH);
}

bool isNavButtonPressedRaw() {
  return readActiveLowAware(BUTTON_NAV_PIN, BUTTON_NAV_ACTIVE_LOW);
}

bool isAdviceButtonPressedRaw() {
  return readActiveLowAware(BUTTON_ADVICE_PIN, BUTTON_ADVICE_ACTIVE_LOW);
}

void buzzerWrite(bool on) {
#if BUZZER_ACTIVE_LOW
  digitalWrite(BUZZER_PIN, on ? LOW : HIGH);
#else
  digitalWrite(BUZZER_PIN, on ? HIGH : LOW);
#endif
}

void ledWrite(int pin, bool on) {
#if LED_ACTIVE_LOW
  digitalWrite(pin, on ? LOW : HIGH);
#else
  digitalWrite(pin, on ? HIGH : LOW);
#endif
}

void blueLedWrite(bool on) {
#if ONBOARD_BLUE_LED_ENABLED
#if ONBOARD_BLUE_LED_ACTIVE_LOW
  digitalWrite(ONBOARD_BLUE_LED_PIN, on ? LOW : HIGH);
#else
  digitalWrite(ONBOARD_BLUE_LED_PIN, on ? HIGH : LOW);
#endif
#endif
}

void allOutputsOff() {
  ledWrite(LED_GREEN_PIN, false);
  ledWrite(LED_YELLOW_PIN, false);
  ledWrite(LED_RED_PIN, false);
  blueLedWrite(false);
  buzzerWrite(false);
}

bool alarmIsMuted() {
  return millis() < alarmMutedUntilMs;
}

void resetScroll() {
  scrollX = OLED_SAFE_RIGHT;
}

float clampFloat(float value, float minValue, float maxValue) {
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}

// =========================
// TEXT HELPERS
// =========================

const char* getStateCode() {
  switch (comfortState) {
    case STATE_GOOD:           return "OK";
    case STATE_WARN:           return "CANH";
    case STATE_DANGER:         return "NONG";
    case STATE_SENSOR_ERROR:   return "DHT";
    case STATE_SENSOR_SUSPECT: return "CHK";
    default:                   return "START";
  }
}

const char* getStateText() {
  switch (comfortState) {
    case STATE_GOOD:           return "ON DINH";
    case STATE_WARN:           return "CANH BAO";
    case STATE_DANGER:         return "NONG AM";
    case STATE_SENSOR_ERROR:   return "LOI DHT";
    case STATE_SENSOR_SUSPECT: return "KIEM TRA";
    default:                   return "KHOI DONG";
  }
}

const char* getThermalText() {
  switch (thermalState) {
    case THERMAL_GOOD:   return "NHIET TOT";
    case THERMAL_WARN:   return "NHIET LECH";
    case THERMAL_DANGER: return "NHIET CAO";
    default:             return "NHIET ?";
  }
}

const char* getLightText() {
  switch (lightState) {
    case LIGHT_DARK:   return "TOI";
    case LIGHT_GOOD:   return "DU SANG";
    case LIGHT_BRIGHT: return "HOI SANG";
    case LIGHT_GLARE:  return "SANG GAT";
    default:           return "SANG ?";
  }
}

const char* getFocusText() {
  if (focusScore >= 80.0) return "DE TAP TRUNG";
  if (focusScore >= 55.0) return "CAN CHINH";
  return "GAY MET MOI";
}

const char* getShortActionText() {
  if (comfortState == STATE_SENSOR_ERROR) return "KIEM TRA DAY";
  if (comfortState == STATE_SENSOR_SUSPECT) return "KIEM TRA DHT";
  if (thermalState == THERMAL_DANGER) return "LAM MAT NGAY";

#if LIGHT_SENSOR_ENABLED
  if (lightState == LIGHT_DARK) return "TANG DEN";
  if (lightState == LIGHT_GLARE) return "GIAM CHOI";
#endif

  if (thermalState == THERMAL_WARN) return "THONG GIO";
  if (lightState == LIGHT_BRIGHT) return "GIAM SANG";
  if (comfortState == STATE_GOOD) return "GIU ON DINH";

  return "THEO DOI";
}

const char* getAlarmProfileText() {
  switch (alarmProfile) {
    case ALARM_QUIET:  return "IM";
    case ALARM_STRONG: return "MANH";
    default:           return "VUA";
  }
}

const char* getAdviceTitle() {
  switch (adviceTopic) {
    case ADVICE_REASON: return "VI SAO";
    case ADVICE_ACTION: return "LAM GI";
    case ADVICE_BASIS:  return "CO SO";
    default:            return "CAM NHAN";
  }
}

const char* getAdviceMessage() {
  if (comfortState == STATE_SENSOR_ERROR) {
    return "DHT22 khong phan hoi. Kiem tra 3V3, GND, DATA GPIO va dau noi truoc khi tin vao canh bao.";
  }

  if (comfortState == STATE_SENSOR_SUSPECT) {
    return "Du lieu DHT22 co dau hieu bat thuong. Kiem tra cam bien, day DATA, dien tro keo 10K va do am tren dau cam bien.";
  }

  switch (adviceTopic) {
    case ADVICE_REASON:
      if (thermalState == THERMAL_DANGER) {
        return "Nong am cao lam co the tang ganh dieu hoa nhiet, de met moi, cham suy nghi va kho duy tri chu y.";
      }
      if (lightState == LIGHT_DARK) {
        return "Thieu sang lam mat phai dieu tiet nhieu hon, de moi mat va lam giam kha nang tap trung lau.";
      }
      if (lightState == LIGHT_GLARE) {
        return "Sang gat hoac chieu thang vao mat/man hinh de gay loe, moi mat va lam roi nhip lam viec.";
      }
      return "Khi nhiet am va anh sang on dinh, co the it bi xao nhang sinh ly hon, de giu nhip lam viec sau.";

    case ADVICE_ACTION:
      if (thermalState == THERMAL_DANGER) {
        return "Uu tien: bat quat hoac thong gio, lam mat phong, uong nuoc, nghi ngan 3-5 phut, tranh tiep tuc lam viec cang thang.";
      }
      if (thermalState == THERMAL_WARN) {
        return "Nen tang luu thong gio, giam nguon nhiet, kiem tra dieu hoa/quat va theo doi lai sau vai phut.";
      }
      if (lightState == LIGHT_DARK) {
        return "Tang den ban, dua nguon sang gan mat ban lam viec, tranh de cam bien nam trong bong toi cuc bo.";
      }
      if (lightState == LIGHT_GLARE) {
        return "Doi goc chieu, che bot den truc tiep, tranh anh sang dap vao mat hoac phan xa tren man hinh.";
      }
      return "Giu dieu kien hien tai: thong gio nhe, anh sang on dinh, tranh tang them nguon nhiet trong phong.";

    case ADVICE_BASIS:
      return "HP12 la comfort proxy: dung nhiet do, do am, heat index va anh sang tuong doi. Chua thay the danh gia y te hay ASHRAE day du.";

    case ADVICE_NOW:
    default:
      if (thermalState == THERMAL_DANGER) {
        return "Khong gian dang nong am ro. Cam nhan nhiet cao co the pha vo kha nang tap trung, can hanh dong ngay.";
      }
      if (thermalState == THERMAL_WARN) {
        return "Khong gian dang lech khoi vung de tap trung. Nen dieu chinh som truoc khi co the met va giam chu y.";
      }
      if (lightState == LIGHT_DARK) {
        return "Anh sang dang thieu cho lam viec. Mat co the nhanh moi hon, nen tang sang tai mat ban.";
      }
      if (lightState == LIGHT_GLARE) {
        return "Anh sang dang gat. Nen giam chieu truc tiep de tranh moi mat va loe man hinh.";
      }
      return "Khong gian dang kha on. Hay giu nhiet am, thong gio va anh sang o muc on dinh de ho tro lam viec sau.";
  }
}

const char* getScrollingMessage() {
  if (currentPage == PAGE_ADVICE) {
    return getAdviceMessage();
  }

  if (alarmIsMuted() && comfortState == STATE_DANGER) {
    return "DA TAM IM COI: LED va man hinh van canh bao. Khong gian van nong am, can lam mat.";
  }

  if (comfortState == STATE_SENSOR_ERROR) {
    return "LOI CAM BIEN: DHT22 khong phan hoi. Kiem tra 3V3, GND, DATA GPIO va dau noi.";
  }

  if (comfortState == STATE_SENSOR_SUSPECT) {
    return "DU LIEU NGHI NGO: Kiem tra DHT22, day DATA, dien tro keo 10K va do am tren dau cam bien.";
  }

  if (thermalState == THERMAL_DANGER) {
    return "NONG AM CAO: Cam nhan nhiet vuot vung tap trung. Lam mat phong, tang gio, uong nuoc, nghi ngan.";
  }

#if LIGHT_SENSOR_ENABLED
  if (lightState == LIGHT_DARK) {
    return "THIEU SANG: Mat phai dieu tiet nhieu hon, de giam tap trung. Tang den ban hoac dua cam bien gan mat ban.";
  }

  if (lightState == LIGHT_GLARE) {
    return "SANG GAT: De moi mat va loe man hinh. Giam den truc tiep, doi goc chieu hoac che bot anh sang.";
  }

  if (lightState == LIGHT_BRIGHT) {
    return "HOI SANG: Van lam viec duoc nhung nen tranh anh sang chieu thang vao mat hoac man hinh.";
  }
#endif

  if (thermalState == THERMAL_WARN) {
    return "CANH BAO: Nhiet am bat dau lech vung tap trung. Nen bat quat, mo thong gio, giam nguon nhiet.";
  }

  if (comfortState == STATE_GOOD) {
    return "CAM NHAN TOT: Nhiet am va anh sang dang ho tro tap trung. Giu thong gio va nguon sang on dinh.";
  }

  return "HP12 dang doc khong gian: nhiet do, do am, cam nhan nhiet va anh sang tuong doi.";
}

// =========================
// CLASSIFICATION
// =========================

bool isSensorValueReasonable(float t, float h) {
  if (isnan(t) || isnan(h)) return false;

  if (t < SENSOR_TEMP_MIN || t > SENSOR_TEMP_MAX) return false;
  if (h < SENSOR_HUM_MIN || h > SENSOR_HUM_MAX) return false;

  if (h >= SENSOR_HUM_SUSPECT) return false;

  return true;
}

ThermalState classifyThermal(float t, float h, float hi) {
  if (isnan(t) || isnan(h) || isnan(hi)) {
    return THERMAL_UNKNOWN;
  }

  if (hi >= HEAT_INDEX_DANGER || t >= TEMP_DANGER || h >= HUM_DANGER) {
    return THERMAL_DANGER;
  }

  if (
    hi >= HEAT_INDEX_WARN ||
    t >= TEMP_WARN ||
    h >= HUM_WARN ||
    t < TEMP_GOOD_MIN ||
    h < HUM_GOOD_MIN
  ) {
    return THERMAL_WARN;
  }

  if (
    t >= TEMP_GOOD_MIN &&
    t <= TEMP_GOOD_MAX &&
    h >= HUM_GOOD_MIN &&
    h <= HUM_GOOD_MAX
  ) {
    return THERMAL_GOOD;
  }

  return THERMAL_WARN;
}

LightState classifyLight(float pct) {
#if LIGHT_SENSOR_ENABLED
  if (isnan(pct)) return LIGHT_UNKNOWN;

  if (pct < LIGHT_DARK_PCT) return LIGHT_DARK;
  if (pct >= LIGHT_GLARE_PCT) return LIGHT_GLARE;
  if (pct > LIGHT_FOCUS_MAX_PCT) return LIGHT_BRIGHT;
  if (pct >= LIGHT_FOCUS_MIN_PCT && pct <= LIGHT_FOCUS_MAX_PCT) return LIGHT_GOOD;

  return LIGHT_DARK;
#else
  return LIGHT_UNKNOWN;
#endif
}

// Focus Proxy Score la diem UX noi bo, khong phai chan doan y te.
float calculateFocusScore() {
  if (!sensorHasValidData) return 0.0;

  float score = 100.0;

  if (thermalState == THERMAL_DANGER) score -= 55.0;
  else if (thermalState == THERMAL_WARN) score -= 25.0;

  if (!isnan(heatIndexC)) {
    if (heatIndexC >= HEAT_INDEX_MED_DANGER_C) score -= 15.0;
    if (heatIndexC >= HEAT_INDEX_MED_EXTREME_C) score -= 20.0;
  }

#if LIGHT_SENSOR_ENABLED
  if (lightState == LIGHT_DARK) score -= 22.0;
  if (lightState == LIGHT_GLARE) score -= 22.0;
  if (lightState == LIGHT_BRIGHT) score -= 10.0;
#endif

  if (!isnan(humidity) && humidity > HUM_GOOD_MAX && humidity < HUM_WARN) {
    score -= 8.0;
  }

  return clampFloat(score, 0.0, 100.0);
}

void updateComfortState() {
  if (comfortState == STATE_SENSOR_ERROR || comfortState == STATE_SENSOR_SUSPECT) {
    focusScore = 0.0;
    return;
  }

  if (!sensorHasValidData) {
    comfortState = STATE_STARTUP;
    focusScore = 0.0;
    return;
  }

  focusScore = calculateFocusScore();

  if (thermalState == THERMAL_DANGER) {
    comfortState = STATE_DANGER;
    return;
  }

#if LIGHT_SENSOR_ENABLED
  if (lightState == LIGHT_DARK || lightState == LIGHT_GLARE || lightState == LIGHT_BRIGHT) {
    comfortState = STATE_WARN;
    return;
  }
#endif

  if (thermalState == THERMAL_WARN) {
    comfortState = STATE_WARN;
    return;
  }

  if (thermalState == THERMAL_GOOD) {
    comfortState = STATE_GOOD;
    return;
  }

  comfortState = STATE_WARN;
}

// =========================
// SENSOR READ
// =========================

void readSensorNonBlocking() {
  unsigned long now = millis();

  if (now - lastSensorReadMs < SENSOR_READ_INTERVAL_MS) {
    return;
  }

  lastSensorReadMs = now;

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    lastSensorReadOk = false;
    sensorHasValidData = false;
    comfortState = STATE_SENSOR_ERROR;

    Serial.println("[DHT22] Read failed. Check VCC/GND/DATA.");
    return;
  }

  sensorHasRawData = true;
  rawTempC = t;
  rawHumidity = h;

  if (!isSensorValueReasonable(t, h)) {
    lastSensorReadOk = false;
    sensorHasValidData = false;
    comfortState = STATE_SENSOR_SUSPECT;

    Serial.print("[DHT22] SUSPECT RAW DATA | Temp: ");
    Serial.print(rawTempC);
    Serial.print(" C | Hum: ");
    Serial.print(rawHumidity);
    Serial.println(" %. Check sensor/pullup/wiring.");
    return;
  }

  lastSensorReadOk = true;
  sensorHasValidData = true;

  tempC = t;
  humidity = h;
  heatIndexC = dht.computeHeatIndex(tempC, humidity, false);

  thermalState = classifyThermal(tempC, humidity, heatIndexC);
  updateComfortState();

  Serial.print("[DHT22] OK | T:");
  Serial.print(tempC);
  Serial.print("C H:");
  Serial.print(humidity);
  Serial.print("% HI:");
  Serial.print(heatIndexC);
  Serial.print("C Light:");
  if (!isnan(lightPercent)) Serial.print(lightPercent, 0); else Serial.print("-");
  Serial.print("% Focus:");
  Serial.print(focusScore, 0);
  Serial.print(" State:");
  Serial.print(getStateCode());
  Serial.print(" Page:");
  Serial.print((int)currentPage);
  Serial.print(" Alarm:");
  Serial.print(getAlarmProfileText());
  Serial.print(" Muted:");
  Serial.println(alarmIsMuted() ? "YES" : "NO");
}

void readLightNonBlocking() {
#if LIGHT_SENSOR_ENABLED
  unsigned long now = millis();

  if (now - lastLightReadMs < LIGHT_READ_INTERVAL_MS) {
    return;
  }

  lastLightReadMs = now;

  int raw = analogRead(LIGHT_AO_PIN);
  lightRaw = raw;

  if (isnan(lightRawFiltered)) {
    lightRawFiltered = raw;
  } else {
    lightRawFiltered = lightRawFiltered * 0.75 + raw * 0.25;
  }

#if LIGHT_ADC_BRIGHT_IS_HIGH
  lightPercent = (lightRawFiltered / 4095.0) * 100.0;
#else
  lightPercent = (1.0 - (lightRawFiltered / 4095.0)) * 100.0;
#endif

  lightPercent = clampFloat(lightPercent, 0.0, 100.0);

#if LIGHT_USE_DIGITAL_DO
  lightDigital = digitalRead(LIGHT_DO_PIN);
#endif

  lightReady = true;
  lightState = classifyLight(lightPercent);
  updateComfortState();
#endif
}

// =========================
// BUTTON UX
// =========================

ButtonEvent updateButtonRuntime(ButtonRuntime &btn, bool readingPressed) {
  unsigned long now = millis();
  ButtonEvent event = BTN_NONE;

  if (readingPressed != btn.lastReadingPressed) {
    btn.lastChangeMs = now;
    btn.lastReadingPressed = readingPressed;
  }

  if (now - btn.lastChangeMs >= BUTTON_DEBOUNCE_MS) {
    if (readingPressed != btn.stablePressed) {
      btn.stablePressed = readingPressed;

      if (btn.stablePressed) {
        btn.pressedAtMs = now;
        btn.longHandled = false;
      } else {
        if (!btn.longHandled) {
          btn.clickCount++;
          btn.lastClickMs = now;
        }
      }
    }
  }

  if (btn.stablePressed && !btn.longHandled) {
    if (now - btn.pressedAtMs >= BUTTON_LONG_PRESS_MS) {
      btn.longHandled = true;
      btn.clickCount = 0;
      return BTN_LONG;
    }
  }

  if (btn.clickCount > 0 && (now - btn.lastClickMs >= BUTTON_CLICK_WINDOW_MS)) {
    if (btn.clickCount >= 3) event = BTN_TRIPLE;
    else if (btn.clickCount == 2) event = BTN_DOUBLE;
    else event = BTN_SINGLE;

    btn.clickCount = 0;
  }

  return event;
}

void goMainNextPage() {
  switch (currentPage) {
    case PAGE_HOME:
      currentPage = PAGE_FOCUS;
      break;

    case PAGE_FOCUS:
      currentPage = PAGE_HEAT;
      break;

    case PAGE_HEAT:
      currentPage = PAGE_TEMP;
      break;

    case PAGE_TEMP:
      currentPage = PAGE_HUM;
      break;

    case PAGE_HUM:
      currentPage = PAGE_LIGHT;
      break;

    case PAGE_LIGHT:
      currentPage = PAGE_BASIS;
      break;

    default:
      currentPage = PAGE_HOME;
      break;
  }

  helpPageUntilMs = 0;
  resetScroll();
}

void goAdviceNext() {
  currentPage = PAGE_ADVICE;

  switch (adviceTopic) {
    case ADVICE_NOW:
      adviceTopic = ADVICE_REASON;
      break;

    case ADVICE_REASON:
      adviceTopic = ADVICE_ACTION;
      break;

    case ADVICE_ACTION:
      adviceTopic = ADVICE_BASIS;
      break;

    case ADVICE_BASIS:
    default:
      adviceTopic = ADVICE_NOW;
      break;
  }

  resetScroll();
}

void showHelpPage() {
  currentPage = PAGE_HELP;
  helpPageUntilMs = millis() + HELP_PAGE_DURATION_MS;
  resetScroll();
}

void toggleAlarmMute() {
  if (alarmIsMuted()) alarmMutedUntilMs = 0;
  else alarmMutedUntilMs = millis() + ALARM_MUTE_MS;

  resetScroll();
}

void cycleAlarmProfile() {
  int next = ((int)alarmProfile + 1) % 3;
  alarmProfile = (AlarmProfile)next;
  showHelpPage();
}

void updateButtonsNonBlocking() {
  ButtonEvent navEvent = updateButtonRuntime(navButton, isNavButtonPressedRaw());
  ButtonEvent adviceEvent = updateButtonRuntime(adviceButton, isAdviceButtonPressedRaw());

  if (navEvent == BTN_SINGLE) {
    goMainNextPage();
    Serial.println("[NAV] 1 click: next data page.");
  } else if (navEvent == BTN_DOUBLE) {
    showHelpPage();
    Serial.println("[NAV] 2 clicks: help.");
  } else if (navEvent == BTN_TRIPLE) {
    toggleAlarmMute();
    Serial.println("[NAV] 3 clicks: toggle mute.");
  } else if (navEvent == BTN_LONG) {
    cycleAlarmProfile();
    Serial.print("[NAV] long: alarm profile ");
    Serial.println(getAlarmProfileText());
  }

  if (adviceEvent == BTN_SINGLE) {
    goAdviceNext();
    Serial.println("[ADVICE] 1 click: advice next.");
  } else if (adviceEvent == BTN_DOUBLE) {
    currentPage = PAGE_ADVICE;
    adviceTopic = ADVICE_ACTION;
    resetScroll();
    Serial.println("[ADVICE] 2 clicks: action advice.");
  } else if (adviceEvent == BTN_TRIPLE) {
    currentPage = PAGE_ADVICE;
    adviceTopic = ADVICE_BASIS;
    resetScroll();
    Serial.println("[ADVICE] 3 clicks: basis advice.");
  } else if (adviceEvent == BTN_LONG) {
    currentPage = PAGE_HOME;
    resetScroll();
    Serial.println("[ADVICE] long: back home.");
  }

  if (currentPage == PAGE_HELP && helpPageUntilMs > 0 && millis() > helpPageUntilMs) {
    currentPage = PAGE_HOME;
    helpPageUntilMs = 0;
    resetScroll();
  }
}

// =========================
// OLED UI HELPERS
// =========================

void showBootScreen() {
  if (!oledReady) return;

  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("HP12");

  display.drawLine(OLED_SAFE_X, 12, OLED_SAFE_RIGHT, 12, SSD1306_WHITE);

  display.setCursor(0, 20);
  display.print("FW ");
  display.print(FIRMWARE_VERSION);

  display.setCursor(0, 34);
  display.print("Clean Line Gauge");

  display.setCursor(0, 50);
  display.print("Starting...");
  display.display();
}

void updateScrollPositionNonBlocking() {
  unsigned long now = millis();

  if (now - lastScrollMs < SCROLL_UPDATE_INTERVAL_MS) return;

  lastScrollMs = now;

  const char* msg = getScrollingMessage();
  int msgWidth = strlen(msg) * 6;

  scrollX -= 4;
  if (scrollX < -msgWidth) scrollX = OLED_SAFE_RIGHT;
}

int textWidth6(const char* text) {
  return strlen(text) * 6;
}

void printClippedText(const char* text, int x, int y, int maxPx) {
  display.setCursor(x, y);

  int maxChars = maxPx / 6;
  for (int i = 0; text[i] != '\0' && i < maxChars; i++) {
    display.print(text[i]);
  }
}

void drawHeader(const char* title) {
  display.setTextSize(1);

  // Safe viewport: khong ve sat bien phai vat ly.
  const int leftX = OLED_SAFE_X;
  const int rightX = OLED_SAFE_RIGHT;

  // Title chi chiem vung trai, tranh tran sang trang thai.
  printClippedText(title, leftX, 0, 68);

  // Cloud indicator: C = ThingsBoard connected, W = WiFi only, L = local/offline.
  display.setCursor(leftX + 66, 0);
  if (mqttClient.connected()) display.print("C");
  else if (WiFi.status() == WL_CONNECTED) display.print("W");
  else display.print("L");

  // Alarm indicator rat ngan.
  display.setCursor(leftX + 76, 0);
  if (alarmIsMuted()) display.print("M");
  else {
    switch (alarmProfile) {
      case ALARM_QUIET:  display.print("I"); break;
      case ALARM_STRONG: display.print("M"); break;
      default:           display.print("V"); break;
    }
  }

  // State canh phai trong safe viewport.
  const char* state = getStateCode();
  int stateX = rightX - textWidth6(state) + 1;
  if (stateX < leftX + 88) stateX = leftX + 88;

  display.setCursor(stateX, 0);
  display.print(state);

  display.drawLine(leftX, 11, rightX, 11, SSD1306_WHITE);
}

int valueToX(int x, int w, float value, float minValue, float maxValue) {
  if (isnan(value)) return x;

  value = clampFloat(value, minValue, maxValue);
  float ratio = (value - minValue) / (maxValue - minValue);
  int px = x + (int)(ratio * w);

  if (px < x) px = x;
  if (px > x + w) px = x + w;

  return px;
}

// Line gauge toi gian:
// - 1 duong ngang = thang min-max co dinh
// - 1 doan dam ben duoi = vung tot
// - 1 vach doc + mui ten = gia tri hien tai
// Khong ve nhieu tick/so tren than chart de tranh roi.
void drawLineGauge(
  int x,
  int y,
  int w,
  float value,
  float minV,
  float maxV,
  float goodMin,
  float goodMax
) {
  int baseY = y;

  display.drawFastHLine(x, baseY, w, SSD1306_WHITE);

  int goodX1 = valueToX(x, w, goodMin, minV, maxV);
  int goodX2 = valueToX(x, w, goodMax, minV, maxV);

  if (goodX2 > goodX1) {
    display.drawFastHLine(goodX1, baseY + 3, goodX2 - goodX1, SSD1306_WHITE);
    display.drawFastHLine(goodX1, baseY + 4, goodX2 - goodX1, SSD1306_WHITE);
  }

  int curX = valueToX(x, w, value, minV, maxV);

  display.drawFastVLine(curX, baseY - 5, 11, SSD1306_WHITE);
  display.fillTriangle(curX, baseY - 8, curX - 3, baseY - 4, curX + 3, baseY - 4, SSD1306_WHITE);
}

void drawFooterScroll() {
  const char* msg = getScrollingMessage();

  display.fillRect(OLED_SAFE_X, 55, OLED_SAFE_W, 9, SSD1306_BLACK);
  display.drawLine(OLED_SAFE_X, 53, OLED_SAFE_RIGHT, 53, SSD1306_WHITE);
  display.setCursor(scrollX, 56);
  display.print(msg);
}

void drawSensorIssuePage() {
  if (comfortState == STATE_SENSOR_ERROR) {
    display.setCursor(0, 18);
    display.print("DHT22 KHONG DOC");

    display.setCursor(0, 32);
    display.print("Kiem tra 3V3 GND");

    display.setCursor(0, 44);
    display.print("DATA GPIO");
    display.print(DHT_PIN);
    return;
  }

  if (comfortState == STATE_SENSOR_SUSPECT) {
    display.setCursor(0, 18);
    display.print("DU LIEU DHT NGHI NGO");

    display.setCursor(0, 32);
    display.print("T:");
    display.print(rawTempC, 1);
    display.print(" H:");
    display.print(rawHumidity, 1);

    display.setCursor(0, 44);
    display.print("Kiem tra sensor");
  }
}

void drawHomePage() {
  drawHeader("TONG QUAN");

  if (!sensorHasValidData) {
    drawSensorIssuePage();
    return;
  }

  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print("FOCUS");

  display.setTextSize(2);
  display.setCursor(0, 27);
  display.print((int)focusScore);

  display.setTextSize(1);
  display.setCursor(47, 31);
  display.print("/100");

  display.setCursor(66, 18);
  printClippedText(getFocusText(), 66, 18, 54);

  display.setCursor(66, 32);
  display.print("HI ");
  display.print(heatIndexC, 0);
  display.print("C");

  display.setCursor(66, 44);
  printClippedText(getShortActionText(), 66, 44, 54);
}

void drawFocusPage() {
  drawHeader("TAP TRUNG");

  if (!sensorHasValidData) {
    drawSensorIssuePage();
    return;
  }

  display.setTextSize(2);
  display.setCursor(0, 15);
  display.print((int)focusScore);

  display.setTextSize(1);
  display.setCursor(45, 19);
  display.print(getFocusText());

  display.setCursor(0, 33);
  display.print("Range 0..100 | Tot >70");

  drawLineGauge(OLED_GAUGE_X, 47, OLED_GAUGE_W, focusScore, FOCUS_SCORE_MIN, FOCUS_SCORE_MAX, FOCUS_SCORE_WARN, FOCUS_SCORE_MAX);
}

void drawHeatPage() {
  drawHeader("CAM NHAN NHIET");

  if (!sensorHasValidData) {
    drawSensorIssuePage();
    return;
  }

  display.setTextSize(2);
  display.setCursor(0, 15);
  display.print(heatIndexC, 1);

  display.setTextSize(1);
  display.setCursor(72, 19);
  display.print("C");

  display.setCursor(0, 33);
  display.print("Range 24..52C");

  display.setCursor(78, 33);
  display.print(getThermalText());

  drawLineGauge(OLED_GAUGE_X, 47, OLED_GAUGE_W, heatIndexC, HEAT_BAR_MIN_C, HEAT_BAR_MAX_C, HEAT_BAR_MIN_C, HEAT_INDEX_WARN);
}

void drawTempPage() {
  drawHeader("NHIET DO");

  if (!sensorHasValidData) {
    drawSensorIssuePage();
    return;
  }

  display.setTextSize(2);
  display.setCursor(0, 15);
  display.print(tempC, 1);

  display.setTextSize(1);
  display.setCursor(72, 19);
  display.print("C");

  display.setCursor(0, 33);
  display.print("Range 18..38C");

  display.setCursor(0, 43);
  display.print("Vung tot 24..27.5");

  drawLineGauge(OLED_GAUGE_X, 51, OLED_GAUGE_W, tempC, TEMP_BAR_MIN_C, TEMP_BAR_MAX_C, TEMP_GOOD_MIN, TEMP_GOOD_MAX);
}

void drawHumPage() {
  drawHeader("DO AM");

  if (!sensorHasValidData) {
    drawSensorIssuePage();
    return;
  }

  display.setTextSize(2);
  display.setCursor(0, 15);
  display.print(humidity, 0);
  display.print("%");

  display.setTextSize(1);
  display.setCursor(0, 33);
  display.print("Range 20..100%");

  display.setCursor(0, 43);
  display.print("Vung tot 40..65%");

  drawLineGauge(OLED_GAUGE_X, 51, OLED_GAUGE_W, humidity, HUM_BAR_MIN_PCT, HUM_BAR_MAX_PCT, HUM_GOOD_MIN, HUM_GOOD_MAX);
}

void drawLightPage() {
  drawHeader("ANH SANG");

#if LIGHT_SENSOR_ENABLED
  display.setTextSize(2);
  display.setCursor(0, 15);
  if (isnan(lightPercent)) display.print("--");
  else display.print(lightPercent, 0);
  display.print("%");

  display.setTextSize(1);
  display.setCursor(74, 19);
  display.print(getLightText());

  display.setCursor(0, 33);
  display.print("Range 0..100%");

  display.setCursor(0, 43);
  display.print("Vung tot 35..78%");

  drawLineGauge(OLED_GAUGE_X, 51, OLED_GAUGE_W, lightPercent, LIGHT_BAR_MIN_PCT, LIGHT_BAR_MAX_PCT, LIGHT_FOCUS_MIN_PCT, LIGHT_FOCUS_MAX_PCT);
#else
  display.setCursor(0, 24);
  display.print("LIGHT SENSOR OFF");
#endif
}

void drawBasisPage() {
  drawHeader("CO SO");

  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print("HP12 = proxy UX");

  display.setCursor(0, 27);
  display.print("T + RH + HI + LDR");

  display.setCursor(0, 39);
  display.print("Chua do gio/buc xa");

  display.setCursor(0, 49);
  display.print("Khong thay the y te");
}

void drawAdvicePage() {
  drawHeader("LOI KHUYEN");

  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print(getAdviceTitle());

  display.setCursor(0, 27);
  display.print("Trang thai: ");
  display.print(getStateText());

  display.setCursor(0, 39);
  display.print("Uu tien: ");
  display.print(getShortActionText());

  // Noi dung dai se nam o dong cuon duoi cung, khong tran dong.
}

void drawHelpPage() {
  drawHeader("HUONG DAN");

  display.setTextSize(1);
  display.setCursor(0, 14);
  display.print("NAV: doi tab du lieu");

  display.setCursor(0, 25);
  display.print("NAV 2: help | 3: im");

  display.setCursor(0, 36);
  display.print("NAV giu: doi coi");

  display.setCursor(0, 47);
  display.print("ADV: loi khuyen");
}

void updateOledNonBlocking() {
  if (!oledReady) return;

  unsigned long now = millis();

  if (now - lastOledUpdateMs < OLED_UPDATE_INTERVAL_MS) return;

  lastOledUpdateMs = now;
  updateScrollPositionNonBlocking();

  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextColor(SSD1306_WHITE);

  switch (currentPage) {
    case PAGE_FOCUS:
      drawFocusPage();
      break;

    case PAGE_HEAT:
      drawHeatPage();
      break;

    case PAGE_TEMP:
      drawTempPage();
      break;

    case PAGE_HUM:
      drawHumPage();
      break;

    case PAGE_LIGHT:
      drawLightPage();
      break;

    case PAGE_BASIS:
      drawBasisPage();
      break;

    case PAGE_ADVICE:
      drawAdvicePage();
      break;

    case PAGE_HELP:
      drawHelpPage();
      break;

    case PAGE_HOME:
    default:
      drawHomePage();
      break;
  }

  drawFooterScroll();
  display.display();
}

// =========================
// BUZZER + LED PATTERNS
// =========================

bool pulsePatternActive(unsigned long now, unsigned long periodMs, unsigned long onMs, unsigned long gapMs, int count) {
  unsigned long phase = now % periodMs;

  for (int i = 0; i < count; i++) {
    unsigned long start = i * (onMs + gapMs);
    if (phase >= start && phase < start + onMs) return true;
  }

  return false;
}

bool shouldBuzzerBeOn(unsigned long now) {
  if (alarmIsMuted()) return false;

  if (comfortState == STATE_WARN && BUZZER_WARN_ENABLED) {
    return pulsePatternActive(now, BUZZER_WARN_PERIOD_MS, BUZZER_WARN_ON_MS, 0, 1);
  }

  if (comfortState != STATE_DANGER) return false;
  if (!sensorHasValidData) return false;

  switch (alarmProfile) {
    case ALARM_QUIET:
      return false;

    case ALARM_STRONG:
      return pulsePatternActive(now, BUZZER_STRONG_PERIOD_MS, BUZZER_STRONG_ON_MS, BUZZER_STRONG_GAP_MS, BUZZER_STRONG_COUNT);

    case ALARM_NORMAL:
    default:
      return pulsePatternActive(now, BUZZER_NORMAL_PERIOD_MS, BUZZER_NORMAL_ON_MS, BUZZER_NORMAL_GAP_MS, BUZZER_NORMAL_COUNT);
  }
}

void updateEnvironmentLedAndBuzzerNonBlocking() {
  unsigned long now = millis();

  if (now - lastLedUpdateMs < LED_UPDATE_INTERVAL_MS) return;

  lastLedUpdateMs = now;

  ledWrite(LED_GREEN_PIN, false);
  ledWrite(LED_YELLOW_PIN, false);
  ledWrite(LED_RED_PIN, false);
  buzzerWrite(false);

  switch (comfortState) {
    case STATE_GOOD:
      ledWrite(LED_GREEN_PIN, true);
      break;

    case STATE_WARN:
      blinkState = ((now / LED_WARN_BLINK_MS) % 2) == 0;
      ledWrite(LED_YELLOW_PIN, blinkState);
      break;

    case STATE_DANGER:
      blinkState = ((now / LED_DANGER_BLINK_MS) % 2) == 0;
      ledWrite(LED_RED_PIN, blinkState);
      buzzerWrite(shouldBuzzerBeOn(now));
      break;

    case STATE_SENSOR_SUSPECT:
    case STATE_SENSOR_ERROR:
      blinkState = ((now / LED_SENSOR_BLINK_MS) % 2) == 0;
      ledWrite(LED_RED_PIN, blinkState);
      break;

    default:
      blinkState = ((now / LED_START_BLINK_MS) % 2) == 0;
      ledWrite(LED_YELLOW_PIN, blinkState);
      break;
  }
}

// Blue LED = system heartbeat, khong phai den moi truong.
// 8 giay dau: self-test nhay nhanh de biet GPIO co dung khong.
// Sau do:
// 1 pulse: OK
// 2 pulses: WARN hoac dang xem ADVICE
// 3 pulses: DANGER
// 4 pulses: loi/nghi ngo cam bien
void updateBlueLedNonBlocking() {
#if ONBOARD_BLUE_LED_ENABLED
  unsigned long now = millis();

  if (now - lastBlueLedUpdateMs < BLUE_LED_UPDATE_INTERVAL_MS) return;

  lastBlueLedUpdateMs = now;

  if (now < blueSelfTestUntilMs) {
    blueLedWrite(((now / 250) % 2) == 0);
    return;
  }

  int pulseCount = 1;

  if (currentPage == PAGE_ADVICE) pulseCount = 2;
  else if (comfortState == STATE_WARN) pulseCount = 2;
  else if (comfortState == STATE_DANGER) pulseCount = 3;
  else if (comfortState == STATE_SENSOR_ERROR || comfortState == STATE_SENSOR_SUSPECT) pulseCount = 4;

  bool on = pulsePatternActive(now, BLUE_LED_ALIVE_PERIOD_MS, BLUE_LED_PULSE_ON_MS, BLUE_LED_PULSE_GAP_MS, pulseCount);
  blueLedWrite(on);
#endif
}


// =========================
// THINGSBOARD / CLOUD BRIDGE
// =========================
//
// Nguyen tac:
// - WiFi/MQTT reconnect bang millis(), khong while chan.
// - Mat mang thi local UX van chay: OLED, DHT22, LDR, LED, button khong bi do.
// - Anti-spam: gui telemetry theo TELEMETRY_INTERVAL_MS.
// - Force first sync: gui ngay 1 goi dau tien khi MQTT ket noi.

const char* resetReasonText(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON:  return "POWERON";
    case ESP_RST_EXT:      return "EXT_RESET";
    case ESP_RST_SW:       return "SW_RESET";
    case ESP_RST_PANIC:    return "PANIC";
    case ESP_RST_INT_WDT:  return "INT_WDT";
    case ESP_RST_TASK_WDT: return "TASK_WDT";
    case ESP_RST_WDT:      return "WDT";
    case ESP_RST_DEEPSLEEP:return "DEEPSLEEP";
    case ESP_RST_BROWNOUT: return "BROWNOUT";
    case ESP_RST_SDIO:     return "SDIO";
    default:               return "UNKNOWN";
  }
}

String jsonEscape(const char* text) {
  String s = String(text);
  s.replace("\\", "\\\\");
  s.replace("\"", "\\\"");
  return s;
}

void jsonComma(String &payload, bool &first) {
  if (!first) payload += ",";
  first = false;
}

void jsonAddString(String &payload, bool &first, const char* key, const char* value) {
  jsonComma(payload, first);
  payload += "\"";
  payload += key;
  payload += "\":\"";
  payload += jsonEscape(value);
  payload += "\"";
}

void jsonAddBool(String &payload, bool &first, const char* key, bool value) {
  jsonComma(payload, first);
  payload += "\"";
  payload += key;
  payload += "\":";
  payload += value ? "true" : "false";
}

void jsonAddInt(String &payload, bool &first, const char* key, long value) {
  jsonComma(payload, first);
  payload += "\"";
  payload += key;
  payload += "\":";
  payload += String(value);
}

void jsonAddFloat(String &payload, bool &first, const char* key, float value, int decimals, bool valid) {
  if (!valid || isnan(value)) return;

  jsonComma(payload, first);
  payload += "\"";
  payload += key;
  payload += "\":";
  payload += String(value, decimals);
}

String buildMqttClientId() {
  String clientId = MQTT_CLIENT_ID_PREFIX;
  clientId += WiFi.macAddress();
  clientId.replace(":", "");
  return clientId;
}

void updateWiFiNonBlocking() {
  if (WiFi.status() == WL_CONNECTED) return;

  unsigned long now = millis();
  if (now - lastWifiRetryMs < WIFI_RETRY_INTERVAL_MS) return;

  lastWifiRetryMs = now;

  Serial.print("[WiFi] Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void sendDeviceAttributes() {
  if (!mqttClient.connected()) return;

  String payload = "{";
  bool first = true;

  jsonAddString(payload, first, "deviceName", DEVICE_NAME);
  jsonAddString(payload, first, "deviceModel", DEVICE_MODEL);
  jsonAddString(payload, first, "firmwareVersion", FIRMWARE_VERSION);
  jsonAddString(payload, first, "ipAddress", WiFi.localIP().toString().c_str());
  jsonAddString(payload, first, "macAddress", WiFi.macAddress().c_str());
  jsonAddString(payload, first, "resetReason", resetReasonText(esp_reset_reason()));
  jsonAddBool(payload, first, "lightSensorEnabled", LIGHT_SENSOR_ENABLED);
  jsonAddInt(payload, first, "dhtPin", DHT_PIN);
  jsonAddInt(payload, first, "lightAoPin", LIGHT_AO_PIN);
  jsonAddInt(payload, first, "oledSafeWidth", OLED_SAFE_W);

  payload += "}";

  bool ok = mqttClient.publish("v1/devices/me/attributes", payload.c_str());
  Serial.print("[TB] Attributes ");
  Serial.println(ok ? "sent." : "failed.");
}

void updateMqttNonBlocking() {
  if (WiFi.status() != WL_CONNECTED) return;

  if (mqttClient.connected()) return;

  unsigned long now = millis();
  if (now - lastMqttRetryMs < MQTT_RETRY_INTERVAL_MS) return;

  lastMqttRetryMs = now;

  String clientId = buildMqttClientId();

  Serial.print("[MQTT] Connecting to ");
  Serial.print(TB_HOST);
  Serial.print(":");
  Serial.println(TB_PORT);

  bool connected = mqttClient.connect(clientId.c_str(), TB_TOKEN, NULL);

  if (connected) {
    Serial.println("[MQTT] Connected to ThingsBoard.");

    cloudEverConnected = true;
    forceFirstTelemetry = FORCE_FIRST_SYNC_ENABLED;
    attributesSentOnce = false;
    lastAttributesMs = 0;

    sendDeviceAttributes();
    attributesSentOnce = true;
    lastAttributesMs = now;
  } else {
    Serial.print("[MQTT] Failed, rc=");
    Serial.println(mqttClient.state());
  }
}

String buildTelemetryPayload() {
  String payload = "{";
  bool first = true;

  jsonAddFloat(payload, first, "temperature", tempC, 2, sensorHasValidData);
  jsonAddFloat(payload, first, "humidity", humidity, 2, sensorHasValidData);
  jsonAddFloat(payload, first, "heatIndex", heatIndexC, 2, sensorHasValidData);

#if LIGHT_SENSOR_ENABLED
  jsonAddFloat(payload, first, "lightPercent", lightPercent, 1, lightReady && !isnan(lightPercent));
  jsonAddInt(payload, first, "lightRaw", lightRaw);
#endif

  jsonAddFloat(payload, first, "focusScore", focusScore, 1, sensorHasValidData);

  jsonAddString(payload, first, "comfortState", getStateCode());
  jsonAddString(payload, first, "comfortText", getStateText());
  jsonAddString(payload, first, "thermalState", getThermalText());
  jsonAddString(payload, first, "lightState", getLightText());
  jsonAddString(payload, first, "action", getShortActionText());
  jsonAddString(payload, first, "alarmProfile", getAlarmProfileText());

  jsonAddBool(payload, first, "alarmMuted", alarmIsMuted());
  jsonAddBool(payload, first, "sensorValid", sensorHasValidData);
  jsonAddBool(payload, first, "lastSensorReadOk", lastSensorReadOk);
  jsonAddBool(payload, first, "mqttConnected", mqttClient.connected());

  jsonAddInt(payload, first, "uptimeSec", millis() / 1000);
  jsonAddInt(payload, first, "wifiRssi", WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : -999);
  jsonAddInt(payload, first, "freeHeap", ESP.getFreeHeap());

  payload += "}";

  return payload;
}

void sendTelemetryNow(const char* reason) {
  if (!mqttClient.connected()) return;

  String payload = buildTelemetryPayload();

  bool ok = mqttClient.publish("v1/devices/me/telemetry", payload.c_str());

  Serial.print("[TB] Telemetry ");
  Serial.print(reason);
  Serial.print(" ");
  Serial.println(ok ? "sent." : "failed.");

  if (!ok) {
    Serial.print("[TB] Payload length=");
    Serial.println(payload.length());
  }
}

void sendTelemetryNonBlocking() {
  if (!mqttClient.connected()) return;

  unsigned long now = millis();

  // Gui lai attributes dinh ky de dashboard co thong tin firmware/IP neu reconnect lau.
  if (!attributesSentOnce || now - lastAttributesMs >= ATTRIBUTES_RESEND_MS) {
    sendDeviceAttributes();
    attributesSentOnce = true;
    lastAttributesMs = now;
  }

  if (forceFirstTelemetry) {
    sendTelemetryNow("first-sync");
    forceFirstTelemetry = false;
    lastTelemetryMs = now;
    return;
  }

  if (now - lastTelemetryMs < TELEMETRY_INTERVAL_MS) return;

  lastTelemetryMs = now;
  sendTelemetryNow("periodic");
}

void updateCloudNonBlocking() {
  updateWiFiNonBlocking();
  updateMqttNonBlocking();

  if (mqttClient.connected()) {
    mqttClient.loop();
  }

  sendTelemetryNonBlocking();
}


// =========================
// SETUP
// =========================

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(BUTTON_NAV_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ADVICE_PIN, INPUT_PULLUP);

#if ONBOARD_BLUE_LED_ENABLED
  pinMode(ONBOARD_BLUE_LED_PIN, OUTPUT);
  blueSelfTestUntilMs = millis() + BLUE_LED_SELF_TEST_MS;
#endif

#if LIGHT_SENSOR_ENABLED
  pinMode(LIGHT_AO_PIN, INPUT);
  pinMode(LIGHT_DO_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(LIGHT_AO_PIN, ADC_11db);
#endif

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  mqttClient.setServer(TB_HOST, TB_PORT);
  mqttClient.setBufferSize(MQTT_BUFFER_SIZE);
  mqttClient.setKeepAlive(30);
  mqttClient.setSocketTimeout(3);

  allOutputsOff();

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  dht.begin();

  oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

  if (!oledReady) {
    Serial.println("[OLED] SSD1306 not found. Check SDA/SCL/VCC/GND/address.");
  } else {
    Serial.println("[OLED] SSD1306 Ready.");
    showBootScreen();
  }

  Serial.println("====================================");
  Serial.print("HP12 ");
  Serial.print(FIRMWARE_VERSION);
  Serial.println(" THINGSBOARD STABLE BRIDGE started.");
  Serial.print("DHT PIN: GPIO");
  Serial.println(DHT_PIN);
  Serial.print("LIGHT AO PIN: GPIO");
  Serial.println(LIGHT_AO_PIN);
  Serial.print("NAV BUTTON: GPIO");
  Serial.println(BUTTON_NAV_PIN);
  Serial.print("ADVICE BUTTON: GPIO");
  Serial.println(BUTTON_ADVICE_PIN);
  Serial.print("BLUE LED: GPIO");
  Serial.println(ONBOARD_BLUE_LED_PIN);
  Serial.println("Blue LED self-test: first 8 seconds.");
  Serial.println("Basis: Temp + RH + Heat Index + relative light; proxy only.");
  Serial.println("Cloud: WiFi + MQTT ThingsBoard non-blocking, anti-spam telemetry.");
  Serial.println("Telemetry interval: 30 seconds + force first sync.");
  Serial.println("====================================");
}

// =========================
// LOOP
// =========================

void loop() {
  updateButtonsNonBlocking();
  readSensorNonBlocking();
  readLightNonBlocking();
  updateOledNonBlocking();
  updateEnvironmentLedAndBuzzerNonBlocking();
  updateBlueLedNonBlocking();
  updateCloudNonBlocking();

  // Khong delay dai.
  // Khong while chan.
}
