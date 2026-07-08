#include "config.h"

#include "secrets.h"

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <esp_system.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================================================
// HP12 v1.12.3 - DeepWork Tropical + WiFi State Machine Fix
// Dong bo voi config.h DeepWork Tropical.
// Muc tieu: giu nguyen logic HP12, sua xung dot WiFi STA/AP khi mang moi, va toi uu LED/coi theo khi hau Viet Nam.
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


#ifndef RPC_ENABLED
#define RPC_ENABLED 1
#endif

#ifndef RPC_RESTART_DELAY_MS
#define RPC_RESTART_DELAY_MS 1200UL
#endif

#ifndef TELEMETRY_INTERVAL_MIN_MS
#define TELEMETRY_INTERVAL_MIN_MS 15000UL
#endif

#ifndef TELEMETRY_INTERVAL_MAX_MS
#define TELEMETRY_INTERVAL_MAX_MS 600000UL
#endif


#ifndef OTA_ENABLED
#define OTA_ENABLED 1
#endif

#ifndef OTA_ALLOW_INSECURE_TLS
#define OTA_ALLOW_INSECURE_TLS 1
#endif

#ifndef OTA_HTTP_TIMEOUT_MS
#define OTA_HTTP_TIMEOUT_MS 20000UL
#endif

#ifndef OTA_RESTART_DELAY_MS
#define OTA_RESTART_DELAY_MS 1200UL
#endif

#ifndef OTA_REQUIRE_HTTPS
#define OTA_REQUIRE_HTTPS 1
#endif


#ifndef OTA_OLED_SUCCESS_HOLD_MS
#define OTA_OLED_SUCCESS_HOLD_MS 2500UL
#endif

#ifndef OTA_OLED_ERROR_HOLD_MS
#define OTA_OLED_ERROR_HOLD_MS 2500UL
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


#ifndef WIFI_SETUP_PORTAL_ENABLED
#define WIFI_SETUP_PORTAL_ENABLED 1
#endif

#ifndef WIFI_CONNECT_TIMEOUT_MS
#define WIFI_CONNECT_TIMEOUT_MS 35000UL
#endif

#ifndef WIFI_SETUP_AP_SSID
#define WIFI_SETUP_AP_SSID "HP12-SETUP"
#endif

#ifndef WIFI_SETUP_AP_PASSWORD
#define WIFI_SETUP_AP_PASSWORD ""
#endif

#ifndef WIFI_SETUP_RESTART_MS
#define WIFI_SETUP_RESTART_MS 2500UL
#endif

#ifndef WIFI_SETUP_TRIGGER_HOLD_MS
#define WIFI_SETUP_TRIGGER_HOLD_MS 5500UL
#endif

#ifndef WIFI_SETUP_DNS_PORT
#define WIFI_SETUP_DNS_PORT 53
#endif

#ifndef WIFI_SETUP_PREF_NAMESPACE
#define WIFI_SETUP_PREF_NAMESPACE "hp12wifi"
#endif

// =========================
// DEEPWORK TROPICAL FALLBACKS
// =========================
// Cac macro nay chi co tac dung khi config.h chua khai bao.
// Neu ban dung config.h v1.12.1 DeepWork Tropical thi cac gia tri trong config.h duoc uu tien.

#ifndef BUZZER_WARN_ENABLED
#define BUZZER_WARN_ENABLED 0
#endif
#ifndef BUZZER_WARN_PERIOD_MS
#define BUZZER_WARN_PERIOD_MS 60000UL
#endif
#ifndef BUZZER_WARN_ON_MS
#define BUZZER_WARN_ON_MS 35UL
#endif
#ifndef BUZZER_NORMAL_PERIOD_MS
#define BUZZER_NORMAL_PERIOD_MS 22000UL
#endif
#ifndef BUZZER_NORMAL_ON_MS
#define BUZZER_NORMAL_ON_MS 55UL
#endif
#ifndef BUZZER_NORMAL_GAP_MS
#define BUZZER_NORMAL_GAP_MS 130UL
#endif
#ifndef BUZZER_NORMAL_COUNT
#define BUZZER_NORMAL_COUNT 2
#endif
#ifndef BUZZER_STRONG_PERIOD_MS
#define BUZZER_STRONG_PERIOD_MS 8000UL
#endif
#ifndef BUZZER_STRONG_ON_MS
#define BUZZER_STRONG_ON_MS 75UL
#endif
#ifndef BUZZER_STRONG_GAP_MS
#define BUZZER_STRONG_GAP_MS 110UL
#endif
#ifndef BUZZER_STRONG_COUNT
#define BUZZER_STRONG_COUNT 3
#endif

#ifndef HEAT_INDEX_MED_CAUTION_C
#define HEAT_INDEX_MED_CAUTION_C 32.0
#endif
#ifndef HEAT_INDEX_MED_DANGER_C
#define HEAT_INDEX_MED_DANGER_C 39.0
#endif
#ifndef HEAT_INDEX_MED_EXTREME_C
#define HEAT_INDEX_MED_EXTREME_C 51.0
#endif

#ifndef FOCUS_SCORE_WARN
#define FOCUS_SCORE_WARN 65.0
#endif
#ifndef FOCUS_SCORE_DANGER
#define FOCUS_SCORE_DANGER 35.0
#endif

// =========================
// WIFI STABILITY FALLBACKS
// =========================
// Muc tieu: mang cu thi tu nho; mang moi/khong thay SSID thi mo HP12-SETUP ro rang.
#ifndef WIFI_CONNECT_FAIL_LIMIT
#define WIFI_CONNECT_FAIL_LIMIT 2
#endif
#ifndef WIFI_SCAN_BEFORE_PORTAL
#define WIFI_SCAN_BEFORE_PORTAL 1
#endif
#ifndef WIFI_ALLOW_HIDDEN_SSID
#define WIFI_ALLOW_HIDDEN_SSID 0
#endif
#ifndef WIFI_STABLE_AFTER_CONNECT_MS
#define WIFI_STABLE_AFTER_CONNECT_MS 2500UL
#endif
#ifndef WIFI_PORTAL_ON_SSID_NOT_FOUND
#define WIFI_PORTAL_ON_SSID_NOT_FOUND 1
#endif
#ifndef WIFI_RADIO_SETTLE_MS
#define WIFI_RADIO_SETTLE_MS 180UL
#endif
#ifndef WIFI_PORTAL_RADIO_RESET_MS
#define WIFI_PORTAL_RADIO_RESET_MS 260UL
#endif
#ifndef WIFI_BEGIN_REISSUE_GUARD_MS
#define WIFI_BEGIN_REISSUE_GUARD_MS 1000UL
#endif

// =========================
// OBJECTS
// =========================

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

Preferences wifiPrefs;
WebServer wifiSetupServer(80);
DNSServer wifiSetupDns;

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
  PAGE_WIFI_SETUP,
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

unsigned long telemetryIntervalMs = TELEMETRY_INTERVAL_MS;
unsigned long scheduledRestartMs = 0;

String lastRpcMethod = "none";
String lastRpcResult = "none";
unsigned long lastRpcAtMs = 0;

bool rpcSubscribed = false;


// =========================
// WIFI PROVISIONING STATE
// =========================
// v1.9.0:
// - WiFi moi: mo portal HP12-SETUP de nguoi dung nhap WiFi bang dien thoai.
// - WiFi cu: luu trong Preferences/NVS, mat dien van nho.
// - Khi khong ket noi duoc WiFi sau timeout, tu mo portal va OLED huong dan.
bool wifiCredentialsLoaded = false;
bool wifiUsingStoredCredentials = false;
bool wifiPortalActive = false;
bool wifiPortalSaveOk = false;
bool wifiEverConnectedThisBoot = false;

String activeWifiSsid = "";
String activeWifiPassword = "";
String wifiPortalReason = "idle";
String wifiPortalMessage = "";
String wifiLastFailure = "none";

unsigned long wifiConnectStartedMs = 0;
unsigned long wifiPortalStartedMs = 0;
unsigned long wifiPortalSavedAtMs = 0;
unsigned long wifiBothButtonHoldStartMs = 0;
bool wifiBothButtonHandled = false;

// Bo dem on dinh WiFi. Chi dung khi WiFi cu khong ket noi duoc.
// Khong can thiep vao MQTT/ThingsBoard, chi quyet dinh luc nao mo HP12-SETUP.
uint8_t wifiConnectFailCount = 0;
int wifiLastScanCount = -1;
int wifiLastRssi = -999;
String wifiLastScanSummary = "not-scanned";
unsigned long wifiConnectedAtMs = 0;


// =========================
// OTA STATE
// =========================
//
// OTA khong chay truc tiep trong callback RPC.
// RPC chi xep lenh, gui phan hoi ve ThingsBoard, sau do loop moi bat dau OTA.
bool otaPending = false;
bool otaInProgress = false;

String otaPendingUrl = "";
String otaTargetVersion = "";
String otaStatus = "idle";
String otaLastError = "";
unsigned long otaRequestedAtMs = 0;

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
  if (wifiPortalActive || currentPage == PAGE_WIFI_SETUP) {
    return "CAI WIFI: Ket noi vao WiFi HP12-SETUP, mo trinh duyet 192.168.4.1, nhap WiFi moi va luu. Sau khi luu HP12 se tu restart va tu nho mang nay.";
  }

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

  // FocusScore = diem UX noi bo cho "deep work", khong phai chan doan y te.
  // Ban tropical khong phat do qua som vi Viet Nam nong am hon chau Au.
  // Diem bi tru theo tung lop: cam nhan nhiet, do am, nhiet phong, anh sang.
  float score = 100.0;

  if (thermalState == THERMAL_DANGER) {
    score -= 45.0;
  } else if (thermalState == THERMAL_WARN) {
    score -= 22.0;
  }

  if (!isnan(heatIndexC)) {
    if (heatIndexC >= HEAT_INDEX_MED_EXTREME_C) score -= 35.0;
    else if (heatIndexC >= HEAT_INDEX_MED_DANGER_C) score -= 22.0;
    else if (heatIndexC >= HEAT_INDEX_MED_CAUTION_C) score -= 10.0;
  }

  if (!isnan(humidity)) {
    if (humidity >= HUM_DANGER) score -= 18.0;
    else if (humidity >= HUM_WARN) score -= 10.0;
    else if (humidity > HUM_GOOD_MAX) score -= 5.0;
  }

  if (!isnan(tempC)) {
    if (tempC >= TEMP_DANGER) score -= 18.0;
    else if (tempC >= TEMP_WARN) score -= 8.0;
    else if (tempC < TEMP_GOOD_MIN) score -= 4.0;
  }

#if LIGHT_SENSOR_ENABLED
  if (lightState == LIGHT_DARK) score -= 18.0;
  if (lightState == LIGHT_GLARE) score -= 18.0;
  if (lightState == LIGHT_BRIGHT) score -= 8.0;
#endif

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

  // RED: chi khi stress nhiet ro hoac FocusScore qua thap.
  // Dieu nay giup LED xanh/vang/do huu dung hon trong khi hau nong am Viet Nam.
  if (thermalState == THERMAL_DANGER || focusScore <= FOCUS_SCORE_DANGER) {
    comfortState = STATE_DANGER;
    return;
  }

#if LIGHT_SENSOR_ENABLED
  if (lightState == LIGHT_DARK || lightState == LIGHT_GLARE || lightState == LIGHT_BRIGHT) {
    comfortState = STATE_WARN;
    return;
  }
#endif

  if (thermalState == THERMAL_WARN || focusScore < FOCUS_SCORE_WARN) {
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

  const int leftX = OLED_SAFE_X;
  const int rightX = OLED_SAFE_RIGHT;

  // Header gon cua v1.9.0:
  // - Trai: ten tab ngan.
  // - Giua/phai: firmware hien hanh.
  // - Cuoi phai: tinh trang cloud 1 ky tu.
  // Khong chen trang thai NONG/CANH vao header nua de tranh de chu.
  printClippedText(title, leftX, 0, 50);

  display.setCursor(58, 0);
  display.print(FIRMWARE_VERSION);

  display.setCursor(rightX - 5, 0);
  if (mqttClient.connected()) display.print("C");
  else if (WiFi.status() == WL_CONNECTED) display.print("W");
  else display.print("L");

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
    display.setCursor(OLED_SAFE_X, 18);
    display.print("DHT22 KHONG DOC");

    display.setCursor(OLED_SAFE_X, 32);
    display.print("Kiem tra 3V3 GND");

    display.setCursor(OLED_SAFE_X, 44);
    display.print("DATA GPIO");
    display.print(DHT_PIN);
    return;
  }

  if (comfortState == STATE_SENSOR_SUSPECT) {
    display.setCursor(OLED_SAFE_X, 18);
    display.print("DU LIEU DHT NGHI NGO");

    display.setCursor(OLED_SAFE_X, 32);
    display.print("T:");
    display.print(rawTempC, 1);
    display.print(" H:");
    display.print(rawHumidity, 1);

    display.setCursor(OLED_SAFE_X, 44);
    display.print("Kiem tra sensor");
  }
}


void drawMiniMetric(int x, int y, const char* label, float value, const char* unit, int decimals) {
  display.setTextSize(1);
  display.setCursor(x, y);
  display.print(label);
  display.print(":");
  if (isnan(value)) {
    display.print("--");
  } else {
    display.print(value, decimals);
  }
  display.print(unit);
}

void drawStatusPill(int x, int y, const char* text) {
  int w = textWidth6(text) + 6;
  if (w > 56) w = 56;
  display.drawRoundRect(x, y, w, 12, 3, SSD1306_WHITE);
  display.setCursor(x + 3, y + 2);
  printClippedText(text, x + 3, y + 2, w - 6);
}

void drawCompactLineGauge(int x, int y, int w, float value, float minV, float maxV, float goodMin, float goodMax) {
  // Gauge cuc gon cho trang tong quan:
  // 1 line ngang, 1 doan vung tot, 1 vach gia tri hien tai.
  display.drawFastHLine(x, y, w, SSD1306_WHITE);

  int goodX1 = valueToX(x, w, goodMin, minV, maxV);
  int goodX2 = valueToX(x, w, goodMax, minV, maxV);
  if (goodX2 > goodX1) {
    display.drawFastHLine(goodX1, y + 2, goodX2 - goodX1, SSD1306_WHITE);
  }

  int curX = valueToX(x, w, value, minV, maxV);
  display.drawFastVLine(curX, y - 4, 9, SSD1306_WHITE);
}

void drawHomePage() {
  drawHeader("HOME");

  if (!sensorHasValidData) {
    drawSensorIssuePage();
    return;
  }

  // v1.9.0 tong quan: khong con nhon nhet.
  // Hang 1: trang thai cam nhan + hanh dong chinh.
  drawStatusPill(OLED_SAFE_X, 14, getStateText());
  display.setCursor(64, 16);
  printClippedText(getShortActionText(), 64, 16, 54);

  // Hang 2: chi so cam nhan nhiet la nhan vat chinh.
  display.setTextSize(2);
  display.setCursor(OLED_SAFE_X, 28);
  display.print(heatIndexC, 0);
  display.setTextSize(1);
  display.setCursor(31, 33);
  display.print("C HI");

  // Ben phai: focus score lon vua du.
  display.setCursor(66, 29);
  display.print("Focus ");
  display.print((int)focusScore);

  // Hang 3: 3 thong so nen, moi thong so 1 cum ngan.
  drawMiniMetric(OLED_SAFE_X, 44, "T", tempC, "C", 1);
  drawMiniMetric(42, 44, "H", humidity, "%", 0);
#if LIGHT_SENSOR_ENABLED
  drawMiniMetric(82, 44, "L", lightPercent, "%", 0);
#else
  display.setCursor(82, 44);
  display.print("L:OFF");
#endif

  // Bottom: gauge ngan cho heat index, khong de so len than chart.
  drawCompactLineGauge(6, 57, 108, heatIndexC, HEAT_BAR_MIN_C, HEAT_BAR_MAX_C, HEAT_BAR_MIN_C, HEAT_INDEX_WARN);
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



void drawWifiSetupPage() {
  drawHeader("CAI WIFI");

  display.setTextSize(1);

  if (wifiPortalSaveOk) {
    display.setCursor(OLED_SAFE_X, 15);
    display.print("DA LUU WIFI");

    display.setCursor(OLED_SAFE_X, 28);
    printClippedText(activeWifiSsid.c_str(), OLED_SAFE_X, 28, OLED_SAFE_W);

    display.setCursor(OLED_SAFE_X, 41);
    display.print("HP12 dang reset...");

    display.drawLine(OLED_SAFE_X, 54, OLED_SAFE_RIGHT, 54, SSD1306_WHITE);
    display.setCursor(OLED_SAFE_X, 56);
    display.print("Lan sau tu ket noi");
    return;
  }

  display.setCursor(OLED_SAFE_X, 14);
  display.print("1 Ket noi dien thoai");

  display.setCursor(OLED_SAFE_X, 25);
  display.print("WiFi: ");
  display.print(WIFI_SETUP_AP_SSID);

  display.setCursor(OLED_SAFE_X, 37);
  display.print("2 Mo: 192.168.4.1");

  display.setCursor(OLED_SAFE_X, 49);
  display.print("Nhap WiFi moi");

  // Portal heartbeat ben phai: cham nhay = AP/DNS/WebServer dang song.
  if (((millis() / 500) % 2) == 0) {
    display.fillCircle(OLED_SAFE_RIGHT - 3, 58, 2, SSD1306_WHITE);
  }
}

bool otaVisualActive() {
  if (otaPending || otaInProgress) return true;
  if (otaStatus == "queued") return true;
  if (otaStatus == "downloading") return true;
  if (otaStatus == "writing") return true;
  if (otaStatus == "success") return true;
  if (otaStatus == "error") return true;
  return false;
}

void drawOtaScreenNow(const char* line1, const char* line2, const char* line3) {
  if (!oledReady) return;

  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(OLED_SAFE_X, 0);
  display.print("OTA");
  display.setCursor(34, 0);
  display.print("Now ");
  display.print(FIRMWARE_VERSION);
  display.drawLine(OLED_SAFE_X, 11, OLED_SAFE_RIGHT, 11, SSD1306_WHITE);

  display.setCursor(OLED_SAFE_X, 16);
  printClippedText(line1, OLED_SAFE_X, 16, OLED_SAFE_W);

  display.setCursor(OLED_SAFE_X, 29);
  printClippedText(line2, OLED_SAFE_X, 29, OLED_SAFE_W);

  display.setCursor(OLED_SAFE_X, 42);
  printClippedText(line3, OLED_SAFE_X, 42, OLED_SAFE_W);

  display.drawLine(OLED_SAFE_X, 53, OLED_SAFE_RIGHT, 53, SSD1306_WHITE);
  display.setCursor(OLED_SAFE_X, 56);
  display.print("KHONG RUT NGUON");
  display.display();
}

void drawOtaPage() {
  // Trang OTA la che do dac biet, khong dung footer scrolling.
  drawHeader("OTA");

  display.setTextSize(1);

  display.setCursor(OLED_SAFE_X, 15);
  display.print("Now : ");
  display.print(FIRMWARE_VERSION);

  display.setCursor(OLED_SAFE_X, 27);
  display.print("Next: ");
  if (otaTargetVersion.length() && otaTargetVersion != "unknown") {
    printClippedText(otaTargetVersion.c_str(), 38, 27, 80);
  } else {
    display.print("latest");
  }

  display.setCursor(OLED_SAFE_X, 39);
  display.print("Run : ");
  if (otaStatus == "queued") display.print("cho lenh");
  else if (otaStatus == "downloading") display.print("dang tai");
  else if (otaStatus == "writing") display.print("dang ghi");
  else if (otaStatus == "success") display.print("sap reset");
  else if (otaStatus == "error") display.print("bi loi");
  else display.print(otaStatus);

  display.drawLine(OLED_SAFE_X, 52, OLED_SAFE_RIGHT, 52, SSD1306_WHITE);
  display.setCursor(OLED_SAFE_X, 55);

  if (otaStatus == "error") {
    printClippedText(otaLastError.c_str(), OLED_SAFE_X, 55, OLED_SAFE_W);
  } else if (otaStatus == "success") {
    display.print("Restart de ap dung");
  } else {
    display.print("Dung tat nguon HP12");
  }
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

  // Khi WiFi setup portal dang mo, uu tien huong dan nguoi dung cai mang moi.
  if (wifiPortalActive) {
    drawWifiSetupPage();
    display.display();
    return;
  }

  // Khi OTA dang dien ra, uu tien man hinh OTA thay vi cac tab thong thuong.
  // Nguoi dung can thay ro: dang update, target version, va se restart.
  if (otaVisualActive()) {
    drawOtaPage();
    display.display();
    return;
  }

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

    case PAGE_WIFI_SETUP:
      drawWifiSetupPage();
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

  // Footer cuon chi dung cho trang co thong diep dai.
  // Cac trang gauge/tong quan khong ve footer de tranh de chu/chart.
  if (currentPage == PAGE_ADVICE || currentPage == PAGE_HELP || currentPage == PAGE_BASIS || currentPage == PAGE_WIFI_SETUP) {
    drawFooterScroll();
  }

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

void onMqttMessage(char* topic, byte* payload, unsigned int length);
void subscribeRpc();
void updateScheduledRestartNonBlocking();
void updateOtaNonBlocking();
bool performOtaUpdate(const String &firmwareUrl);

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


// =========================
// WIFI SETUP PORTAL
// =========================

String htmlEscape(String s) {
  s.replace("&", "&amp;");
  s.replace("<", "&lt;");
  s.replace(">", "&gt;");
  s.replace("\"", "&quot;");
  return s;
}


bool isPlaceholderWifiName(const String &ssid) {
  String s = ssid;
  s.trim();
  s.toUpperCase();
  if (s.length() == 0) return true;
  if (s == "YOUR_FALLBACK_WIFI_NAME") return true;
  if (s == "YOUR_WIFI_SSID") return true;
  if (s == "WIFI_SSID") return true;
  if (s.indexOf("FALLBACK") >= 0) return true;
  if (s.indexOf("PLACEHOLDER") >= 0) return true;
  if (s.indexOf("CHANGE_ME") >= 0) return true;
  return false;
}

bool hasActiveWifiCredentials() {
  return activeWifiSsid.length() > 0 && !isPlaceholderWifiName(activeWifiSsid);
}

void stopStaConnectCleanly(const char *reason) {
  Serial.print("[WiFi] Stop STA cleanly: ");
  Serial.println(reason);
  WiFi.disconnect(false, false);
  delay(WIFI_RADIO_SETTLE_MS);
}

void prepareWifiRadioForSta() {
  // Chỉ dùng trước khi gọi WiFi.begin().
  // Tránh lỗi ESP-IDF: "sta is connecting, cannot set config" khi begin bị gọi chồng.
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  delay(WIFI_RADIO_SETTLE_MS);
}

void prepareWifiRadioForPortalAp() {
  // Khi mở captive portal, phải dừng hẳn STA đang handshake/DHCP.
  // Nếu vẫn để WIFI_AP_STA trong lúc STA đang connecting, ESP32 có thể báo:
  // E wifi:sta is connecting, cannot set config.
  WiFi.disconnect(true, false);
  delay(WIFI_PORTAL_RADIO_RESET_MS);
  WiFi.mode(WIFI_OFF);
  delay(WIFI_PORTAL_RADIO_RESET_MS);
  WiFi.mode(WIFI_AP);
  delay(WIFI_RADIO_SETTLE_MS);
}

bool isSavedSsidVisible(const String &ssid) {
#if WIFI_SCAN_BEFORE_PORTAL
  if (isPlaceholderWifiName(ssid)) return false;

  stopStaConnectCleanly("scan-before-portal");
  prepareWifiRadioForSta();

  Serial.print("[WiFi] Scanning before portal. Target SSID=");
  Serial.println(ssid);

  int n = WiFi.scanNetworks(false, true);
  wifiLastScanCount = n;
  bool found = false;
  int bestRssi = -999;

  if (n > 0) {
    for (int i = 0; i < n; i++) {
      if (WiFi.SSID(i) == ssid) {
        found = true;
        if (WiFi.RSSI(i) > bestRssi) bestRssi = WiFi.RSSI(i);
      }
    }
  }

  WiFi.scanDelete();
  wifiLastRssi = bestRssi;
  wifiLastScanSummary = found ? "ssid-visible" : "ssid-not-found";

  Serial.print("[WiFi] Scan result: ");
  Serial.print(wifiLastScanSummary);
  Serial.print(" | networks=");
  Serial.print(n);
  Serial.print(" | best RSSI=");
  Serial.println(bestRssi);

  return found;
#else
  (void)ssid;
  wifiLastScanSummary = "scan-disabled";
  return true;
#endif
}

void loadWifiCredentials() {
#if WIFI_SETUP_PORTAL_ENABLED
  wifiPrefs.begin(WIFI_SETUP_PREF_NAMESPACE, true);
  String savedSsid = wifiPrefs.getString("ssid", "");
  String savedPass = wifiPrefs.getString("pass", "");
  wifiPrefs.end();

  if (savedSsid.length() > 0 && !isPlaceholderWifiName(savedSsid)) {
    activeWifiSsid = savedSsid;
    activeWifiPassword = savedPass;
    wifiUsingStoredCredentials = true;
    wifiCredentialsLoaded = true;
    Serial.print("[WiFi] Loaded saved WiFi: ");
    Serial.println(activeWifiSsid);
    return;
  }

  if (savedSsid.length() > 0 && isPlaceholderWifiName(savedSsid)) {
    Serial.println("[WiFi] Ignored placeholder saved SSID. Portal will be used.");
  }
#endif

  // Fallback cho nguoi phat trien: neu chua co WiFi trong Preferences,
  // dung secrets.h de thiet bi khong bi mat ket noi ngay sau khi OTA len v1.9.0.
  // Sau khi ket noi thanh cong, firmware se tu luu vao Preferences.
  if (strlen(WIFI_SSID) > 0 && !isPlaceholderWifiName(String(WIFI_SSID))) {
    activeWifiSsid = WIFI_SSID;
    activeWifiPassword = WIFI_PASSWORD;
    wifiUsingStoredCredentials = false;
    wifiCredentialsLoaded = true;
    Serial.print("[WiFi] Using fallback WiFi from secrets.h: ");
    Serial.println(activeWifiSsid);
    return;
  }

  if (strlen(WIFI_SSID) > 0 && isPlaceholderWifiName(String(WIFI_SSID))) {
    Serial.println("[WiFi] Ignored placeholder fallback WiFi in secrets.h.");
  }

  activeWifiSsid = "";
  activeWifiPassword = "";
  wifiUsingStoredCredentials = false;
  wifiCredentialsLoaded = true;
  Serial.println("[WiFi] No WiFi credentials found.");
}

bool saveWifiCredentials(const String &ssid, const String &pass) {
#if WIFI_SETUP_PORTAL_ENABLED
  if (ssid.length() == 0) return false;
  if (isPlaceholderWifiName(ssid)) return false;

  wifiPrefs.begin(WIFI_SETUP_PREF_NAMESPACE, false);
  bool ok1 = wifiPrefs.putString("ssid", ssid) > 0;
  bool ok2 = wifiPrefs.putString("pass", pass) >= 0;
  wifiPrefs.end();

  activeWifiSsid = ssid;
  activeWifiPassword = pass;
  wifiUsingStoredCredentials = true;
  wifiCredentialsLoaded = true;

  Serial.print("[WiFi] Saved WiFi to Preferences: ");
  Serial.println(ssid);
  return ok1 && ok2;
#else
  return false;
#endif
}

void clearWifiCredentials() {
#if WIFI_SETUP_PORTAL_ENABLED
  wifiPrefs.begin(WIFI_SETUP_PREF_NAMESPACE, false);
  wifiPrefs.clear();
  wifiPrefs.end();
#endif

  activeWifiSsid = "";
  activeWifiPassword = "";
  wifiUsingStoredCredentials = false;
  wifiCredentialsLoaded = true;
  wifiConnectStartedMs = 0;
  wifiConnectFailCount = 0;
  wifiLastFailure = "cleared";
  Serial.println("[WiFi] Saved WiFi cleared.");
}

void autoSaveFallbackWifiAfterConnect() {
#if WIFI_SETUP_PORTAL_ENABLED
  if (wifiUsingStoredCredentials) return;
  if (activeWifiSsid.length() == 0) return;

  // Neu ban OTA tu ban cu sang v1.9.0, WiFi trong secrets.h se duoc luu lai 1 lan.
  // Sau do mat dien van nho, va khi doi dia diem se co setup portal.
  saveWifiCredentials(activeWifiSsid, activeWifiPassword);
#endif
}

String wifiSetupPageHtml() {
  String ssidEsc = htmlEscape(activeWifiSsid);
  String reasonEsc = htmlEscape(wifiPortalReason);
  String lastFailEsc = htmlEscape(wifiLastFailure);

  String html;
  html.reserve(5200);
  html += "<!doctype html><html lang='vi'><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>HP12 WiFi Setup</title>";
  html += "<style>";
  html += "body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Arial;background:linear-gradient(135deg,#061a2b,#0b6b5d);color:#fff;min-height:100vh;display:flex;align-items:center;justify-content:center;padding:18px;}";
  html += ".card{width:100%;max-width:430px;background:rgba(255,255,255,.12);backdrop-filter:blur(12px);border:1px solid rgba(255,255,255,.22);border-radius:24px;box-shadow:0 18px 50px rgba(0,0,0,.32);overflow:hidden;}";
  html += ".top{padding:22px 24px 14px;border-bottom:1px solid rgba(255,255,255,.16)}";
  html += "h1{margin:0;font-size:28px;letter-spacing:-.5px}.sub{margin-top:6px;color:#d6fff6;font-size:14px;line-height:1.45}.body{padding:22px 24px}.pill{display:inline-block;border:1px solid rgba(255,255,255,.35);border-radius:999px;padding:7px 11px;margin:0 6px 8px 0;font-size:13px;background:rgba(255,255,255,.1)}";
  html += "label{display:block;margin:16px 0 7px;font-weight:650}input{width:100%;box-sizing:border-box;border:0;border-radius:14px;padding:14px 14px;font-size:16px;outline:none;background:#fff;color:#111}button{width:100%;margin-top:20px;border:0;border-radius:16px;padding:15px 18px;font-size:17px;font-weight:800;color:#08352f;background:#7fffd4;box-shadow:0 8px 20px rgba(127,255,212,.25)}";
  html += ".hint{font-size:13px;line-height:1.55;color:#e8fffa;margin-top:16px}.steps{margin:0;padding-left:18px;color:#e8fffa}.danger{color:#ffe0e0}.ok{color:#b7ffd8}.small{font-size:12px;color:#cdeee8;margin-top:12px}";
  html += "</style></head><body><main class='card'>";
  html += "<section class='top'><h1>HP12 WiFi Setup</h1>";
  html += "<div class='sub'>Thiết lập mạng cho thiết bị cảm nhận tiện nghi không gian làm việc.</div></section>";
  html += "<section class='body'>";

  if (wifiPortalSaveOk) {
    html += "<h2 class='ok'>Đã lưu WiFi</h2>";
    html += "<p>HP12 sẽ tự khởi động lại và kết nối vào mạng mới. Từ lần sau, kể cả mất điện, thiết bị vẫn nhớ WiFi này.</p>";
  } else {
    html += "<div>";
    html += "<span class='pill'>AP: "; html += WIFI_SETUP_AP_SSID; html += "</span>";
    html += "<span class='pill'>IP: 192.168.4.1</span>";
    html += "</div>";
    html += "<p class='hint'>Lý do mở cài đặt: <b>" + reasonEsc + "</b>. Lỗi gần nhất: <b>" + lastFailEsc + "</b>.</p>";
    html += "<form method='POST' action='/save'>";
    html += "<label>Tên WiFi mới</label><input name='ssid' required maxlength='32' placeholder='Ví dụ: My Office WiFi' value='" + ssidEsc + "'>";
    html += "<label>Mật khẩu WiFi</label><input name='pass' type='password' maxlength='64' placeholder='Nhập mật khẩu WiFi'>";
    html += "<button type='submit'>Lưu WiFi và khởi động lại HP12</button></form>";
    html += "<div class='hint'><b>Hướng dẫn nhanh:</b><ol class='steps'><li>Kết nối điện thoại vào WiFi <b>HP12-SETUP</b>.</li><li>Mở trình duyệt vào <b>192.168.4.1</b>.</li><li>Nhập WiFi mới rồi bấm lưu.</li></ol></div>";
    html += "<p class='small'>HP12 chỉ lưu tên WiFi và mật khẩu trong bộ nhớ nội bộ của ESP32, không đưa mật khẩu lên GitHub.</p>";
  }

  html += "</section></main></body></html>";
  return html;
}

void handleWifiSetupRoot() {
  wifiSetupServer.send(200, "text/html; charset=utf-8", wifiSetupPageHtml());
}

void handleWifiSetupSave() {
  String ssid = wifiSetupServer.arg("ssid");
  String pass = wifiSetupServer.arg("pass");
  ssid.trim();

  if (ssid.length() == 0) {
    wifiSetupServer.send(400, "text/plain; charset=utf-8", "SSID không được để trống. Quay lại và nhập tên WiFi.");
    return;
  }

  bool ok = saveWifiCredentials(ssid, pass);
  wifiPortalSaveOk = ok;
  wifiPortalSavedAtMs = millis();
  wifiPortalMessage = ok ? "saved" : "save-failed";

  wifiSetupServer.send(200, "text/html; charset=utf-8", wifiSetupPageHtml());

  if (ok) {
    scheduledRestartMs = millis() + WIFI_SETUP_RESTART_MS;
    Serial.println("[WiFiSetup] WiFi saved. Restart scheduled.");
  }
}

void handleWifiSetupNotFound() {
  wifiSetupServer.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
  wifiSetupServer.send(302, "text/plain", "");
}

void startWifiSetupPortal(const String &reason) {
#if WIFI_SETUP_PORTAL_ENABLED
  if (wifiPortalActive) return;

  wifiPortalReason = reason;
  wifiPortalActive = true;
  wifiPortalSaveOk = false;
  wifiPortalStartedMs = millis();
  wifiPortalMessage = "portal-active";
  currentPage = PAGE_WIFI_SETUP;
  resetScroll();

  mqttClient.disconnect();
  rpcSubscribed = false;

  // Portal chỉ cần AP. Không dùng AP_STA để tránh STA còn đang connecting.
  prepareWifiRadioForPortalAp();

  const char* apPass = WIFI_SETUP_AP_PASSWORD;
  bool apOk;
  if (strlen(apPass) >= 8) apOk = WiFi.softAP(WIFI_SETUP_AP_SSID, apPass);
  else apOk = WiFi.softAP(WIFI_SETUP_AP_SSID);

  IPAddress apIp = WiFi.softAPIP();

  wifiSetupDns.start(WIFI_SETUP_DNS_PORT, "*", apIp);

  wifiSetupServer.on("/", HTTP_GET, handleWifiSetupRoot);
  wifiSetupServer.on("/save", HTTP_POST, handleWifiSetupSave);
  wifiSetupServer.onNotFound(handleWifiSetupNotFound);
  wifiSetupServer.begin();

  Serial.println("[WiFiSetup] Portal started.");
  Serial.print("[WiFiSetup] AP OK: ");
  Serial.println(apOk ? "YES" : "NO");
  Serial.print("[WiFiSetup] SSID: ");
  Serial.println(WIFI_SETUP_AP_SSID);
  Serial.print("[WiFiSetup] Open: http://");
  Serial.println(apIp);
#else
  (void)reason;
#endif
}

void updateWifiSetupPortalNonBlocking() {
#if WIFI_SETUP_PORTAL_ENABLED
  if (!wifiPortalActive) return;

  wifiSetupDns.processNextRequest();
  wifiSetupServer.handleClient();
#endif
}

void updateWifiSetupTriggerNonBlocking() {
#if WIFI_SETUP_PORTAL_ENABLED
  if (wifiPortalActive) return;

  bool bothPressed = isNavButtonPressedRaw() && isAdviceButtonPressedRaw();
  unsigned long now = millis();

  if (bothPressed) {
    if (wifiBothButtonHoldStartMs == 0) {
      wifiBothButtonHoldStartMs = now;
      wifiBothButtonHandled = false;
    }

    if (!wifiBothButtonHandled && now - wifiBothButtonHoldStartMs >= WIFI_SETUP_TRIGGER_HOLD_MS) {
      wifiBothButtonHandled = true;
      navButton.clickCount = 0;
      adviceButton.clickCount = 0;
      clearWifiCredentials();
      startWifiSetupPortal("manual-reset");
    }
  } else {
    wifiBothButtonHoldStartMs = 0;
    wifiBothButtonHandled = false;
  }
#endif
}


String buildMqttClientId() {
  String clientId = MQTT_CLIENT_ID_PREFIX;
  clientId += WiFi.macAddress();
  clientId.replace(":", "");
  return clientId;
}

void updateWiFiNonBlocking() {
  if (wifiPortalActive) return;

  if (!wifiCredentialsLoaded) {
    loadWifiCredentials();
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiEverConnectedThisBoot) {
      wifiEverConnectedThisBoot = true;
      wifiConnectStartedMs = 0;
      wifiConnectFailCount = 0;
      wifiConnectedAtMs = millis();
      wifiLastFailure = "none";
      wifiLastRssi = WiFi.RSSI();
      Serial.print("[WiFi] Connected. SSID=");
      Serial.print(WiFi.SSID());
      Serial.print(" | IP=");
      Serial.print(WiFi.localIP());
      Serial.print(" | RSSI=");
      Serial.println(WiFi.RSSI());
      autoSaveFallbackWifiAfterConnect();
    }
    return;
  }

  wifiEverConnectedThisBoot = false;

  if (!hasActiveWifiCredentials()) {
    wifiLastFailure = "no-credentials";
    startWifiSetupPortal("first-setup");
    return;
  }

  unsigned long now = millis();

  if (wifiConnectStartedMs > 0 && now - wifiConnectStartedMs >= WIFI_CONNECT_TIMEOUT_MS) {
    wifiConnectFailCount++;
    wifiConnectStartedMs = 0;
    wifiLastFailure = "connect-timeout";

    Serial.print("[WiFi] Connect timeout. failCount=");
    Serial.println(wifiConnectFailCount);

#if WIFI_PORTAL_ON_SSID_NOT_FOUND
#if WIFI_ALLOW_HIDDEN_SSID
    bool visible = true;
#else
    bool visible = isSavedSsidVisible(activeWifiSsid);
#endif
    if (!visible) {
      wifiLastFailure = "ssid-not-found";
      Serial.println("[WiFi] Saved SSID not visible. Opening setup portal for new location.");
      startWifiSetupPortal("wifi-not-found");
      return;
    }
#endif

    if (wifiConnectFailCount >= WIFI_CONNECT_FAIL_LIMIT) {
      wifiLastFailure = "connect-failed";
      Serial.println("[WiFi] Fail limit reached. Opening setup portal.");
      startWifiSetupPortal("wifi-login-failed");
      return;
    }
  }

  // Nếu đã phát lệnh WiFi.begin() và vẫn đang trong cửa sổ timeout, KHÔNG gọi begin() lại.
  // Đây là chỗ sửa lỗi chính cho log: E wifi:sta is connecting, cannot set config.
  if (wifiConnectStartedMs > 0) {
    return;
  }

  if (now - lastWifiRetryMs < WIFI_RETRY_INTERVAL_MS) return;

  lastWifiRetryMs = now;
  wifiConnectStartedMs = now;

  Serial.print("[WiFi] Connecting to ");
  Serial.print(activeWifiSsid);
  Serial.print(" | attempt ");
  Serial.println(wifiConnectFailCount + 1);

  stopStaConnectCleanly("before-begin");
  prepareWifiRadioForSta();
  delay(WIFI_BEGIN_REISSUE_GUARD_MS);
  WiFi.begin(activeWifiSsid.c_str(), activeWifiPassword.c_str());
}
void sendDeviceAttributes() {
  if (!mqttClient.connected()) return;

  String payload = "{";
  bool first = true;

  jsonAddString(payload, first, "deviceName", DEVICE_NAME);
  jsonAddString(payload, first, "deviceModel", DEVICE_MODEL);
  jsonAddString(payload, first, "firmwareVersion", FIRMWARE_VERSION);
  jsonAddString(payload, first, "ipAddress", WiFi.localIP().toString().c_str());
  jsonAddString(payload, first, "wifiSsid", activeWifiSsid.c_str());
  jsonAddString(payload, first, "wifiLastFailure", wifiLastFailure.c_str());
  jsonAddString(payload, first, "wifiLastScan", wifiLastScanSummary.c_str());
  jsonAddString(payload, first, "macAddress", WiFi.macAddress().c_str());
  jsonAddString(payload, first, "resetReason", resetReasonText(esp_reset_reason()));
  jsonAddBool(payload, first, "lightSensorEnabled", LIGHT_SENSOR_ENABLED);
  jsonAddInt(payload, first, "dhtPin", DHT_PIN);
  jsonAddInt(payload, first, "lightAoPin", LIGHT_AO_PIN);
  jsonAddInt(payload, first, "oledSafeWidth", OLED_SAFE_W);
  jsonAddBool(payload, first, "rpcEnabled", RPC_ENABLED);
  jsonAddString(payload, first, "rpcMethods", "getStatus,getTelemetry,muteAlarm,setAlarmProfile,setTelemetryInterval,restart,updateFirmware");
  jsonAddBool(payload, first, "otaEnabled", OTA_ENABLED);
  jsonAddString(payload, first, "otaMode", "rpc-updateFirmware-https-url");
  jsonAddBool(payload, first, "wifiSetupPortalEnabled", WIFI_SETUP_PORTAL_ENABLED);
  jsonAddBool(payload, first, "wifiUsingStoredCredentials", wifiUsingStoredCredentials);

  payload += "}";

  bool ok = mqttClient.publish("v1/devices/me/attributes", payload.c_str());
  Serial.print("[TB] Attributes ");
  Serial.println(ok ? "sent." : "failed.");
}

void updateMqttNonBlocking() {
  if (WiFi.status() != WL_CONNECTED) {
    rpcSubscribed = false;
    return;
  }

  if (mqttClient.connected()) return;

  // Neu MQTT da rot ket noi, danh dau RPC chua san sang.
  rpcSubscribed = false;

  unsigned long now = millis();
  if (now - lastMqttRetryMs < MQTT_RETRY_INTERVAL_MS) return;

  lastMqttRetryMs = now;

  String clientId = buildMqttClientId();

  Serial.print("[MQTT] Connecting to ");
  Serial.print(TB_HOST);
  Serial.print(":");
  Serial.print(TB_PORT);
  Serial.print(" | RSSI=");
  Serial.println(WiFi.RSSI());

  bool connected = mqttClient.connect(clientId.c_str(), TB_TOKEN, NULL);

  if (connected) {
    Serial.println("[MQTT] Connected to ThingsBoard.");

    cloudEverConnected = true;
    forceFirstTelemetry = FORCE_FIRST_SYNC_ENABLED;
    attributesSentOnce = false;
    lastAttributesMs = 0;

    // v1.7.1 FIX:
    // v1.7.0 thieu lenh subscribeRpc() sau khi reconnect,
    // nen device online nhung khong nhan duoc RPC.
    subscribeRpc();

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
  jsonAddBool(payload, first, "rpcReady", mqttClient.connected() && rpcSubscribed);
  jsonAddString(payload, first, "lastRpcMethod", lastRpcMethod.c_str());
  jsonAddString(payload, first, "lastRpcResult", lastRpcResult.c_str());
  jsonAddInt(payload, first, "lastRpcAtSec", lastRpcAtMs / 1000);
  jsonAddInt(payload, first, "telemetryIntervalMs", telemetryIntervalMs);

  jsonAddString(payload, first, "otaStatus", otaStatus.c_str());
  jsonAddString(payload, first, "otaTargetVersion", otaTargetVersion.c_str());
  jsonAddString(payload, first, "otaLastError", otaLastError.c_str());
  jsonAddBool(payload, first, "otaPending", otaPending);
  jsonAddBool(payload, first, "otaInProgress", otaInProgress);

  jsonAddBool(payload, first, "mqttConnected", mqttClient.connected());

  jsonAddInt(payload, first, "uptimeSec", millis() / 1000);
  jsonAddInt(payload, first, "wifiRssi", WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : -999);
  jsonAddBool(payload, first, "wifiPortalActive", wifiPortalActive);
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


// =========================
// RPC CONTROL
// =========================
//
// ThingsBoard RPC topic:
//   request : v1/devices/me/rpc/request/+
//   response: v1/devices/me/rpc/response/<requestId>
//
// Parser duoc viet toi gian de khong can them ArduinoJson.
// Cac RPC method ho tro o v1.7.0:
// - getStatus
// - getTelemetry
// - muteAlarm
// - setAlarmProfile
// - setTelemetryInterval
// - restart
// - updateFirmware: chi tra loi "OTA chua bat", OTA bridge da co tu v1.8.x

void markRpcResult(const char* method, const char* result) {
  lastRpcMethod = method;
  lastRpcResult = result;
  lastRpcAtMs = millis();
}

String extractRpcRequestId(const String &topic) {
  int slash = topic.lastIndexOf('/');
  if (slash < 0 || slash >= (int)topic.length() - 1) return "0";
  return topic.substring(slash + 1);
}

String extractJsonStringValue(const String &json, const char* key) {
  String pattern = "\"";
  pattern += key;
  pattern += "\"";

  int keyPos = json.indexOf(pattern);
  if (keyPos < 0) return "";

  int colonPos = json.indexOf(':', keyPos);
  if (colonPos < 0) return "";

  int quote1 = json.indexOf('"', colonPos + 1);
  if (quote1 < 0) return "";

  int quote2 = json.indexOf('"', quote1 + 1);
  if (quote2 < 0) return "";

  return json.substring(quote1 + 1, quote2);
}

String extractRpcParamsRaw(const String &json) {
  int keyPos = json.indexOf("\"params\"");
  if (keyPos < 0) return "";

  int colonPos = json.indexOf(':', keyPos);
  if (colonPos < 0) return "";

  String params = json.substring(colonPos + 1);
  params.trim();

  if (params.endsWith("}")) {
    params.remove(params.length() - 1);
    params.trim();
  }

  return params;
}

long extractFirstNumber(const String &text, long fallbackValue) {
  String digits = "";
  bool started = false;

  for (int i = 0; i < (int)text.length(); i++) {
    char c = text.charAt(i);

    if ((c >= '0' && c <= '9') || (c == '-' && !started)) {
      digits += c;
      started = true;
    } else if (started) {
      break;
    }
  }

  if (digits.length() == 0) return fallbackValue;
  return digits.toInt();
}

void sendRpcResponse(const String &requestId, const String &responsePayload) {
  if (!mqttClient.connected()) return;

  String responseTopic = "v1/devices/me/rpc/response/";
  responseTopic += requestId;

  bool ok = mqttClient.publish(responseTopic.c_str(), responsePayload.c_str());

  Serial.print("[RPC] Response ");
  Serial.print(requestId);
  Serial.print(" ");
  Serial.println(ok ? "sent." : "failed.");
}

String buildStatusPayload() {
  String payload = "{";
  bool first = true;

  jsonAddBool(payload, first, "success", true);
  jsonAddString(payload, first, "deviceName", DEVICE_NAME);
  jsonAddString(payload, first, "firmwareVersion", FIRMWARE_VERSION);
  jsonAddString(payload, first, "state", getStateCode());
  jsonAddString(payload, first, "stateText", getStateText());
  jsonAddString(payload, first, "action", getShortActionText());

  jsonAddFloat(payload, first, "temperature", tempC, 2, sensorHasValidData);
  jsonAddFloat(payload, first, "humidity", humidity, 2, sensorHasValidData);
  jsonAddFloat(payload, first, "heatIndex", heatIndexC, 2, sensorHasValidData);
#if LIGHT_SENSOR_ENABLED
  jsonAddFloat(payload, first, "lightPercent", lightPercent, 1, lightReady && !isnan(lightPercent));
#endif
  jsonAddFloat(payload, first, "focusScore", focusScore, 1, sensorHasValidData);

  jsonAddBool(payload, first, "alarmMuted", alarmIsMuted());
  jsonAddString(payload, first, "alarmProfile", getAlarmProfileText());
  jsonAddInt(payload, first, "telemetryIntervalMs", telemetryIntervalMs);
  jsonAddString(payload, first, "otaStatus", otaStatus.c_str());
  jsonAddString(payload, first, "otaTargetVersion", otaTargetVersion.c_str());
  jsonAddString(payload, first, "otaLastError", otaLastError.c_str());
  jsonAddBool(payload, first, "otaPending", otaPending);
  jsonAddBool(payload, first, "otaInProgress", otaInProgress);
  jsonAddInt(payload, first, "uptimeSec", millis() / 1000);
  jsonAddInt(payload, first, "wifiRssi", WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : -999);
  jsonAddInt(payload, first, "freeHeap", ESP.getFreeHeap());

  payload += "}";
  return payload;
}

void handleRpcGetStatus(const String &requestId) {
  markRpcResult("getStatus", "ok");
  sendRpcResponse(requestId, buildStatusPayload());
}

void handleRpcGetTelemetry(const String &requestId) {
  sendTelemetryNow("rpc-getTelemetry");
  markRpcResult("getTelemetry", "sent");
  sendRpcResponse(requestId, "{\"success\":true,\"message\":\"telemetry sent\"}");
}

void handleRpcRestart(const String &requestId) {
  markRpcResult("restart", "scheduled");
  sendRpcResponse(requestId, "{\"success\":true,\"message\":\"restart scheduled\"}");
  scheduledRestartMs = millis() + RPC_RESTART_DELAY_MS;
}

void handleRpcMuteAlarm(const String &requestId, const String &paramsRaw) {
  String p = paramsRaw;
  p.toLowerCase();

  if (p.indexOf("false") >= 0 || p.indexOf("unmute") >= 0 || p.indexOf("off") >= 0 || p == "0") {
    alarmMutedUntilMs = 0;
    markRpcResult("muteAlarm", "unmuted");
    sendRpcResponse(requestId, "{\"success\":true,\"alarmMuted\":false}");
    return;
  }

  if (p.indexOf("true") >= 0 || p.indexOf("mute") >= 0 || p.indexOf("on") >= 0 || p == "1") {
    alarmMutedUntilMs = millis() + ALARM_MUTE_MS;
    markRpcResult("muteAlarm", "muted");
    sendRpcResponse(requestId, "{\"success\":true,\"alarmMuted\":true}");
    return;
  }

  // Neu params rong/khong ro: toggle.
  if (alarmIsMuted()) {
    alarmMutedUntilMs = 0;
    markRpcResult("muteAlarm", "toggle-unmuted");
    sendRpcResponse(requestId, "{\"success\":true,\"alarmMuted\":false}");
  } else {
    alarmMutedUntilMs = millis() + ALARM_MUTE_MS;
    markRpcResult("muteAlarm", "toggle-muted");
    sendRpcResponse(requestId, "{\"success\":true,\"alarmMuted\":true}");
  }
}

void handleRpcSetAlarmProfile(const String &requestId, const String &paramsRaw) {
  String p = paramsRaw;
  p.toUpperCase();

  bool ok = true;

  if (p.indexOf("QUIET") >= 0 || p.indexOf("IM") >= 0 || p == "0") {
    alarmProfile = ALARM_QUIET;
  } else if (p.indexOf("NORMAL") >= 0 || p.indexOf("VUA") >= 0 || p == "1") {
    alarmProfile = ALARM_NORMAL;
  } else if (p.indexOf("STRONG") >= 0 || p.indexOf("MANH") >= 0 || p == "2") {
    alarmProfile = ALARM_STRONG;
  } else {
    ok = false;
  }

  if (ok) {
    markRpcResult("setAlarmProfile", getAlarmProfileText());

    String response = "{\"success\":true,\"alarmProfile\":\"";
    response += getAlarmProfileText();
    response += "\"}";
    sendRpcResponse(requestId, response);
  } else {
    markRpcResult("setAlarmProfile", "invalid");
    sendRpcResponse(requestId, "{\"success\":false,\"message\":\"invalid alarm profile. Use 0/1/2 or IM/VUA/MANH\"}");
  }
}

void handleRpcSetTelemetryInterval(const String &requestId, const String &paramsRaw) {
  long requested = extractFirstNumber(paramsRaw, TELEMETRY_INTERVAL_MS);

  if (requested < (long)TELEMETRY_INTERVAL_MIN_MS) requested = TELEMETRY_INTERVAL_MIN_MS;
  if (requested > (long)TELEMETRY_INTERVAL_MAX_MS) requested = TELEMETRY_INTERVAL_MAX_MS;

  telemetryIntervalMs = (unsigned long)requested;

  markRpcResult("setTelemetryInterval", "ok");

  String response = "{\"success\":true,\"telemetryIntervalMs\":";
  response += String(telemetryIntervalMs);
  response += "}";
  sendRpcResponse(requestId, response);
}

String normalizeOtaUrl(const String &paramsRaw) {
  String url = extractJsonStringValue(paramsRaw, "url");

  if (url.length() == 0) {
    url = paramsRaw;
    url.trim();

    if (url.startsWith("\"") && url.endsWith("\"") && url.length() > 1) {
      url = url.substring(1, url.length() - 1);
    }
  }

  url.trim();
  return url;
}

String normalizeOtaVersion(const String &paramsRaw) {
  String version = extractJsonStringValue(paramsRaw, "version");
  version.trim();
  return version;
}

bool isOtaUrlAllowed(const String &url) {
#if OTA_REQUIRE_HTTPS
  if (!url.startsWith("https://")) return false;
#else
  if (!url.startsWith("http://") && !url.startsWith("https://")) return false;
#endif

  return true;
}

void setOtaError(const String &errorText) {
  otaStatus = "error";
  otaLastError = errorText;
  otaPending = false;
  otaInProgress = false;

  String result = "error:";
  result += errorText;
  markRpcResult("updateFirmware", result.c_str());

  Serial.print("[OTA] ERROR: ");
  Serial.println(errorText);

  drawOtaScreenNow("OTA bi loi", errorText.c_str(), "Kiem tra URL .bin");
  sendTelemetryNow("ota-error");
  delay(OTA_OLED_ERROR_HOLD_MS);
}

void handleRpcUpdateFirmware(const String &requestId, const String &paramsRaw) {
#if OTA_ENABLED
  if (otaPending || otaInProgress) {
    sendRpcResponse(requestId, "{\"success\":false,\"message\":\"OTA already pending or in progress\"}");
    return;
  }

  String url = normalizeOtaUrl(paramsRaw);
  String version = normalizeOtaVersion(paramsRaw);

  if (url.length() == 0 || !isOtaUrlAllowed(url)) {
    markRpcResult("updateFirmware", "bad-url");
    sendRpcResponse(requestId, "{\"success\":false,\"message\":\"missing or invalid firmware url. Use HTTPS url\"}");
    return;
  }

  if (version.length() && version == FIRMWARE_VERSION) {
    markRpcResult("updateFirmware", "same-version");
    sendRpcResponse(requestId, "{\"success\":false,\"message\":\"device already runs this firmware version\"}");
    return;
  }

  otaPendingUrl = url;
  otaTargetVersion = version.length() ? version : "unknown";
  otaStatus = "queued";
  otaLastError = "";
  otaRequestedAtMs = millis();
  otaPending = true;
  otaInProgress = false;

  markRpcResult("updateFirmware", "queued");

  String response = "{\"success\":true,\"message\":\"OTA queued\",\"targetVersion\":\"";
  response += jsonEscape(otaTargetVersion.c_str());
  response += "\"}";
  sendRpcResponse(requestId, response);

  sendTelemetryNow("ota-queued");
#else
  markRpcResult("updateFirmware", "ota-disabled");
  sendRpcResponse(requestId, "{\"success\":false,\"message\":\"OTA disabled in config\"}");
#endif
}


void handleRpcRequest(const String &requestId, const String &method, const String &paramsRaw) {
  Serial.print("[RPC] Method=");
  Serial.print(method);
  Serial.print(" Params=");
  Serial.println(paramsRaw);

  if (method == "getStatus") {
    handleRpcGetStatus(requestId);
  } else if (method == "getTelemetry") {
    handleRpcGetTelemetry(requestId);
  } else if (method == "restart") {
    handleRpcRestart(requestId);
  } else if (method == "muteAlarm") {
    handleRpcMuteAlarm(requestId, paramsRaw);
  } else if (method == "setAlarmProfile") {
    handleRpcSetAlarmProfile(requestId, paramsRaw);
  } else if (method == "setTelemetryInterval") {
    handleRpcSetTelemetryInterval(requestId, paramsRaw);
  } else if (method == "updateFirmware") {
    handleRpcUpdateFirmware(requestId, paramsRaw);
  } else {
    markRpcResult(method.c_str(), "unknown-method");

    String response = "{\"success\":false,\"message\":\"unknown RPC method: ";
    response += jsonEscape(method.c_str());
    response += "\"}";
    sendRpcResponse(requestId, response);
  }
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
#if RPC_ENABLED
  String topicStr = String(topic);
  String requestId = extractRpcRequestId(topicStr);

  String body = "";
  for (unsigned int i = 0; i < length; i++) {
    body += (char)payload[i];
  }

  String method = extractJsonStringValue(body, "method");
  String paramsRaw = extractRpcParamsRaw(body);

  if (method.length() == 0) {
    markRpcResult("unknown", "bad-payload");
    sendRpcResponse(requestId, "{\"success\":false,\"message\":\"missing method\"}");
    return;
  }

  handleRpcRequest(requestId, method, paramsRaw);
#else
  (void)topic;
  (void)payload;
  (void)length;
#endif
}

void subscribeRpc() {
#if RPC_ENABLED
  if (!mqttClient.connected()) {
    rpcSubscribed = false;
    return;
  }

  bool ok = mqttClient.subscribe("v1/devices/me/rpc/request/+");
  rpcSubscribed = ok;

  Serial.print("[RPC] Subscribe ");
  Serial.println(ok ? "ok." : "failed.");
#endif
}

void updateScheduledRestartNonBlocking() {
  if (scheduledRestartMs == 0) return;

  if (millis() >= scheduledRestartMs) {
    Serial.println("[SYS] Restarting by RPC...");
    delay(50);
    ESP.restart();
  }
}

bool performOtaUpdate(const String &firmwareUrl) {
#if OTA_ENABLED
  if (WiFi.status() != WL_CONNECTED) {
    setOtaError("wifi-not-connected");
    return false;
  }

  otaStatus = "downloading";
  otaInProgress = true;
  otaPending = false;
  otaLastError = "";

  drawOtaScreenNow("Dang tai firmware", otaTargetVersion.c_str(), "Trang thai: download");

  Serial.println("[OTA] Starting firmware update...");
  Serial.print("[OTA] URL: ");
  Serial.println(firmwareUrl);

  sendTelemetryNow("ota-start");

  WiFiClientSecure secureClient;
#if OTA_ALLOW_INSECURE_TLS
  // MVP mode: khong verify certificate de tranh loi chain/cert tren ESP32.
  // Ban san xuat nen dung certificate pinning/CA root.
  secureClient.setInsecure();
#endif

  HTTPClient http;
  http.setTimeout(OTA_HTTP_TIMEOUT_MS);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  if (!http.begin(secureClient, firmwareUrl)) {
    setOtaError("http-begin-failed");
    return false;
  }

  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    String err = "http-code-";
    err += String(httpCode);
    http.end();
    setOtaError(err);
    return false;
  }

  int contentLength = http.getSize();

  Serial.print("[OTA] Content-Length: ");
  Serial.println(contentLength);

  otaStatus = "writing";
  drawOtaScreenNow("Dang ghi firmware", otaTargetVersion.c_str(), "Trang thai: write");

  if (!Update.begin(contentLength > 0 ? contentLength : UPDATE_SIZE_UNKNOWN)) {
    String err = "update-begin-failed:";
    err += Update.errorString();
    http.end();
    setOtaError(err);
    return false;
  }

  WiFiClient *stream = http.getStreamPtr();
  size_t written = Update.writeStream(*stream);

  Serial.print("[OTA] Written bytes: ");
  Serial.println(written);

  if (contentLength > 0 && written != (size_t)contentLength) {
    String err = "size-mismatch:";
    err += String(written);
    err += "/";
    err += String(contentLength);
    Update.abort();
    http.end();
    setOtaError(err);
    return false;
  }

  if (!Update.end(true)) {
    String err = "update-end-failed:";
    err += Update.errorString();
    http.end();
    setOtaError(err);
    return false;
  }

  if (!Update.isFinished()) {
    http.end();
    setOtaError("update-not-finished");
    return false;
  }

  http.end();

  otaStatus = "success";
  otaInProgress = false;
  otaPending = false;
  otaLastError = "";

  markRpcResult("updateFirmware", "success");

  Serial.println("[OTA] Update success. Restarting...");
  drawOtaScreenNow("Cap nhat thanh cong", otaTargetVersion.c_str(), "Dang reset HP12...");
  sendTelemetryNow("ota-success");

  delay(OTA_OLED_SUCCESS_HOLD_MS);
  ESP.restart();

  return true;
#else
  setOtaError("ota-disabled");
  return false;
#endif
}

void updateOtaNonBlocking() {
  if (!otaPending || otaInProgress) return;

  performOtaUpdate(otaPendingUrl);
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

  if (now - lastTelemetryMs < telemetryIntervalMs) return;

  lastTelemetryMs = now;
  sendTelemetryNow("periodic");
}

void updateCloudNonBlocking() {
  updateWifiSetupPortalNonBlocking();

  // Khi dang o portal cai WiFi, khong thu MQTT de tranh gay roi va tranh spam.
  if (wifiPortalActive) return;

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

  WiFi.persistent(false);
  prepareWifiRadioForSta();
  loadWifiCredentials();

  mqttClient.setServer(TB_HOST, TB_PORT);
  mqttClient.setCallback(onMqttMessage);
  mqttClient.setBufferSize(MQTT_BUFFER_SIZE);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(6);

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

#if WIFI_SETUP_PORTAL_ENABLED
  // Neu khong co WiFi hop le thi mo portal ngay trong setup,
  // khong de nguoi dung nhin thay dashboard binh thuong roi bo qua buoc cai WiFi.
  if (!hasActiveWifiCredentials()) {
    wifiLastFailure = "no-credentials";
    startWifiSetupPortal("first-setup");
  }
#endif

  Serial.println("====================================");
  Serial.print("HP12 ");
  Serial.print(FIRMWARE_VERSION);
  Serial.println(" DEEPWORK TROPICAL WIFI STABLE started.");
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
  Serial.println("WiFi setup: old WiFi remembered, new location opens HP12-SETUP with OLED guidance.");
  Serial.println("Hold NAV + ADVICE for 5.5s to reset WiFi and open setup portal.");
  Serial.println("Telemetry interval: 30 seconds + force first sync.");
  Serial.println("====================================");
}

// =========================
// LOOP
// =========================

void loop() {
  updateWifiSetupTriggerNonBlocking();
  if (!wifiPortalActive) {
    updateButtonsNonBlocking();
  }
  readSensorNonBlocking();
  readLightNonBlocking();
  updateOledNonBlocking();
  updateEnvironmentLedAndBuzzerNonBlocking();
  updateBlueLedNonBlocking();
  updateCloudNonBlocking();
  updateOtaNonBlocking();
  updateScheduledRestartNonBlocking();

  // Khong delay dai.
  // Khong while chan.
}
