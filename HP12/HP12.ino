/*
  =============================================================================
  PCODE HP12 - DATA INTELLIGENCE EDITION
  Firmware: v1.13.19 WiFi Identity + IP Location Clarity
  Board   : ESP32 30 pin / ESP32-WROOM compatible
  Sensor  : DHT22 + LDR LM393
  Display : OLED SSD1306 128x64 I2C
  Cloud   : ThingsBoard MQTT + RPC + OTA

  NGUYÊN TẮC NÂNG CẤP BẢN NÀY
  - Giữ luồng vận hành cốt lõi: đọc cảm biến không chặn, OLED không chặn,
    LED/còi không chặn, WiFi/MQTT/Telemetry không spam, OTA qua RPC.
  - Đồng bộ toàn bộ ngưỡng/nhịp điệu theo config.h.
  - Sửa lỗi WiFi quan trọng khi chuyển địa điểm: trước khi mở HP12-SETUP Portal
    phải dừng STA sạch sẽ, tránh lỗi ESP32: "sta is connecting, cannot set config".
  - Không hard-code mật khẩu WiFi hoặc ThingsBoard token trong file này.
    Có thể tạo file secrets.h cùng thư mục nếu cần.

  secrets.h gợi ý:
    #pragma once
    #define FALLBACK_WIFI_SSID     "Ten_WiFi_Du_Phong"
    #define FALLBACK_WIFI_PASSWORD "Mat_Khau"
    #define TB_HOST                "thingsboard.cloud"
    #define TB_PORT                1883
    #define TB_TOKEN               "YOUR_DEVICE_ACCESS_TOKEN"
    // v1.13.19 adds OLED WiFi identity and clearer IP-location status; no factory token is embedded.
  =============================================================================
*/

#include "config.h"

#if __has_include("secrets.h")
  #include "secrets.h"
#endif

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

// ============================================================================
// SAFE DEFAULTS - chỉ dùng khi config.h/secrets.h chưa khai báo
// ============================================================================
#ifndef BUZZER_ACTIVE_LOW
  #define BUZZER_ACTIVE_LOW 0
#endif
#ifndef LED_ACTIVE_LOW
  #define LED_ACTIVE_LOW 0
#endif
#ifndef ONBOARD_BLUE_LED_ENABLED
  #define ONBOARD_BLUE_LED_ENABLED 0
#endif
#ifndef ONBOARD_BLUE_LED_PIN
  #define ONBOARD_BLUE_LED_PIN 2
#endif
#ifndef ONBOARD_BLUE_LED_ACTIVE_LOW
  #define ONBOARD_BLUE_LED_ACTIVE_LOW 0
#endif
#ifndef WIFI_STA_STOP_SETTLE_MS
  #define WIFI_STA_STOP_SETTLE_MS 350UL
#endif
#ifndef WIFI_SET_SLEEP_DISABLE
  #define WIFI_SET_SLEEP_DISABLE 1
#endif
#ifndef WIFI_SET_TX_POWER_MAX
  #define WIFI_SET_TX_POWER_MAX 0
#endif
#ifndef WIFI_COUNTRY_VN_ENABLED
  #define WIFI_COUNTRY_VN_ENABLED 1
#endif
#ifndef WIFI_SCAN_BEFORE_CONNECT
  #define WIFI_SCAN_BEFORE_CONNECT 1
#endif
#ifndef WIFI_FAST_PORTAL_IF_SSID_NOT_FOUND
  #define WIFI_FAST_PORTAL_IF_SSID_NOT_FOUND 1
#endif
#ifndef WIFI_SCAN_MIN_RSSI_DBM
  #define WIFI_SCAN_MIN_RSSI_DBM -88
#endif
#ifndef WIFI_DHCP_GRACE_MS
  #define WIFI_DHCP_GRACE_MS 12000UL
#endif
#ifndef WIFI_REJOIN_CLEAN_DISCONNECT_MS
  #define WIFI_REJOIN_CLEAN_DISCONNECT_MS 250UL
#endif
#ifndef WIFI_PORTAL_AFTER_BOOT_TIMEOUTS
  #define WIFI_PORTAL_AFTER_BOOT_TIMEOUTS 1
#endif
#ifndef WIFI_PORTAL_AFTER_DROP_TIMEOUTS
  #define WIFI_PORTAL_AFTER_DROP_TIMEOUTS 3
#endif
#ifndef WIFI_SETUP_FORCE_AP_ONLY
  #define WIFI_SETUP_FORCE_AP_ONLY 1
#endif
#ifndef WIFI_SETUP_AP_CHANNEL
  #define WIFI_SETUP_AP_CHANNEL 6
#endif
#ifndef WIFI_SETUP_AP_MAX_CONNECTIONS
  #define WIFI_SETUP_AP_MAX_CONNECTIONS 4
#endif
#ifndef WIFI_PORTAL_SCAN_LIST_ENABLED
  #define WIFI_PORTAL_SCAN_LIST_ENABLED 1
#endif
#ifndef WIFI_PORTAL_SCAN_MAX_ITEMS
  #define WIFI_PORTAL_SCAN_MAX_ITEMS 12
#endif
#ifndef OTA_SAFE_WIFI_BOOT_ENABLED
  #define OTA_SAFE_WIFI_BOOT_ENABLED 1
#endif
#ifndef OTA_SAFE_WIFI_GRACE_MS
  #define OTA_SAFE_WIFI_GRACE_MS 180000UL
#endif
#ifndef WIFI_AUTO_PORTAL_ON_WEAK_RSSI_AFTER_OTA
  #define WIFI_AUTO_PORTAL_ON_WEAK_RSSI_AFTER_OTA 0
#endif
#ifndef WIFI_PORTAL_USE_GET_SAVE
  #define WIFI_PORTAL_USE_GET_SAVE 1
#endif
#ifndef WIFI_SETUP_AP_SSID_DYNAMIC
  #define WIFI_SETUP_AP_SSID_DYNAMIC 1
#endif
#ifndef WIFI_SETUP_AP_PASSWORD_DEFAULT
  #define WIFI_SETUP_AP_PASSWORD_DEFAULT "12345678"
#endif
#ifndef WIFI_PORTAL_PRINT_CLIENT_DIAGNOSTICS_MS
  #define WIFI_PORTAL_PRINT_CLIENT_DIAGNOSTICS_MS 8000UL
#endif
#ifndef WIFI_PORTAL_REFRESH_SCAN_MS
  #define WIFI_PORTAL_REFRESH_SCAN_MS 45000UL
#endif
#ifndef WIFI_IOT_EXCELLENT_RSSI_DBM
  #define WIFI_IOT_EXCELLENT_RSSI_DBM -55
#endif
#ifndef WIFI_IOT_GOOD_RSSI_DBM
  #define WIFI_IOT_GOOD_RSSI_DBM -67
#endif
#ifndef WIFI_IOT_ACCEPTABLE_RSSI_DBM
  #define WIFI_IOT_ACCEPTABLE_RSSI_DBM -75
#endif
#ifndef WIFI_IOT_WEAK_RSSI_DBM
  #define WIFI_IOT_WEAK_RSSI_DBM -82
#endif
#ifndef WIFI_IOT_BLOCK_RSSI_DBM
  #define WIFI_IOT_BLOCK_RSSI_DBM -83
#endif
#ifndef WIFI_PORTAL_BLOCK_WEAK_WIFI
  #define WIFI_PORTAL_BLOCK_WEAK_WIFI 1
#endif
#ifndef WIFI_PORTAL_ALLOW_FORCE_WEAK
  #define WIFI_PORTAL_ALLOW_FORCE_WEAK 1
#endif
#ifndef WIFI_PORTAL_VALIDATE_SCAN_BEFORE_SAVE
  #define WIFI_PORTAL_VALIDATE_SCAN_BEFORE_SAVE 1
#endif
#ifndef TELEMETRY_INTERVAL_MIN_MS
  #define TELEMETRY_INTERVAL_MIN_MS 15000UL
#endif
#ifndef TELEMETRY_INTERVAL_MAX_MS
  #define TELEMETRY_INTERVAL_MAX_MS 600000UL
#endif
#ifndef LOCATION_FETCH_ENABLED
  #define LOCATION_FETCH_ENABLED 0
#endif
#ifndef LOCATION_HTTP_TIMEOUT_MS
  #define LOCATION_HTTP_TIMEOUT_MS 3500UL
#endif
#ifndef LOCATION_RETRY_INTERVAL_MS
  #define LOCATION_RETRY_INTERVAL_MS 45000UL
#endif
#ifndef LOCATION_RETRY_SLOW_INTERVAL_MS
  #define LOCATION_RETRY_SLOW_INTERVAL_MS 600000UL
#endif
#ifndef LOCATION_MAX_FAST_ATTEMPTS
  #define LOCATION_MAX_FAST_ATTEMPTS 6
#endif
#ifndef LOCATION_REFRESH_ON_WIFI_CHANGE
  #define LOCATION_REFRESH_ON_WIFI_CHANGE 1
#endif
#ifndef LOCATION_USE_DEFAULT
  #define LOCATION_USE_DEFAULT 0
#endif
#ifndef DEFAULT_CITY
  #define DEFAULT_CITY ""
#endif
#ifndef DEFAULT_COUNTRY_CODE
  #define DEFAULT_COUNTRY_CODE ""
#endif
#ifndef DEFAULT_LATITUDE
  #define DEFAULT_LATITUDE NAN
#endif
#ifndef DEFAULT_LONGITUDE
  #define DEFAULT_LONGITUDE NAN
#endif
#ifndef TB_HOST
  #ifdef THINGSBOARD_HOST
    #define TB_HOST THINGSBOARD_HOST
  #elif defined(THINGSBOARD_SERVER)
    #define TB_HOST THINGSBOARD_SERVER
  #else
    #define TB_HOST "thingsboard.cloud"
  #endif
#endif
#ifndef TB_PORT
  #ifdef THINGSBOARD_PORT
    #define TB_PORT THINGSBOARD_PORT
  #else
    #define TB_PORT 1883
  #endif
#endif
#ifndef TB_TOKEN
  #ifdef THINGSBOARD_TOKEN
    #define TB_TOKEN THINGSBOARD_TOKEN
  #elif defined(THINGSBOARD_ACCESS_TOKEN)
    #define TB_TOKEN THINGSBOARD_ACCESS_TOKEN
  #elif defined(THINGSBOARD_DEVICE_TOKEN)
    #define TB_TOKEN THINGSBOARD_DEVICE_TOKEN
  #elif defined(DEVICE_ACCESS_TOKEN)
    #define TB_TOKEN DEVICE_ACCESS_TOKEN
  #elif defined(ACCESS_TOKEN)
    #define TB_TOKEN ACCESS_TOKEN
  #elif defined(TOKEN)
    #define TB_TOKEN TOKEN
  #else
    #define TB_TOKEN ""
  #endif
#endif
#ifndef HP12_FACTORY_TB_TOKEN
  // Security: never embed a real ThingsBoard token in firmware source.
  // Use local secrets.h or the HP12-SETUP portal instead.
  #define HP12_FACTORY_TB_TOKEN ""
#endif
#ifndef OTA_FOLLOW_REDIRECTS_ENABLED
  #define OTA_FOLLOW_REDIRECTS_ENABLED 1
#endif
#ifndef OTA_REDIRECT_LIMIT
  #define OTA_REDIRECT_LIMIT 8
#endif
#ifndef MQTT_FORCE_IMMEDIATE_AFTER_WIFI_MS
  #define MQTT_FORCE_IMMEDIATE_AFTER_WIFI_MS 2500UL
#endif
#ifndef MQTT_TOKEN_WARN_INTERVAL_MS
  #define MQTT_TOKEN_WARN_INTERVAL_MS 30000UL
#endif
#ifndef FALLBACK_WIFI_SSID
  #ifdef WIFI_SSID
    #define FALLBACK_WIFI_SSID WIFI_SSID
  #else
    #define FALLBACK_WIFI_SSID ""
  #endif
#endif
#ifndef FALLBACK_WIFI_PASSWORD
  #ifdef WIFI_PASSWORD
    #define FALLBACK_WIFI_PASSWORD WIFI_PASSWORD
  #else
    #define FALLBACK_WIFI_PASSWORD ""
  #endif
#endif

// ============================================================================
// OBJECTS
// ============================================================================
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

Preferences wifiPrefs;
DNSServer dnsServer;
WebServer wifiServer(80);

WiFiClient mqttNetClient;
PubSubClient mqttClient(mqttNetClient);

// ============================================================================
// ENUMS / STATE
// ============================================================================
enum ComfortState {
  STATE_STARTUP,
  STATE_SENSOR_ERROR,
  STATE_SENSOR_SUSPECT,
  STATE_GOOD,
  STATE_WARN,
  STATE_DANGER
};

enum AlarmProfile {
  ALARM_QUIET = 0,
  ALARM_NORMAL = 1,
  ALARM_STRONG = 2
};

enum UiPage {
  PAGE_HOME,
  PAGE_FOCUS,
  PAGE_HEAT,
  PAGE_TEMP,
  PAGE_HUM,
  PAGE_LIGHT,
  PAGE_ADVICE,
  PAGE_BASIS,
  PAGE_WIFI,
  PAGE_HELP
};

ComfortState comfortState = STATE_STARTUP;
AlarmProfile alarmProfile = (AlarmProfile)BUZZER_PROFILE_START;
UiPage currentPage = PAGE_HOME;

// Sensor state
float rawTempC = NAN;
float rawHumidity = NAN;
float tempC = NAN;
float humidity = NAN;
float heatIndexC = NAN;
float lightPercent = NAN;
int lightRaw = -1;
float focusScore = NAN;

bool oledReady = false;
bool sensorHasRawData = false;
bool sensorHasValidData = false;
bool lastSensorReadOk = false;
bool lightHasData = false;

// Timing
unsigned long lastSensorReadMs = 0;
unsigned long lastLightReadMs = 0;
unsigned long lastOledUpdateMs = 0;
unsigned long lastLedUpdateMs = 0;
unsigned long lastBlueLedUpdateMs = 0;
unsigned long lastScrollMs = 0;
unsigned long helpPageUntilMs = 0;
unsigned long alarmMutedUntilMs = 0;
unsigned long blueSelfTestUntilMs = 0;
unsigned long scheduledRestartAtMs = 0;

int scrollX = OLED_WIDTH;
bool blinkState = false;

// WiFi / Portal state
String savedWifiSsid = "";
String savedWifiPassword = "";
String activeWifiSsid = "";
String activeWifiPassword = "";
String savedTbToken = "";
String activeTbToken = "";
String activeTbTokenSource = "none";
bool wifiCredentialsLoaded = false;
bool wifiUsingStoredCredentials = false;
bool wifiPortalActive = false;
bool wifiConnectedAtLeastOnceThisBoot = false;
bool wifiConnectAnnounced = false;
unsigned long lastWifiRetryMs = 0;
unsigned long wifiConnectStartedMs = 0;
unsigned int wifiTimeoutCount = 0;
String wifiLastFailure = "none";
String wifiPortalReason = "";
String wifiPortalScanHtml = "";
unsigned long wifiPortalStartedMs = 0;
String wifiPortalApSsid = "";
String wifiPortalApPassword = "";
unsigned long wifiPortalLastDiagMs = 0;
unsigned long wifiPortalLastScanMs = 0;
int wifiPortalLastStationCount = -1;
unsigned long bothButtonsHoldStartedMs = 0;
bool wifiResetPortalTriggered = false;
unsigned long singleButtonHoldStartedMs = 0;
uint8_t singleButtonHoldWhich = 0;
bool wifiSinglePortalTriggered = false;
int wifiLastBestRssiDbm = -999;
String wifiLastScanSummary = "not-scanned";
String wifiLastStatusText = "init";
String wifiPortalStepText = "";
String wifiPortalUserHint = "";
int wifiPortalCandidateRssiDbm = -999;
String wifiPortalScannedSsids[WIFI_PORTAL_SCAN_MAX_ITEMS];
int wifiPortalScannedRssis[WIFI_PORTAL_SCAN_MAX_ITEMS];
int wifiPortalScannedCount = 0;
unsigned long wifiConnectedSinceMs = 0;
bool otaSafeWifiBoot = false;
unsigned long otaSafeWifiUntilMs = 0;
String bootContext = "normal";

// Location / time state
bool timeSyncRequested = false;
bool locationFetched = false;
String ipCity = "";
String ipCountryCode = "";
float latitude = NAN;
float longitude = NAN;
String publicIp = "";
String locationSource = "default";
String locationProvider = "none";
String locationFetchStatus = "not-started";
String locationWifiFingerprint = "";
unsigned long lastLocationAttemptMs = 0;
unsigned long locationLastSuccessMs = 0;
unsigned int locationFetchAttempt = 0;

// MQTT / RPC / OTA state
bool rpcSubscribed = false;
bool attributesSentOnce = false;
bool forceFirstTelemetry = FORCE_FIRST_SYNC_ENABLED;
unsigned long lastMqttRetryMs = 0;
unsigned long lastAttributesMs = 0;
unsigned long lastTelemetryMs = 0;
unsigned long telemetryIntervalMs = TELEMETRY_INTERVAL_MS;
unsigned long telemetrySeq = 0;
bool mqttFirstAttemptAfterWifi = false;
String lastMqttStatusText = "not-started";
String lastRpcMethod = "none";
String lastRpcResult = "none";
unsigned long lastRpcAtMs = 0;

bool otaPending = false;
bool otaInProgress = false;
String otaPendingUrl = "";
String otaTargetVersion = "";
String otaStatus = "idle";
String otaLastError = "";
unsigned long otaVisualHoldUntilMs = 0;

// ============================================================================
// BUTTON STATE
// ============================================================================
struct ButtonState {
  uint8_t pin;
  bool activeLow;
  bool stablePressed;
  bool lastRawPressed;
  unsigned long lastDebounceMs;
  unsigned long pressedAtMs;
  unsigned long releasedAtMs;
  uint8_t clickCount;
  bool longHandled;
};

ButtonState navButton = {BUTTON_NAV_PIN, BUTTON_NAV_ACTIVE_LOW, false, false, 0, 0, 0, 0, false};
ButtonState adviceButton = {BUTTON_ADVICE_PIN, BUTTON_ADVICE_ACTIVE_LOW, false, false, 0, 0, 0, 0, false};

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================
void startWifiSetupPortal(const char* reason);
void clearSavedWifiOnly();
void openWifiSetupPortalAndClearWifi(const char* reason);
void updateWifiSetupPortalNonBlocking();
void updateSerialCommandsNonBlocking();
bool parseWifiSerialCommand(const String &cmd, String &ssid, String &pass);
void saveWifiFromSerialAndRestart(const String &ssid, const String &pass);
void updateCloudNonBlocking();
void sendTelemetryNow(const char* reason);
void drawOtaScreenNow(const char* title, const char* line2, const char* line3);
String buildTelemetryPayload();
String buildStatusJson();
void markRpcResult(const char* method, const char* result);
void updateScheduledRestartNonBlocking();
void initDefaultLocation();
void markOtaSafeWifiBootNext();
void consumeOtaSafeWifiBootFlag();
void applyWifiStabilityTuning();
int wifiSignalQualityPercent();
const char* wifiQualityText();
const char* wifiStatusText(wl_status_t status);
bool wifiLocalIpUsable();
int scanBestRssiForSsid(const String &ssid);

// ============================================================================
// BASIC HELPERS
// ============================================================================
float clampFloat(float v, float lo, float hi) {
  if (isnan(v)) return lo;
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

long clampLong(long v, long lo, long hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

float mapFloatClamped(float x, float inMin, float inMax, float outMin, float outMax) {
  if (inMax == inMin) return outMin;
  float ratio = (x - inMin) / (inMax - inMin);
  ratio = clampFloat(ratio, 0.0, 1.0);
  return outMin + ratio * (outMax - outMin);
}

bool isNonEmptyToken(const char* s) {
  if (!s) return false;
  String x = String(s);
  x.trim();
  if (x.length() == 0) return false;
  String l = x;
  l.toLowerCase();
  if (l.indexOf("your_") >= 0 || l.indexOf("change_me") >= 0 || l.indexOf("replace") >= 0 || l.indexOf("paste_") >= 0) return false;
  if (l.indexOf("placeholder") >= 0 || l.indexOf("access_token_here") >= 0) return false;
  return true;
}

bool isNonEmptyTokenString(const String &s) {
  String x = s;
  x.trim();
  if (x.length() == 0) return false;
  String l = x;
  l.toLowerCase();
  if (l.indexOf("your_") >= 0 || l.indexOf("change_me") >= 0 || l.indexOf("replace") >= 0 || l.indexOf("paste_") >= 0) return false;
  if (l.indexOf("placeholder") >= 0 || l.indexOf("access_token_here") >= 0) return false;
  return true;
}

String tokenPreview(const String &token) {
  if (!isNonEmptyTokenString(token)) return "none";
  if (token.length() <= 6) return "set";
  return token.substring(0, 2) + "***" + token.substring(token.length() - 2);
}

void loadThingsBoardToken() {
  wifiPrefs.begin(WIFI_SETUP_PREF_NAMESPACE, true);
  savedTbToken = wifiPrefs.getString("tbToken", "");
  wifiPrefs.end();
  savedTbToken.trim();

  if (isNonEmptyTokenString(savedTbToken)) {
    activeTbToken = savedTbToken;
    activeTbTokenSource = "preferences";
  } else if (isNonEmptyToken(TB_TOKEN)) {
    activeTbToken = String(TB_TOKEN);
    activeTbTokenSource = "secrets.h";
  } else if (isNonEmptyToken(HP12_FACTORY_TB_TOKEN)) {
    activeTbToken = String(HP12_FACTORY_TB_TOKEN);
    activeTbTokenSource = "factory-fallback-old-working";
  } else {
    activeTbToken = "";
    activeTbTokenSource = "missing";
  }
  activeTbToken.trim();
}

void saveThingsBoardToken(const String &token) {
  String t = token;
  t.trim();
  if (!isNonEmptyTokenString(t)) return;
  wifiPrefs.begin(WIFI_SETUP_PREF_NAMESPACE, false);
  wifiPrefs.putString("tbToken", t);
  wifiPrefs.end();
  savedTbToken = t;
  activeTbToken = t;
  activeTbTokenSource = "preferences";
}

String mqttStateText(int rc) {
  switch (rc) {
    case MQTT_CONNECTION_TIMEOUT: return "timeout/server-or-network-unreachable";
    case MQTT_CONNECTION_LOST: return "connection-lost";
    case MQTT_CONNECT_FAILED: return "tcp-connect-failed";
    case MQTT_DISCONNECTED: return "disconnected";
    case MQTT_CONNECTED: return "connected";
    case MQTT_CONNECT_BAD_PROTOCOL: return "bad-protocol";
    case MQTT_CONNECT_BAD_CLIENT_ID: return "bad-client-id";
    case MQTT_CONNECT_UNAVAILABLE: return "server-unavailable";
    case MQTT_CONNECT_BAD_CREDENTIALS: return "bad-credentials";
    case MQTT_CONNECT_UNAUTHORIZED: return "unauthorized-token-or-device";
    default: return "unknown";
  }
}

void printMqttTokenHint(bool force) {
  static unsigned long lastWarnMs = 0;
  unsigned long now = millis();
  if (!force && lastWarnMs > 0 && now - lastWarnMs < MQTT_TOKEN_WARN_INTERVAL_MS) return;
  lastWarnMs = now;

  Serial.print("[MQTT] Host=");
  Serial.print(TB_HOST);
  Serial.print(":");
  Serial.print(TB_PORT);
  if (activeTbTokenSource.length() == 0 || activeTbTokenSource == "none") loadThingsBoardToken();
  Serial.print(" | tokenConfigured=");
  Serial.print(isNonEmptyTokenString(activeTbToken) ? "YES" : "NO/PLACEHOLDER");
  Serial.print(" | tokenLength=");
  Serial.print(activeTbToken.length());
  Serial.print(" | source=");
  Serial.print(activeTbTokenSource);
  Serial.print(" | preview=");
  Serial.println(tokenPreview(activeTbToken));

  if (!isNonEmptyTokenString(activeTbToken)) {
    Serial.println("[MQTT] SKIP: ThingsBoard token missing. Enter token in HP12-SETUP portal, or put TB_TOKEN in secrets.h.");
    Serial.println("[MQTT] Accepted names: TB_TOKEN, THINGSBOARD_TOKEN, THINGSBOARD_ACCESS_TOKEN, DEVICE_ACCESS_TOKEN, ACCESS_TOKEN.");
  }
}

bool looksPlaceholder(const String &s) {
  String x = s;
  x.trim();
  String l = x;
  l.toLowerCase();
  if (x.length() == 0) return true;
  if (l.indexOf("your_") >= 0) return true;
  if (l.indexOf("change_me") >= 0) return true;
  if (l.indexOf("replace") >= 0) return true;
  if (l.indexOf("fallback") >= 0) return true;
  return false;
}

String htmlEscape(const String &in) {
  String out = in;
  out.replace("&", "&amp;");
  out.replace("<", "&lt;");
  out.replace(">", "&gt;");
  out.replace("\"", "&quot;");
  out.replace("'", "&#39;");
  return out;
}


String jsEscape(const String &in) {
  String out;
  out.reserve(in.length() + 8);
  for (int i = 0; i < (int)in.length(); i++) {
    char c = in.charAt(i);
    if (c == '\\') out += "\\\\";
    else if (c == '\'') out += "\\'";
    else if (c == '"') out += "\\\"";
    else if (c == '\n' || c == '\r') out += ' ';
    else out += c;
  }
  return out;
}

String urlEncode(const String &in) {
  const char *hex = "0123456789ABCDEF";
  String out;
  out.reserve(in.length() * 3);
  for (int i = 0; i < (int)in.length(); i++) {
    uint8_t c = (uint8_t)in.charAt(i);
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
      out += (char)c;
    } else if (c == ' ') {
      out += "+";
    } else {
      out += "%";
      out += hex[(c >> 4) & 0x0F];
      out += hex[c & 0x0F];
    }
  }
  return out;
}

String jsonEscape(const String &in) {
  String out = "";
  out.reserve(in.length() + 8);
  for (int i = 0; i < (int)in.length(); i++) {
    char c = in.charAt(i);
    if (c == '\\') out += "\\\\";
    else if (c == '"') out += "\\\"";
    else if (c == '\n') out += "\\n";
    else if (c == '\r') out += "\\r";
    else if ((uint8_t)c < 32) out += ' ';
    else out += c;
  }
  return out;
}

void jsonComma(String &payload, bool &first) {
  if (first) first = false;
  else payload += ',';
}

void jsonAddString(String &payload, bool &first, const char* key, const String &value) {
  jsonComma(payload, first);
  payload += '"'; payload += key; payload += "\":\"";
  payload += jsonEscape(value);
  payload += '"';
}

void jsonAddString(String &payload, bool &first, const char* key, const char* value) {
  jsonAddString(payload, first, key, String(value));
}

void jsonAddBool(String &payload, bool &first, const char* key, bool value) {
  jsonComma(payload, first);
  payload += '"'; payload += key; payload += "\":";
  payload += value ? "true" : "false";
}

void jsonAddInt(String &payload, bool &first, const char* key, long value) {
  jsonComma(payload, first);
  payload += '"'; payload += key; payload += "\":";
  payload += String(value);
}

void jsonAddFloat(String &payload, bool &first, const char* key, float value, int decimals, bool valid = true) {
  jsonComma(payload, first);
  payload += '"'; payload += key; payload += "\":";
  if (!valid || isnan(value) || isinf(value)) payload += "null";
  else payload += String(value, decimals);
}

const char* resetReasonText(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON:   return "POWERON";
    case ESP_RST_EXT:       return "EXT_RESET";
    case ESP_RST_SW:        return "SW_RESET";
    case ESP_RST_PANIC:     return "PANIC";
    case ESP_RST_INT_WDT:   return "INT_WDT";
    case ESP_RST_TASK_WDT:  return "TASK_WDT";
    case ESP_RST_DEEPSLEEP: return "DEEPSLEEP";
    case ESP_RST_BROWNOUT:  return "BROWNOUT";
    case ESP_RST_SDIO:      return "SDIO";
    default:                return "UNKNOWN";
  }
}

void applyWifiStabilityTuning() {
#if WIFI_COUNTRY_VN_ENABLED
  // Quan trọng tại Việt Nam: một số router dùng kênh 12/13.
  // Nếu ESP32 để country mặc định kiểu US, có thể scan/thấy WiFi kém hoặc không thấy SSID.
  wifi_country_t country = {};
  country.cc[0] = 'V';
  country.cc[1] = 'N';
  country.cc[2] = '\0';
  country.schan = 1;
  country.nchan = 13;
  country.policy = WIFI_COUNTRY_POLICY_AUTO;
  esp_wifi_set_country(&country);
#endif
#if WIFI_SET_SLEEP_DISABLE
  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE);
#endif
#if WIFI_SET_TX_POWER_MAX
  // ESP32 Dev Module: tăng TX power giúp giảm RSSI đỏ/chập chờn khi đặt xa router.
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
#endif
}

const char* wifiStatusText(wl_status_t status) {
  switch (status) {
    case WL_IDLE_STATUS:     return "IDLE";
    case WL_NO_SSID_AVAIL:   return "NO_SSID";
    case WL_SCAN_COMPLETED:  return "SCAN_DONE";
    case WL_CONNECTED:       return "CONNECTED";
    case WL_CONNECT_FAILED:  return "CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "CONNECTION_LOST";
    case WL_DISCONNECTED:    return "DISCONNECTED";
    default:                 return "UNKNOWN";
  }
}

bool wifiLocalIpUsable() {
  if (WiFi.status() != WL_CONNECTED) return false;
  IPAddress ip = WiFi.localIP();
  return !(ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0);
}

int scanBestRssiForSsid(const String &ssid) {
#if WIFI_SCAN_BEFORE_CONNECT
  if (ssid.length() == 0) return -999;
  Serial.print("[WiFi] Scanning 2.4GHz for saved SSID: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  applyWifiStabilityTuning();
  delay(80);

  int n = WiFi.scanNetworks(false, true); // sync scan, include hidden; ESP32 only supports 2.4GHz
  int best = -999;
  int visibleCount = 0;
  String summary = "networks=" + String(n);

  if (n > 0) {
    for (int i = 0; i < n; i++) {
      String found = WiFi.SSID(i);
      int rssi = WiFi.RSSI(i);
      if (found.length() > 0) visibleCount++;
      if (found == ssid && rssi > best) best = rssi;
    }
  }

  summary += ", visible=" + String(visibleCount);
  if (best > -999) summary += ", targetRssi=" + String(best);
  else summary += ", target=not-found";

  wifiLastBestRssiDbm = best;
  wifiLastScanSummary = summary;
  Serial.print("[WiFi] Scan result: ");
  Serial.println(summary);

  WiFi.scanDelete();
  return best;
#else
  (void)ssid;
  wifiLastScanSummary = "scan-disabled";
  return -998;
#endif
}


const char* wifiDoctorRssiLabel(int rssi) {
  if (rssi <= -998) return "KHONG THAY";
  if (rssi >= WIFI_IOT_EXCELLENT_RSSI_DBM) return "RAT MANH";
  if (rssi >= WIFI_IOT_GOOD_RSSI_DBM) return "TOT";
  if (rssi >= WIFI_IOT_ACCEPTABLE_RSSI_DBM) return "ON DINH";
  if (rssi >= WIFI_IOT_WEAK_RSSI_DBM) return "YEU";
  return "QUA YEU";
}

const char* wifiDoctorCssClass(int rssi) {
  if (rssi <= -998) return "badnet";
  if (rssi >= WIFI_IOT_GOOD_RSSI_DBM) return "goodnet";
  if (rssi >= WIFI_IOT_ACCEPTABLE_RSSI_DBM) return "oknet";
  if (rssi >= WIFI_IOT_WEAK_RSSI_DBM) return "warnnet";
  return "badnet";
}

String wifiDoctorHumanAdvice(int rssi) {
  if (rssi <= -998) return "HP12 khong thay WiFi nay tren 2.4GHz. Hay chon WiFi khac hoac dua thiet bi gan router hon.";
  if (rssi >= WIFI_IOT_GOOD_RSSI_DBM) return "Tin hieu tot cho IoT. Co the ket noi.";
  if (rssi >= WIFI_IOT_ACCEPTABLE_RSSI_DBM) return "Tin hieu chap nhan duoc. Co the ket noi, nhung nen dat HP12 thoang hon.";
  if (rssi >= WIFI_IOT_WEAK_RSSI_DBM) return "Tin hieu yeu. Van co the thu, nhung khong khuyen nghi cho lap dat co dinh.";
  return "Tin hieu qua yeu doi voi ESP32. Dien thoai co the vao duoc nhung HP12 rat de fail. Hay chon WiFi manh hon hoac dua thiet bi gan router.";
}

bool wifiDoctorShouldBlockRssi(int rssi) {
#if WIFI_PORTAL_BLOCK_WEAK_WIFI
  return (rssi <= -998) || (rssi <= WIFI_IOT_BLOCK_RSSI_DBM);
#else
  (void)rssi;
  return false;
#endif
}

int lookupPortalCachedRssiForSsid(const String &ssid) {
  int best = -999;
  for (int i = 0; i < wifiPortalScannedCount && i < WIFI_PORTAL_SCAN_MAX_ITEMS; i++) {
    if (wifiPortalScannedSsids[i] == ssid && wifiPortalScannedRssis[i] > best) best = wifiPortalScannedRssis[i];
  }
  return best;
}

int scanRssiForSetupCandidate(const String &ssid) {
  if (ssid.length() == 0) return -999;
  Serial.print("[WiFiDoctor] Validate SSID before save: ");
  Serial.println(ssid);
  if (wifiPortalActive) WiFi.mode(WIFI_AP_STA);
  else WiFi.mode(WIFI_STA);
  applyWifiStabilityTuning();
  delay(120);
  int n = WiFi.scanNetworks(false, true);
  int best = -999;
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      String found = WiFi.SSID(i);
      if (found == ssid && WiFi.RSSI(i) > best) best = WiFi.RSSI(i);
    }
  }
  WiFi.scanDelete();
#if WIFI_SETUP_FORCE_AP_ONLY
  if (wifiPortalActive) {
    WiFi.mode(WIFI_AP);
    applyWifiStabilityTuning();
  }
#endif
  Serial.print("[WiFiDoctor] Candidate RSSI=");
  Serial.print(best);
  Serial.print(" dBm | label=");
  Serial.println(wifiDoctorRssiLabel(best));
  return best;
}

int wifiSignalQualityPercent() {
  if (WiFi.status() != WL_CONNECTED) return 0;
  long rssi = WiFi.RSSI();
  if (rssi <= -100) return 0;
  if (rssi >= -50) return 100;
  return (int)(2 * (rssi + 100));
}

const char* wifiQualityText() {
  int q = wifiSignalQualityPercent();
  if (WiFi.status() != WL_CONNECTED) return "OFFLINE";
  if (q >= 80) return "EXCELLENT";
  if (q >= 60) return "GOOD";
  if (q >= 40) return "FAIR";
  if (q >= 20) return "WEAK";
  return "POOR";
}

void initDefaultLocation() {
#if LOCATION_USE_DEFAULT
  ipCity = String(DEFAULT_CITY);
  ipCountryCode = String(DEFAULT_COUNTRY_CODE);
  latitude = (float)DEFAULT_LATITUDE;
  longitude = (float)DEFAULT_LONGITUDE;
  locationSource = "default";
  locationProvider = "configured-fallback";
  if (locationFetchStatus == "not-started") locationFetchStatus = "fallback-ready";
#endif
}

// ============================================================================
// GPIO HELPERS
// ============================================================================
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
  buzzerWrite(false);
  blueLedWrite(false);
}

bool buttonRawPressed(const ButtonState &btn) {
  int v = digitalRead(btn.pin);
  return btn.activeLow ? (v == LOW) : (v == HIGH);
}

// ============================================================================
// TEXT / STATE HELPERS
// ============================================================================
const char* getStateCode() {
  switch (comfortState) {
    case STATE_GOOD:           return "OPT";
    case STATE_WARN:           return "WRN";
    case STATE_DANGER:         return "STR";
    case STATE_SENSOR_ERROR:   return "ERR";
    case STATE_SENSOR_SUSPECT: return "CHK";
    default:                   return "INI";
  }
}

const char* getStateText() {
  switch (comfortState) {
    case STATE_GOOD:           return "TOI UU";
    case STATE_WARN:           return "CANH BAO";
    case STATE_DANGER:         return "STRESS";
    case STATE_SENSOR_ERROR:   return "LOI DHT";
    case STATE_SENSOR_SUSPECT: return "CHECK";
    default:                   return "KHOI DONG";
  }
}

const char* getThermalText() {
  if (!sensorHasValidData) {
    if (comfortState == STATE_SENSOR_SUSPECT) return "NGHI NGO DHT";
    if (comfortState == STATE_SENSOR_ERROR) return "MAT DHT";
    return "DANG DOC";
  }
  if (heatIndexC >= HEAT_INDEX_DANGER || tempC >= TEMP_DANGER || humidity >= HUM_DANGER) return "QUA TAI NHIET";
  if (heatIndexC >= HEAT_INDEX_WARN || tempC >= TEMP_WARN || humidity >= HUM_WARN) return "LECH NHIET";
  if (tempC < TEMP_GOOD_MIN) return "HOI LANH";
  if (humidity < HUM_GOOD_MIN) return "HOI KHO";
  if (tempC <= TEMP_GOOD_MAX && humidity <= HUM_GOOD_MAX) return "TIEN NGHI";
  return "CAN THEO DOI";
}

const char* getLightText() {
#if LIGHT_SENSOR_ENABLED
  if (!lightHasData || isnan(lightPercent)) return "SANG ?";
  if (lightPercent < LIGHT_DARK_PCT) return "THIEU SANG";
  if (lightPercent < LIGHT_FOCUS_MIN_PCT) return "HOI TOI";
  if (lightPercent <= LIGHT_FOCUS_MAX_PCT) return "TOT CHO MAT";
  if (lightPercent >= LIGHT_GLARE_PCT) return "CHOI GAT";
  return "SANG MANH";
#else
  return "TAT LDR";
#endif
}

const char* getAlarmProfileText() {
  switch (alarmProfile) {
    case ALARM_QUIET:  return "IM";
    case ALARM_STRONG: return "MANH";
    case ALARM_NORMAL:
    default:           return "VUA";
  }
}

const char* getShortActionText() {
  switch (comfortState) {
    case STATE_GOOD:
      return "DUY TRI T/H";
    case STATE_WARN:
      if (lightHasData && lightPercent < LIGHT_FOCUS_MIN_PCT) return "BO SUNG LUX";
      if (lightHasData && lightPercent >= LIGHT_GLARE_PCT) return "CHE CHOI";
      return "TANG DOI LUU";
    case STATE_DANGER:
      return "HA NHIET NGAY";
    case STATE_SENSOR_SUSPECT:
      return "KIEM TRA DHT";
    case STATE_SENSOR_ERROR:
      return "NOI LAI DHT";
    default:
      return "DANG DOC";
  }
}

const char* getAdviceText() {
  switch (comfortState) {
    case STATE_GOOD:
      return "Khong gian dang ho tro deep work. Duy tri nhiet-am, gio nhe va anh sang on dinh.";
    case STATE_WARN:
      if (lightHasData && lightPercent < LIGHT_FOCUS_MIN_PCT) return "Bo sung den ban/anh sang gian tiep. Tranh lam viec lau trong vung thieu lux.";
      if (lightHasData && lightPercent >= LIGHT_GLARE_PCT) return "Giam choi: keo rem, doi goc man hinh, tranh nguon sang chieu thang vao mat.";
      return "Tang doi luu khong khi, bat quat, giam tai nhiet va nghi ngan 2-3 phut neu thay bi.";
    case STATE_DANGER:
      return "Stress nhiet ro. Ha nhiet phong, uong nuoc, tang quat/dieu hoa, giam workload ngay.";
    case STATE_SENSOR_SUSPECT:
      return "Du lieu DHT bat thuong. Kiem tra day DATA, VCC/GND, dien tro pullup 10K va do am bam cam bien.";
    case STATE_SENSOR_ERROR:
      return "DHT22 mat tin hieu. Kiem tra GPIO, dau cam, cap nguon 3V3 va GND truoc khi tin vao so lieu.";
    default:
      return "HP12 dang khoi dong va thu thap du lieu moi truong.";
  }
}

const char* getBasisText() {
  return "HP12 dung Temp + RH + Heat Index + LDR de tao Focus Score. Day la proxy thuc dung, khong phai thiet bi y te.";
}

const char* getScrollingMessage() {
  if (wifiPortalActive) return "WIFI SETUP: ket noi dien thoai vao HP12-SETUP, mo 192.168.4.1 va nhap WiFi moi.";
  if (otaInProgress || otaPending || otaStatus == "success" || otaStatus == "error") return "OTA: khong tat nguon khi dang cap nhat firmware.";
  switch (comfortState) {
    case STATE_GOOD:
      return "SPACE OK: dieu kien hien tai con ho tro tap trung sau. Duy tri gio nhe va anh sang on dinh.";
    case STATE_WARN:
      return "CANH BAO: khong gian bat dau lech focus zone. Xu ly nhe truoc khi suy giam hieu suat.";
    case STATE_DANGER:
      return "STRESS NHIET: can ha nhiet, tang doi luu, bo sung nuoc va giam tai cong viec ngay.";
    case STATE_SENSOR_SUSPECT:
      return "CHECK DHT: du lieu bat thuong, co the do day long, dong nuoc hoac thieu pullup.";
    case STATE_SENSOR_ERROR:
      return "LOI DHT22: mat tin hieu. Kiem tra 3V3, GND, DATA GPIO va dau noi.";
    default:
      return "HP12 Ergonomic Proxy: dang doc du lieu moi truong cho deep work...";
  }
}

// ============================================================================
// SENSOR + FOCUS SCORE
// ============================================================================
bool isSensorValueReasonable(float t, float h) {
  if (isnan(t) || isnan(h)) return false;
  if (t < SENSOR_TEMP_MIN || t > SENSOR_TEMP_MAX) return false;
  if (h < SENSOR_HUM_MIN || h > SENSOR_HUM_MAX) return false;
  if (h >= SENSOR_HUM_SUSPECT) return false;
  return true;
}

float calculateFocusScore(float t, float h, float hi, float lightPct, bool lightValid) {
  if (isnan(t) || isnan(h) || isnan(hi)) return NAN;

  float score = 100.0;

  // Heat Index penalty: càng vượt ngưỡng càng hao năng lượng nhận thức.
  if (hi >= HEAT_INDEX_MED_EXTREME_C) score -= 45.0;
  else if (hi >= HEAT_INDEX_MED_DANGER_C) score -= 30.0;
  else if (hi >= HEAT_INDEX_MED_CAUTION_C) score -= mapFloatClamped(hi, HEAT_INDEX_MED_CAUTION_C, HEAT_INDEX_MED_DANGER_C, 8.0, 25.0);

  // Nhiệt độ lệch khỏi vùng xanh.
  if (t > TEMP_GOOD_MAX) score -= mapFloatClamped(t, TEMP_GOOD_MAX, TEMP_DANGER, 4.0, 20.0);
  if (t < TEMP_GOOD_MIN) score -= mapFloatClamped(TEMP_GOOD_MIN - t, 0.0, 5.0, 3.0, 12.0);

  // Độ ẩm lệch khỏi vùng xanh.
  if (h > HUM_GOOD_MAX) score -= mapFloatClamped(h, HUM_GOOD_MAX, HUM_DANGER, 4.0, 20.0);
  if (h < HUM_GOOD_MIN) score -= mapFloatClamped(HUM_GOOD_MIN - h, 0.0, 20.0, 2.0, 10.0);

  // Ánh sáng: tối hoặc chói đều kéo giảm deep work.
  if (lightValid && !isnan(lightPct)) {
    if (lightPct < LIGHT_DARK_PCT) score -= 18.0;
    else if (lightPct < LIGHT_FOCUS_MIN_PCT) score -= mapFloatClamped(LIGHT_FOCUS_MIN_PCT - lightPct, 0.0, LIGHT_FOCUS_MIN_PCT - LIGHT_DARK_PCT, 4.0, 12.0);
    if (lightPct > LIGHT_FOCUS_MAX_PCT) score -= mapFloatClamped(lightPct, LIGHT_FOCUS_MAX_PCT, LIGHT_GLARE_PCT, 4.0, 15.0);
    if (lightPct >= LIGHT_GLARE_PCT) score -= 18.0;
  }

  return clampFloat(score, FOCUS_SCORE_MIN, FOCUS_SCORE_MAX);
}

ComfortState classifyComfort(float t, float h, float hi, float lightPct, bool lightValid, float score) {
  if (isnan(t) || isnan(h) || isnan(hi)) return STATE_SENSOR_ERROR;

  bool thermalDanger = (hi >= HEAT_INDEX_DANGER || t >= TEMP_DANGER || h >= HUM_DANGER);
  bool thermalWarn = (hi >= HEAT_INDEX_WARN || t >= TEMP_WARN || h >= HUM_WARN ||
                      t < TEMP_GOOD_MIN || h < HUM_GOOD_MIN || t > TEMP_GOOD_MAX || h > HUM_GOOD_MAX);

  bool lightWarn = false;
#if LIGHT_SENSOR_ENABLED
  if (lightValid && !isnan(lightPct)) {
    lightWarn = (lightPct < LIGHT_FOCUS_MIN_PCT || lightPct >= LIGHT_GLARE_PCT);
  }
#endif

  if (thermalDanger || (!isnan(score) && score <= FOCUS_SCORE_DANGER && thermalWarn)) return STATE_DANGER;
  if (thermalWarn || lightWarn || (!isnan(score) && score < FOCUS_SCORE_WARN)) return STATE_WARN;
  return STATE_GOOD;
}

void readSensorNonBlocking() {
  unsigned long now = millis();
  if (now - lastSensorReadMs < SENSOR_READ_INTERVAL_MS) return;
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
  focusScore = calculateFocusScore(tempC, humidity, heatIndexC, lightPercent, lightHasData);
  comfortState = classifyComfort(tempC, humidity, heatIndexC, lightPercent, lightHasData, focusScore);

  Serial.print("[DHT22] OK | T:");
  Serial.print(tempC, 1);
  Serial.print("C H:");
  Serial.print(humidity, 1);
  Serial.print("% HI:");
  Serial.print(heatIndexC, 1);
  Serial.print("C Light:");
  if (lightHasData) Serial.print(lightPercent, 0); else Serial.print("NA");
  Serial.print("% Focus:");
  if (!isnan(focusScore)) Serial.print(focusScore, 1); else Serial.print("NA");
  Serial.print(" State:");
  Serial.println(getStateCode());
}

void readLightNonBlocking() {
#if LIGHT_SENSOR_ENABLED
  unsigned long now = millis();
  if (now - lastLightReadMs < LIGHT_READ_INTERVAL_MS) return;
  lastLightReadMs = now;

  int raw = analogRead(LIGHT_AO_PIN);
  raw = clampLong(raw, 0, 4095);
  float pct = (raw / 4095.0) * 100.0;
#if !LIGHT_ADC_BRIGHT_IS_HIGH
  pct = 100.0 - pct;
#endif

  if (!lightHasData || isnan(lightPercent)) lightPercent = pct;
  else lightPercent = lightPercent * 0.75 + pct * 0.25;

  lightRaw = raw;
  lightHasData = true;

  if (sensorHasValidData) {
    focusScore = calculateFocusScore(tempC, humidity, heatIndexC, lightPercent, lightHasData);
    comfortState = classifyComfort(tempC, humidity, heatIndexC, lightPercent, lightHasData, focusScore);
  }
#endif
}


// ============================================================================
// WiFi identity + IP-location display helpers (v1.13.19)
// ============================================================================
String shortOledText(String s, uint8_t maxChars) {
  s.trim();
  if (maxChars == 0) return "";
  if (s.length() <= maxChars) return s;
  if (maxChars <= 1) return s.substring(0, maxChars);
  return s.substring(0, maxChars - 1) + "~";
}

String wifiConnectedSsidText() {
  String s = "";
  if (WiFi.status() == WL_CONNECTED) s = WiFi.SSID();
  if (s.length() == 0) s = activeWifiSsid;
  if (s.length() == 0) s = savedWifiSsid;
  if (s.length() == 0) s = "unknown";
  return s;
}

String wifiConnectedBssidText() {
  if (WiFi.status() == WL_CONNECTED) return WiFi.BSSIDstr();
  return "";
}

String locationConfidenceText() {
  if (locationFetchStatus == "success" && locationSource == "ip") return "ip-city-level";
  if (WiFi.status() != WL_CONNECTED) return "offline";
  if (locationSource == "pending-ip" || locationFetchStatus.endsWith("pending")) return "pending-refresh";
  if (locationSource == "default" || locationProvider == "configured-fallback") return "fallback-not-current";
  if (locationFetchStatus == "disabled") return "disabled";
  return "retrying";
}

String locationSummaryText() {
  String c = locationConfidenceText();
  if (c == "ip-city-level") {
    String s = "IP loc: ";
    s += ipCity.length() ? ipCity : "city?";
    if (ipCountryCode.length()) { s += ","; s += ipCountryCode; }
    s += " via "; s += locationProvider;
    if (publicIp.length()) { s += " "; s += publicIp; }
    return s;
  }
  if (c == "pending-refresh") return "IP loc: dang cap nhat theo WiFi moi";
  if (c == "fallback-not-current") return "IP loc: tam dung fallback, chua phai vi tri WiFi hien tai";
  if (c == "offline") return "IP loc: offline";
  if (c == "disabled") return "IP loc: disabled";
  return String("IP loc: retry ") + locationFetchStatus;
}

String locationOledShortText() {
  String c = locationConfidenceText();
  if (c == "ip-city-level") {
    String s = "LOC:";
    s += ipCity.length() ? ipCity : "OK";
    if (publicIp.length()) { s += " "; s += publicIp; }
    return shortOledText(s, 21);
  }
  if (c == "pending-refresh") return "LOC:dang cap nhat";
  if (c == "fallback-not-current") return "LOC:fallback tam";
  if (c == "offline") return "LOC:offline";
  if (c == "disabled") return "LOC:off";
  return shortOledText(String("LOC:retry ") + locationFetchStatus, 21);
}

String wifiOledSignalText() {
  if (WiFi.status() != WL_CONNECTED) return "R:-999 Q:0%";
  String s = "R:";
  s += String(WiFi.RSSI());
  s += " Q:";
  s += String(wifiSignalQualityPercent());
  s += "%";
  return s;
}

// ============================================================================
// OLED UI
// ============================================================================
void printClippedText(const char* text, int x, int y, int maxW) {
  int maxChars = maxW / 6;
  String s = String(text);
  if ((int)s.length() > maxChars) s = s.substring(0, maxChars);
  display.setCursor(x, y);
  display.print(s);
}

void drawGauge(int x, int y, int w, int h, float value, float minV, float maxV) {
  display.drawRect(x, y, w, h, SSD1306_WHITE);
  if (isnan(value)) return;
  int fillW = (int)round(mapFloatClamped(value, minV, maxV, 0, w - 2));
  if (fillW > 0) display.fillRect(x + 1, y + 1, fillW, h - 2, SSD1306_WHITE);
}

void showBootScreen() {
  if (!oledReady) return;
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("HP12 DEEP WORK");
  display.drawLine(0, 12, 127, 12, SSD1306_WHITE);
  display.setCursor(0, 20);
  display.print("FW: ");
  display.print(FIRMWARE_VERSION);
  display.setCursor(0, 34);
  display.print("Tropical Config");
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
  if (scrollX < -msgWidth) scrollX = OLED_WIDTH;
}

void drawFooterScroll() {
  display.fillRect(0, 55, 128, 9, SSD1306_BLACK);
  display.drawLine(0, 53, 127, 53, SSD1306_WHITE);
  display.setCursor(scrollX, 56);
  display.print(getScrollingMessage());
}

void drawHomePage() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("HP12 ");
  display.print(FIRMWARE_VERSION);
  printClippedText(getStateText(), 86, 0, 42);
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

  if (wifiPortalActive) {
    display.setCursor(0, 15); display.print("WIFI SETUP PORTAL");
    display.setCursor(0, 28); display.print("AP: "); { String ap = wifiPortalApSsid.length() ? wifiPortalApSsid : String(WIFI_SETUP_AP_SSID); printClippedText(ap.c_str(), 22, 28, 106); }
    display.setCursor(0, 41); display.print("IP: 192.168.4.1");
    return;
  }

  if (comfortState == STATE_SENSOR_ERROR) {
    display.setCursor(0, 17); display.print("DHT22 READ FAILED");
    display.setCursor(0, 31); display.print("GPIO"); display.print(DHT_PIN); display.print(" DATA");
    display.setCursor(0, 44); display.print("3V3 GND 10K");
    return;
  }

  if (comfortState == STATE_SENSOR_SUSPECT) {
    display.setCursor(0, 16); display.print("RAW T:"); display.print(rawTempC, 1); display.print("C");
    display.setCursor(0, 29); display.print("RAW H:"); display.print(rawHumidity, 1); display.print("%");
    display.setCursor(0, 42); display.print("DATA SUSPECT");
    return;
  }

  if (!sensorHasValidData) {
    display.setCursor(0, 26);
    display.print("Waiting data...");
    return;
  }

  display.setCursor(0, 14); display.print("F:");
  if (!isnan(focusScore)) display.print(focusScore, 0); else display.print("--");
  display.print(" "); display.print(getStateCode());

  display.setCursor(66, 14); display.print("L:");
  if (lightHasData) display.print(lightPercent, 0); else display.print("--");
  display.print("%");

  display.setCursor(0, 27); display.print("T:"); display.print(tempC, 1); display.print("C");
  display.setCursor(66, 27); display.print("H:"); display.print(humidity, 0); display.print("%");

  display.setCursor(0, 40); display.print("HI:"); display.print(heatIndexC, 1); display.print("C ");
  printClippedText(getShortActionText(), 62, 40, 66);
}

void drawFocusPage() {
  display.setCursor(0, 0); display.print("H.SUAT NHAN THUC");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  display.setCursor(0, 16); display.print("Focus: ");
  if (!isnan(focusScore)) display.print(focusScore, 1); else display.print("--");
  display.print("/100");
  drawGauge(OLED_GAUGE_X, 29, OLED_GAUGE_W, 8, focusScore, FOCUS_SCORE_MIN, FOCUS_SCORE_MAX);
  display.setCursor(0, 43); display.print(getStateText());
  display.print(" | "); display.print(getShortActionText());
}

void drawHeatPage() {
  display.setCursor(0, 0); display.print("CAM NHAN NHIET");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  display.setCursor(0, 16); display.print("HeatIdx: ");
  if (sensorHasValidData) display.print(heatIndexC, 1); else display.print("--");
  display.print("C");
  drawGauge(OLED_GAUGE_X, 29, OLED_GAUGE_W, 8, heatIndexC, HEAT_BAR_MIN_C, HEAT_BAR_MAX_C);
  display.setCursor(0, 43); display.print("Warn:"); display.print(HEAT_INDEX_WARN, 0);
  display.print(" Red:"); display.print(HEAT_INDEX_DANGER, 0);
}

void drawTempPage() {
  display.setCursor(0, 0); display.print("NHIET DO PHONG");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  display.setCursor(0, 16); display.print("Temp: ");
  if (sensorHasValidData) display.print(tempC, 1); else display.print("--");
  display.print("C");
  drawGauge(OLED_GAUGE_X, 29, OLED_GAUGE_W, 8, tempC, TEMP_BAR_MIN_C, TEMP_BAR_MAX_C);
  display.setCursor(0, 43); display.print("Green <="); display.print(TEMP_GOOD_MAX, 1);
  display.print("C");
}

void drawHumPage() {
  display.setCursor(0, 0); display.print("DO AM TUONG DOI");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  display.setCursor(0, 16); display.print("Hum: ");
  if (sensorHasValidData) display.print(humidity, 1); else display.print("--");
  display.print("%");
  drawGauge(OLED_GAUGE_X, 29, OLED_GAUGE_W, 8, humidity, HUM_BAR_MIN_PCT, HUM_BAR_MAX_PCT);
  display.setCursor(0, 43); display.print("Green <="); display.print(HUM_GOOD_MAX, 0);
  display.print("%");
}

void drawLightPage() {
  display.setCursor(0, 0); display.print("ANH SANG LDR");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  display.setCursor(0, 16); display.print("Light: ");
  if (lightHasData) display.print(lightPercent, 0); else display.print("--");
  display.print("%");
  drawGauge(OLED_GAUGE_X, 29, OLED_GAUGE_W, 8, lightPercent, LIGHT_BAR_MIN_PCT, LIGHT_BAR_MAX_PCT);
  display.setCursor(0, 43); display.print(getLightText());
  display.print(" R:"); display.print(lightRaw);
}

void drawAdvicePage() {
  display.setCursor(0, 0); display.print("TU VAN DEEP WORK");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  printClippedText(getAdviceText(), 0, 18, 126);
  printClippedText(getAdviceText() + min(20, (int)strlen(getAdviceText())), 0, 31, 126);
  printClippedText(getShortActionText(), 0, 44, 126);
}

void drawBasisPage() {
  display.setCursor(0, 0); display.print("CO SO DANH GIA");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  printClippedText(getBasisText(), 0, 18, 126);
  display.setCursor(0, 31); display.print("Temp+RH+HI+LDR");
  display.setCursor(0, 44); display.print("Proxy, not medical");
}

void drawWifiPage() {
  display.setCursor(0, 0); display.print("MANG KET NOI");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

  if (wifiPortalActive) {
    display.setCursor(0, 15); display.print("Portal: ACTIVE");
    display.setCursor(0, 27);
    printClippedText(wifiPortalStepText.length() ? wifiPortalStepText.c_str() : "Mo 192.168.4.1", 0, 27, 126);
    display.setCursor(0, 39);
    display.print("AP:");
    String ap = wifiPortalApSsid.length() ? wifiPortalApSsid : String(WIFI_SETUP_AP_SSID);
    printClippedText(ap.c_str(), 18, 39, 110);
    display.setCursor(0, 50);
    display.print("Client:"); display.print(WiFi.softAPgetStationNum()); display.print(" IP:4.1");
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    String ssid = wifiConnectedSsidText();
    display.setCursor(0, 15);
    display.print("SSID:");
    printClippedText(ssid.c_str(), 30, 15, 98);

    display.setCursor(0, 27);
    display.print("IP:");
    String ip = WiFi.localIP().toString();
    printClippedText(ip.c_str(), 18, 27, 110);

    display.setCursor(0, 39);
    String sig = wifiOledSignalText();
    display.print(sig);
    display.print(" M:");
    display.print(mqttClient.connected() ? "OK" : "NO");

    display.setCursor(0, 50);
    String loc = locationOledShortText();
    printClippedText(loc.c_str(), 0, 50, 126);
    return;
  }

  display.setCursor(0, 15); display.print("WiFi: OFFLINE");
  display.setCursor(0, 28);
  display.print("Fail:"); printClippedText(wifiLastFailure.c_str(), 34, 28, 94);
  display.setCursor(0, 41);
  display.print("Last:"); printClippedText(activeWifiSsid.c_str(), 30, 41, 98);
  display.setCursor(0, 52);
  display.print("Hold 2 nut: setup");
}

void drawHelpPage() {
  display.setCursor(0, 0); display.print("HUONG DAN NUT");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  display.setCursor(0, 15); display.print("NAV 1: Next tab");
  display.setCursor(0, 27); display.print("NAV 3: Mute alarm");
  display.setCursor(0, 39); display.print("ADV: Advice/Basis");
  display.setCursor(0, 51); display.print("Hold 2 nut: WiFi");
}

bool otaVisualActive() {
  return otaInProgress || otaPending || millis() < otaVisualHoldUntilMs;
}

void drawOtaPage() {
  display.setCursor(0, 0); display.print("CAP NHAT OTA");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  display.setCursor(0, 16); display.print("Status: "); display.print(otaStatus);
  display.setCursor(0, 29); display.print("Target: "); printClippedText(otaTargetVersion.c_str(), 48, 29, 80);
  display.setCursor(0, 42);
  if (otaStatus == "error") printClippedText(otaLastError.c_str(), 0, 42, 126);
  else display.print("Khong tat nguon");
}

void drawOtaScreenNow(const char* title, const char* line2, const char* line3) {
  if (!oledReady) return;
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  printClippedText(title, 0, 0, 126);
  display.drawLine(0, 12, 127, 12, SSD1306_WHITE);
  printClippedText(line2, 0, 22, 126);
  printClippedText(line3, 0, 42, 126);
  display.display();
}

void updateOledNonBlocking() {
  if (!oledReady) return;
  unsigned long now = millis();
  if (now - lastOledUpdateMs < OLED_UPDATE_INTERVAL_MS) return;
  lastOledUpdateMs = now;

  if (currentPage == PAGE_HELP && helpPageUntilMs > 0 && now > helpPageUntilMs) {
    currentPage = PAGE_HOME;
    helpPageUntilMs = 0;
  }

  updateScrollPositionNonBlocking();

  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  if (otaVisualActive()) {
    drawOtaPage();
    display.display();
    return;
  }

  switch (currentPage) {
    case PAGE_FOCUS:  drawFocusPage(); break;
    case PAGE_HEAT:   drawHeatPage(); break;
    case PAGE_TEMP:   drawTempPage(); break;
    case PAGE_HUM:    drawHumPage(); break;
    case PAGE_LIGHT:  drawLightPage(); break;
    case PAGE_ADVICE: drawAdvicePage(); break;
    case PAGE_BASIS:  drawBasisPage(); break;
    case PAGE_WIFI:   drawWifiPage(); break;
    case PAGE_HELP:   drawHelpPage(); break;
    case PAGE_HOME:
    default:          drawHomePage(); break;
  }

  if (currentPage == PAGE_HOME || currentPage == PAGE_ADVICE || currentPage == PAGE_BASIS || currentPage == PAGE_WIFI) {
    drawFooterScroll();
  }

  display.display();
}

// ============================================================================
// LED + BUZZER PATTERNS
// ============================================================================
bool pulsePatternActive(unsigned long now, unsigned long periodMs, unsigned long onMs, unsigned long gapMs, int count) {
  if (periodMs == 0 || onMs == 0 || count <= 0) return false;
  unsigned long phase = now % periodMs;
  for (int i = 0; i < count; i++) {
    unsigned long start = i * (onMs + gapMs);
    if (phase >= start && phase < start + onMs) return true;
  }
  return false;
}

bool alarmIsMuted() {
  return alarmMutedUntilMs > 0 && millis() < alarmMutedUntilMs;
}

bool shouldBuzzerBeOn(unsigned long now) {
  if (alarmIsMuted()) return false;
  if (wifiPortalActive || otaInProgress) return false;

  if (comfortState == STATE_WARN && BUZZER_WARN_ENABLED) {
    return pulsePatternActive(now, BUZZER_WARN_PERIOD_MS, BUZZER_WARN_ON_MS, 0, 1);
  }

  if (comfortState != STATE_DANGER || !sensorHasValidData) return false;

  if (alarmProfile == ALARM_QUIET) return false;
  if (alarmProfile == ALARM_STRONG) {
    return pulsePatternActive(now, BUZZER_STRONG_PERIOD_MS, BUZZER_STRONG_ON_MS, BUZZER_STRONG_GAP_MS, BUZZER_STRONG_COUNT);
  }
  return pulsePatternActive(now, BUZZER_NORMAL_PERIOD_MS, BUZZER_NORMAL_ON_MS, BUZZER_NORMAL_GAP_MS, BUZZER_NORMAL_COUNT);
}

void updateEnvironmentLedAndBuzzerNonBlocking() {
  unsigned long now = millis();
  if (now - lastLedUpdateMs < LED_UPDATE_INTERVAL_MS) return;
  lastLedUpdateMs = now;

  ledWrite(LED_GREEN_PIN, false);
  ledWrite(LED_YELLOW_PIN, false);
  ledWrite(LED_RED_PIN, false);

  if (wifiPortalActive) {
    blinkState = ((now / LED_START_BLINK_MS) % 2) == 0;
    ledWrite(LED_YELLOW_PIN, blinkState);
    buzzerWrite(false);
    return;
  }

  if (otaInProgress) {
    blinkState = ((now / 160) % 2) == 0;
    ledWrite(LED_YELLOW_PIN, blinkState);
    buzzerWrite(false);
    return;
  }

  switch (comfortState) {
    case STATE_GOOD:
      // Xanh ổn định = không cần hành động, phù hợp deep work.
      ledWrite(LED_GREEN_PIN, true);
      break;

    case STATE_WARN:
      blinkState = ((now / LED_WARN_BLINK_MS) % 2) == 0;
      ledWrite(LED_YELLOW_PIN, blinkState);
      break;

    case STATE_DANGER:
      blinkState = ((now / LED_DANGER_BLINK_MS) % 2) == 0;
      ledWrite(LED_RED_PIN, blinkState);
      break;

    case STATE_SENSOR_SUSPECT:
      blinkState = ((now / LED_SENSOR_BLINK_MS) % 2) == 0;
      ledWrite(LED_YELLOW_PIN, blinkState);
      ledWrite(LED_RED_PIN, !blinkState);
      break;

    case STATE_SENSOR_ERROR:
      blinkState = ((now / 1200UL) % 2) == 0;
      ledWrite(LED_RED_PIN, blinkState);
      break;

    case STATE_STARTUP:
    default:
      blinkState = ((now / LED_START_BLINK_MS) % 2) == 0;
      ledWrite(LED_YELLOW_PIN, blinkState);
      break;
  }

  buzzerWrite(shouldBuzzerBeOn(now));
}

void updateBlueLedNonBlocking() {
#if ONBOARD_BLUE_LED_ENABLED
  unsigned long now = millis();
  if (now - lastBlueLedUpdateMs < BLUE_LED_UPDATE_INTERVAL_MS) return;
  lastBlueLedUpdateMs = now;

  if (now < blueSelfTestUntilMs) {
    blueLedWrite(((now / 120UL) % 2) == 0);
    return;
  }

  if (wifiPortalActive) {
    blueLedWrite(((now / 250UL) % 2) == 0);
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    blueLedWrite(((now / 1000UL) % 2) == 0);
    return;
  }

  // Server-rack heartbeat: 2 pulse nếu MQTT OK, 1 pulse nếu chỉ WiFi OK.
  int count = mqttClient.connected() ? 2 : 1;
  blueLedWrite(pulsePatternActive(now, BLUE_LED_ALIVE_PERIOD_MS, BLUE_LED_PULSE_ON_MS, BLUE_LED_PULSE_GAP_MS, count));
#endif
}

// ============================================================================
// BUTTONS
// ============================================================================
void handleNavClicks(uint8_t clicks) {
  if (clicks == 1) {
    currentPage = (UiPage)((currentPage + 1) % 10);
  } else if (clicks == 2) {
    currentPage = PAGE_HELP;
    helpPageUntilMs = millis() + HELP_PAGE_DURATION_MS;
  } else if (clicks >= 3) {
    if (alarmIsMuted()) alarmMutedUntilMs = 0;
    else alarmMutedUntilMs = millis() + ALARM_MUTE_MS;
    currentPage = PAGE_HOME;
  }
}

void handleAdviceClicks(uint8_t clicks) {
  if (clicks == 1) currentPage = PAGE_ADVICE;
  else if (clicks == 2) currentPage = PAGE_BASIS;
  else if (clicks >= 3) currentPage = PAGE_WIFI;
}

void handleNavLongPress() {
  if (alarmProfile == ALARM_QUIET) alarmProfile = ALARM_NORMAL;
  else if (alarmProfile == ALARM_NORMAL) alarmProfile = ALARM_STRONG;
  else alarmProfile = ALARM_QUIET;
  currentPage = PAGE_HOME;
}

void handleAdviceLongPress() {
  currentPage = PAGE_HOME;
}

void updateButton(ButtonState &btn, void (*clickHandler)(uint8_t), void (*longHandler)()) {
  unsigned long now = millis();
  bool rawPressed = buttonRawPressed(btn);

  if (rawPressed != btn.lastRawPressed) {
    btn.lastDebounceMs = now;
    btn.lastRawPressed = rawPressed;
  }

  if (now - btn.lastDebounceMs >= BUTTON_DEBOUNCE_MS) {
    if (rawPressed != btn.stablePressed) {
      btn.stablePressed = rawPressed;

      if (btn.stablePressed) {
        btn.pressedAtMs = now;
        btn.longHandled = false;
      } else {
        if (!btn.longHandled) {
          btn.clickCount++;
          btn.releasedAtMs = now;
        }
      }
    }
  }

  if (btn.stablePressed && !btn.longHandled && now - btn.pressedAtMs >= BUTTON_LONG_PRESS_MS) {
    btn.longHandled = true;
    btn.clickCount = 0;
    longHandler();
  }

  if (!btn.stablePressed && btn.clickCount > 0 && now - btn.releasedAtMs >= BUTTON_CLICK_WINDOW_MS) {
    uint8_t clicks = btn.clickCount;
    btn.clickCount = 0;
    clickHandler(clicks);
  }
}


void clearSavedWifiOnly() {
  wifiPrefs.begin(WIFI_SETUP_PREF_NAMESPACE, false);
  wifiPrefs.remove("ssid");
  wifiPrefs.remove("pass");
  wifiPrefs.end();

  savedWifiSsid = "";
  savedWifiPassword = "";
  activeWifiSsid = "";
  activeWifiPassword = "";
  wifiCredentialsLoaded = true;
  wifiUsingStoredCredentials = false;
  wifiConnectStartedMs = 0;
  lastWifiRetryMs = 0;
  wifiTimeoutCount = 0;
  wifiConnectAnnounced = false;
  wifiLastFailure = "manual-setup";

  // KHÔNG xóa tbToken trong Preferences: đổi WiFi không được làm mất ThingsBoard token.
  loadThingsBoardToken();
}

void openWifiSetupPortalAndClearWifi(const char* reason) {
#if WIFI_SETUP_PORTAL_ENABLED
  Serial.print("[WiFiSetup] Force setup portal: ");
  Serial.println(reason ? reason : "manual");
  Serial.println("[WiFiSetup] Preserving saved WiFi and ThingsBoard token until a NEW WiFi is successfully saved.");
  wifiLastFailure = reason ? String(reason) : String("manual-setup");
  wifiConnectStartedMs = 0;
  lastWifiRetryMs = 0;
  wifiTimeoutCount = 0;
  wifiConnectAnnounced = false;
  startWifiSetupPortal(reason ? reason : "manual");
#else
  (void)reason;
#endif
}

void updateWifiSetupTriggerNonBlocking() {
#if WIFI_SETUP_PORTAL_ENABLED
  unsigned long now = millis();
  bool navPressed = buttonRawPressed(navButton);
  bool advPressed = buttonRawPressed(adviceButton);

  // Cách chính: giữ đồng thời NAV + ADVICE 5.5s.
  if (navPressed && advPressed) {
    singleButtonHoldStartedMs = 0;
    singleButtonHoldWhich = 0;
    wifiSinglePortalTriggered = false;
    if (bothButtonsHoldStartedMs == 0) {
      bothButtonsHoldStartedMs = now;
      Serial.println("[WiFiSetup] Both buttons detected. Keep holding to open HP12-SETUP...");
    }
    if (!wifiResetPortalTriggered && now - bothButtonsHoldStartedMs >= WIFI_SETUP_TRIGGER_HOLD_MS) {
      wifiResetPortalTriggered = true;
      Serial.println("[WiFiSetup] Manual setup: opening HP12-SETUP; keeping old WiFi until new save succeeds.");
      openWifiSetupPortalAndClearWifi("manual-both-buttons");
    }
    return;
  }

  bothButtonsHoldStartedMs = 0;
  wifiResetPortalTriggered = false;

  // Fallback thực địa: giữ riêng NAV hoặc ADVICE 8.5s để mở setup.
  // Mặc định TẮT vì nó xung đột với thao tác click/long-press dùng để chuyển tab OLED.
  // Khi cần bật lại, set WIFI_SETUP_SINGLE_HOLD_ENABLED = 1 trong config.h.
#if defined(WIFI_SETUP_SINGLE_HOLD_ENABLED) && WIFI_SETUP_SINGLE_HOLD_ENABLED
  if (navPressed || advPressed) {
    uint8_t which = navPressed ? 1 : 2;
    if (singleButtonHoldStartedMs == 0 || singleButtonHoldWhich != which) {
      singleButtonHoldStartedMs = now;
      singleButtonHoldWhich = which;
      wifiSinglePortalTriggered = false;
      Serial.print("[WiFiSetup] Single-button setup hold detected: ");
      Serial.println(which == 1 ? "NAV" : "ADVICE");
    }
    if (!wifiSinglePortalTriggered && now - singleButtonHoldStartedMs >= WIFI_SETUP_SINGLE_HOLD_MS) {
      wifiSinglePortalTriggered = true;
      Serial.println("[WiFiSetup] Single-button fallback setup: opening HP12-SETUP; keeping old WiFi until new save succeeds.");
      openWifiSetupPortalAndClearWifi(which == 1 ? "manual-nav-long" : "manual-advice-long");
    }
    return;
  }
#endif

  singleButtonHoldStartedMs = 0;
  singleButtonHoldWhich = 0;
  wifiSinglePortalTriggered = false;
#endif
}

void updateButtonsNonBlocking() {
  if (wifiPortalActive) return;

  // Chỉ khóa xử lý click thường khi người dùng đang giữ ĐỒNG THỜI NAV+ADVICE để vào setup.
  // Không khóa khi chỉ nhấn một nút, nếu không NAV/ADVICE sẽ không còn chuyển tab OLED được.
  bool navPressed = buttonRawPressed(navButton);
  bool advPressed = buttonRawPressed(adviceButton);
  if (navPressed && advPressed) return;

  updateButton(navButton, handleNavClicks, handleNavLongPress);
  updateButton(adviceButton, handleAdviceClicks, handleAdviceLongPress);
}

// ============================================================================
// WIFI CREDENTIALS + PORTAL
// ============================================================================
bool hasActiveWifiCredentials() {
  return activeWifiSsid.length() > 0 && !looksPlaceholder(activeWifiSsid);
}

void loadWifiCredentials() {
  if (wifiCredentialsLoaded) return;

  wifiPrefs.begin(WIFI_SETUP_PREF_NAMESPACE, true);
  savedWifiSsid = wifiPrefs.getString("ssid", "");
  savedWifiPassword = wifiPrefs.getString("pass", "");
  wifiPrefs.end();

  savedWifiSsid.trim();

  if (!looksPlaceholder(savedWifiSsid)) {
    activeWifiSsid = savedWifiSsid;
    activeWifiPassword = savedWifiPassword;
    wifiUsingStoredCredentials = true;
    Serial.print("[WiFi] Loaded saved WiFi: ");
    Serial.println(activeWifiSsid);
  } else {
    String fallbackSsid = String(FALLBACK_WIFI_SSID);
    String fallbackPass = String(FALLBACK_WIFI_PASSWORD);
    fallbackSsid.trim();

    if (!looksPlaceholder(fallbackSsid)) {
      activeWifiSsid = fallbackSsid;
      activeWifiPassword = fallbackPass;
      wifiUsingStoredCredentials = false;
      Serial.print("[WiFi] Using fallback WiFi from secrets.h: ");
      Serial.println(activeWifiSsid);
    } else {
      activeWifiSsid = "";
      activeWifiPassword = "";
      wifiUsingStoredCredentials = false;
    }
  }

  wifiCredentialsLoaded = true;
}

void saveWifiCredentials(const String &ssid, const String &pass) {
  wifiPrefs.begin(WIFI_SETUP_PREF_NAMESPACE, false);
  wifiPrefs.putString("ssid", ssid);
  wifiPrefs.putString("pass", pass);
  wifiPrefs.end();

  savedWifiSsid = ssid;
  savedWifiPassword = pass;
  activeWifiSsid = ssid;
  activeWifiPassword = pass;
  wifiCredentialsLoaded = true;
  wifiUsingStoredCredentials = true;
}

void markOtaSafeWifiBootNext() {
#if OTA_SAFE_WIFI_BOOT_ENABLED
  wifiPrefs.begin(WIFI_SETUP_PREF_NAMESPACE, false);
  wifiPrefs.putBool("otaSafeBoot", true);
  wifiPrefs.putString("otaSafeVer", FIRMWARE_VERSION);
  wifiPrefs.end();
  Serial.println("[OTA] Marked next boot as OTA-safe WiFi boot. Auto portal will be suppressed during grace window.");
#endif
}

void consumeOtaSafeWifiBootFlag() {
#if OTA_SAFE_WIFI_BOOT_ENABLED
  wifiPrefs.begin(WIFI_SETUP_PREF_NAMESPACE, true);
  bool flag = wifiPrefs.getBool("otaSafeBoot", false);
  String fromVer = wifiPrefs.getString("otaSafeVer", "unknown");
  wifiPrefs.end();

  if (flag) {
    wifiPrefs.begin(WIFI_SETUP_PREF_NAMESPACE, false);
    wifiPrefs.remove("otaSafeBoot");
    wifiPrefs.remove("otaSafeVer");
    wifiPrefs.end();

    otaSafeWifiBoot = true;
    otaSafeWifiUntilMs = millis() + OTA_SAFE_WIFI_GRACE_MS;
    bootContext = "ota-safe-wifi";
    Serial.print("[BOOT] OTA-safe WiFi boot detected from ");
    Serial.print(fromVer);
    Serial.print(". Suppressing automatic HP12-SETUP for ");
    Serial.print(OTA_SAFE_WIFI_GRACE_MS / 1000UL);
    Serial.println("s so device can reconnect to saved WiFi after OTA.");
  }
#endif
}


String buildPortalApSsid() {
#if WIFI_SETUP_AP_SSID_DYNAMIC
  uint64_t mac = ESP.getEfuseMac();
  char suffix[9];
  snprintf(suffix, sizeof(suffix), "%04X", (uint16_t)(mac & 0xFFFF));
  String ssid = String(WIFI_SETUP_AP_SSID) + "-" + String(suffix);
  if (ssid.length() > 31) ssid = ssid.substring(0, 31);
  return ssid;
#else
  return String(WIFI_SETUP_AP_SSID);
#endif
}

String effectivePortalApPassword() {
  String p = String(WIFI_SETUP_AP_PASSWORD);
  p.trim();
  if (p.length() >= 8) return p;
#if defined(WIFI_SETUP_AP_PASSWORD_DEFAULT)
  String d = String(WIFI_SETUP_AP_PASSWORD_DEFAULT);
  d.trim();
  if (d.length() >= 8) return d;
#endif
  return String("");
}

String buildPortalScanListHtml() {
#if WIFI_PORTAL_SCAN_LIST_ENABLED
  Serial.println("[WiFiSetup] Scanning nearby WiFi for phone setup page...");
  if (wifiPortalActive) WiFi.mode(WIFI_AP_STA);
  else WiFi.mode(WIFI_STA);
  applyWifiStabilityTuning();
  delay(120);
  int n = WiFi.scanNetworks(false, true); // sync scan; include hidden; ESP32 = 2.4GHz only

  const int maxItems = WIFI_PORTAL_SCAN_MAX_ITEMS;
  String ssids[maxItems];
  int rssis[maxItems];
  int encs[maxItems];
  int count = 0;
  for (int i = 0; i < maxItems; i++) { rssis[i] = -999; encs[i] = 0; }

  if (n > 0) {
    for (int i = 0; i < n; i++) {
      String ssid = WiFi.SSID(i);
      ssid.trim();
      if (ssid.length() == 0) continue;
      int rssi = WiFi.RSSI(i);
      int enc = WiFi.encryptionType(i);

      bool duplicate = false;
      for (int k = 0; k < count; k++) {
        if (ssids[k] == ssid) {
          duplicate = true;
          if (rssi > rssis[k]) { rssis[k] = rssi; encs[k] = enc; }
          break;
        }
      }
      if (duplicate) continue;

      int pos = count < maxItems ? count++ : maxItems - 1;
      ssids[pos] = ssid; rssis[pos] = rssi; encs[pos] = enc;

      // Sort roughly by strongest RSSI.
      for (int a = pos; a > 0 && rssis[a] > rssis[a - 1]; a--) {
        String ts = ssids[a - 1]; ssids[a - 1] = ssids[a]; ssids[a] = ts;
        int tr = rssis[a - 1]; rssis[a - 1] = rssis[a]; rssis[a] = tr;
        int te = encs[a - 1]; encs[a - 1] = encs[a]; encs[a] = te;
      }
    }
  }

  wifiPortalScannedCount = count;
  for (int i = 0; i < WIFI_PORTAL_SCAN_MAX_ITEMS; i++) {
    if (i < count) { wifiPortalScannedSsids[i] = ssids[i]; wifiPortalScannedRssis[i] = rssis[i]; }
    else { wifiPortalScannedSsids[i] = ""; wifiPortalScannedRssis[i] = -999; }
  }

  String html;
  html.reserve(4200);
  html += "<div class='netbox'><b>WiFi 2.4GHz gan HP12</b>";
  html += "<p class='hint'>HP12 dung anten nho hon dien thoai. Nen chon WiFi <b>tu ";
  html += String(WIFI_IOT_ACCEPTABLE_RSSI_DBM);
  html += " dBm tro len</b>. Duoi ";
  html += String(WIFI_IOT_BLOCK_RSSI_DBM);
  html += " dBm se bi chan de tranh vong lap setup.</p>";

  if (count == 0) {
    html += "<p class='hint bad'>Khong thay WiFi 2.4GHz nao. Hay dua HP12 gan router hon, hoac dam bao router co bat 2.4GHz/WPA2.</p>";
  } else {
    int bestOk = -1;
    for (int i = 0; i < count; i++) {
      if (rssis[i] >= WIFI_IOT_ACCEPTABLE_RSSI_DBM) { bestOk = i; break; }
    }
    if (bestOk >= 0) {
      html += "<p class='recommend'>Khuyen nghi: <b>";
      html += htmlEscape(ssids[bestOk]);
      html += "</b> (";
      html += String(rssis[bestOk]);
      html += " dBm - ";
      html += wifiDoctorRssiLabel(rssis[bestOk]);
      html += ").</p>";
    } else {
      html += "<p class='recommend bad'>Chua co WiFi nao dat nguong IoT on dinh. Hay doi vi tri HP12/gian day anten/gan router hon.</p>";
    }

    for (int i = 0; i < count; i++) {
      String sec = (encs[i] == WIFI_AUTH_OPEN) ? "OPEN" : "WPA";
      bool blocked = wifiDoctorShouldBlockRssi(rssis[i]);
      html += "<a class='net ";
      html += wifiDoctorCssClass(rssis[i]);
      if (blocked) html += " blocked";
      html += "' href='/?ssid=";
      html += urlEncode(ssids[i]);
      html += "' onclick=\"return pick('";
      html += jsEscape(ssids[i]);
      html += "')\">";
      html += htmlEscape(ssids[i]);
      html += " <span>";
      html += String(rssis[i]);
      html += " dBm · ";
      html += sec;
      html += " · ";
      html += wifiDoctorRssiLabel(rssis[i]);
      if (blocked) html += " · KHONG KHUYEN NGHI";
      html += "</span></a>";
    }
  }
  html += "</div>";
  WiFi.scanDelete();
#if WIFI_SETUP_FORCE_AP_ONLY
  if (wifiPortalActive) {
    WiFi.mode(WIFI_AP);
    applyWifiStabilityTuning();
  }
#endif
  return html;
#else
  return String("");
#endif
}

String portalPageHtml(const String &message = "") {
  String preset = activeWifiSsid.length() ? activeWifiSsid : savedWifiSsid;
  if (wifiServer.hasArg("ssid")) preset = wifiServer.arg("ssid");
  String current = htmlEscape(preset);
  String apName = wifiPortalApSsid.length() ? wifiPortalApSsid : String(WIFI_SETUP_AP_SSID);
  String apPass = wifiPortalApPassword;
  String html;
  html.reserve(13500);
  html += "<!doctype html><html lang='vi'><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1'>";
  html += "<meta http-equiv='Cache-Control' content='no-store'>";
  html += "<title>HP12 WiFi Setup</title>";
  html += "<style>";
  html += "body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Arial;background:#071b2a;color:#fff;min-height:100vh;padding:14px;}";
  html += ".wrap{max-width:520px;margin:0 auto}.card{background:linear-gradient(145deg,rgba(7,107,93,.96),rgba(6,26,43,.96));border:1px solid rgba(255,255,255,.22);border-radius:22px;box-shadow:0 18px 45px rgba(0,0,0,.35);overflow:hidden}.top{padding:20px 20px 12px;border-bottom:1px solid rgba(255,255,255,.15)}h1{margin:0;font-size:27px}.sub{margin-top:8px;color:#d6fff6;font-size:14px;line-height:1.48}.body{padding:18px 20px}.step{display:flex;gap:10px;align-items:flex-start;margin:11px 0;padding:12px;border-radius:14px;background:rgba(255,255,255,.11)}.num{flex:0 0 28px;height:28px;border-radius:50%;background:#7fffd4;color:#07342f;font-weight:900;display:flex;align-items:center;justify-content:center}.txt{font-size:14px;line-height:1.45}.pill{display:inline-block;border:1px solid rgba(255,255,255,.35);border-radius:999px;padding:7px 10px;margin:0 5px 8px 0;font-size:13px;background:rgba(255,255,255,.1)}label{display:block;margin:14px 0 7px;font-weight:800}input{width:100%;box-sizing:border-box;border:0;border-radius:13px;padding:14px;font-size:17px;outline:none;background:#fff;color:#111}.row{display:grid;grid-template-columns:1fr;gap:10px}button,.btn{display:block;width:100%;box-sizing:border-box;text-align:center;text-decoration:none;margin-top:14px;border:0;border-radius:15px;padding:15px 16px;font-size:17px;font-weight:900;color:#08352f;background:#7fffd4;box-shadow:0 8px 20px rgba(127,255,212,.22)}.btn2{background:#eafff8;color:#07342f}.danger{background:#ffd8d8;color:#5c1111}.hint{font-size:13px;line-height:1.55;color:#e8fffa;margin-top:13px}.ok{color:#b7ffd8}.bad{color:#ffe0e0}.msg{padding:12px;border-radius:13px;background:rgba(255,255,255,.14);margin:12px 0}.small{font-size:12px;color:#cdeee8;margin-top:12px}.netbox{margin-top:14px;padding:13px;border-radius:16px;background:rgba(0,0,0,.2)}.net{display:block;text-decoration:none;color:#08352f;background:#eafff8;border-radius:12px;padding:12px;margin-top:8px;font-weight:900}.net span{float:right;color:#456;font-weight:650;font-size:12px}.goodnet{border-left:7px solid #3ee58f}.oknet{border-left:7px solid #9ee58f}.warnnet{border-left:7px solid #ffca42}.badnet{border-left:7px solid #ff5a5a}.blocked{opacity:.82}.recommend{padding:10px;border-radius:12px;background:rgba(127,255,212,.16);border:1px solid rgba(127,255,212,.28);font-size:13px;line-height:1.45}.force{display:flex;gap:10px;align-items:flex-start;margin-top:12px;padding:12px;border-radius:13px;background:rgba(255,197,66,.14);border:1px solid rgba(255,197,66,.35)}.force input{width:auto;margin-top:3px}.copy{font-family:ui-monospace,Consolas,monospace;background:rgba(0,0,0,.22);padding:2px 6px;border-radius:7px}.warnbox{background:rgba(255,197,66,.16);border:1px solid rgba(255,197,66,.35);padding:12px;border-radius:14px;margin-top:12px}.footer{padding:0 20px 18px}.mini{font-size:12px;opacity:.9}";
  html += "</style>";
  html += "<script>function pick(s){document.getElementById('ssid').value=s;document.getElementById('pass').focus();return false;} function togglePass(){var p=document.getElementById('pass');p.type=p.type==='password'?'text':'password';}</script>";
  html += "</head><body><div class='wrap'><main class='card'><section class='top'><h1>HP12 WiFi Setup</h1>";
  html += "<div class='sub'>Dung dien thoai de doi WiFi cho HP12. Dien thoai chi ket noi vao mang setup; WiFi moi duoc chon trong trang nay.</div></section><section class='body'>";
  html += "<span class='pill'>AP: "; html += htmlEscape(apName); html += "</span><span class='pill'>IP: 192.168.4.1</span>";
  if (apPass.length()) { html += "<span class='pill'>Pass AP: "; html += htmlEscape(apPass); html += "</span>"; } else { html += "<span class='pill'>AP mo, khong mat khau</span>"; }
  if (message.length()) { html += "<div class='msg'>"; html += message; html += "</div>"; }
  html += "<div class='warnbox'><b>Neu dien thoai bao khong co Internet:</b> chon <b>Van ket noi / Use without Internet / Keep WiFi connection</b>. Neu van bi day ra, tat tam 4G/5G roi mo lai <span class='copy'>http://192.168.4.1</span>.</div>";
  html += "<div class='step'><div class='num'>1</div><div class='txt'>Dien thoai phai dang ket noi vao <b>"; html += htmlEscape(apName); html += "</b>. Khong chuyen WiFi dien thoai sang WiFi moi.</div></div>";
  html += "<div class='step'><div class='num'>2</div><div class='txt'>Chon WiFi co song tot. Neu WiFi hien <b>QUA YEU</b>, HP12 se khong luu de tranh lap setup.</div></div>";
  html += "<div class='step'><div class='num'>3</div><div class='txt'>Nhap mat khau WiFi moi, bam <b>Luu & Ket noi</b>. HP12 se reboot va vao ThingsBoard lai.</div></div>";
  html += "<div class='warnbox'><b>Location:</b> sau khi vao WiFi moi, HP12 tu cap nhat vi tri theo <b>public IP/router</b>. Day la vi tri gan dung theo mang, khong phai GPS.</div>";
  html += "<p class='hint'>Ly do mo portal: <b>"; html += htmlEscape(wifiPortalReason); html += "</b>. Loi gan nhat: <b>"; html += htmlEscape(wifiLastFailure); html += "</b>.</p>";
  if (wifiPortalUserHint.length()) { html += "<div class='msg'>"; html += htmlEscape(wifiPortalUserHint); html += "</div>"; }
  String tokenStat = isNonEmptyTokenString(activeTbToken) ? (String("da co token, nguon: ") + activeTbTokenSource + String(", ") + tokenPreview(activeTbToken)) : String("chua co token");
  html += wifiPortalScanHtml;
  html += "<form method='POST' action='/save' autocomplete='off'><label>Ten WiFi moi / SSID</label><input id='ssid' name='ssid' required maxlength='32' placeholder='Vi du: FPT Telecom-20BD' value='";
  html += current;
  html += "'><label>Mat khau WiFi moi</label><input id='pass' name='pass' type='password' maxlength='64' placeholder='De trong neu WiFi open'><a class='btn btn2' href='#' onclick='togglePass();return false;'>Hien / an mat khau</a>";
  html += "<label>ThingsBoard Access Token HP12</label><input name='tbtoken' maxlength='80' placeholder='De trong neu khong doi token'>";
  html += "<p class='hint'>Trang thai token: <b>"; html += htmlEscape(tokenStat); html += "</b>. Doi WiFi khong xoa token ThingsBoard.</p>";
  html += "<div class='force'><input type='checkbox' name='forceWeak' value='1'><div class='txt'><b>Ky thuat vien:</b> van cho phep thu WiFi yeu. Nguoi dung binh thuong khong nen tick muc nay.</div></div>";
  html += "<button type='submit'>Kiem tra song & Luu WiFi</button></form>";
  html += "<a class='btn btn2' href='/'>Tai lai danh sach WiFi</a>";
  html += "<p class='small'>Neu trang nay khong luu duoc, go truc tiep tren trinh duyet: <span class='copy'>http://192.168.4.1/save?ssid=TEN_WIFI&pass=MATKHAU</span></p>";
  html += "</section><section class='footer'><p class='mini'>HP12 chi luu SSID/mat khau trong bo nho ESP32. Khong dua secrets.h len GitHub.</p></section></main></div></body></html>";
  return html;
}

void handlePortalRoot() {
  wifiPortalStepText = "2 Chon WiFi 2.4G";
  Serial.print("[WiFiSetup] HTTP GET ");
  Serial.print(wifiServer.uri());
  Serial.print(" from ");
  Serial.println(wifiServer.client().remoteIP());
  wifiServer.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
  wifiServer.sendHeader("Pragma", "no-cache");
  wifiServer.send(200, "text/html; charset=utf-8", portalPageHtml());
}

void handlePortalHealth() {
  String out = "HP12-SETUP OK\nAP=";
  out += wifiPortalApSsid.length() ? wifiPortalApSsid : String(WIFI_SETUP_AP_SSID);
  out += "\nIP="; out += WiFi.softAPIP().toString();
  out += "\nclients="; out += String(WiFi.softAPgetStationNum());
  out += "\nreason="; out += wifiPortalReason;
  out += "\nlastFailure="; out += wifiLastFailure;
  wifiServer.send(200, "text/plain; charset=utf-8", out);
}

void handlePortalScanRefresh() {
  Serial.println("[WiFiSetup] HTTP /scan requested. Refreshing WiFi list.");
  wifiPortalScanHtml = buildPortalScanListHtml();
  wifiPortalLastScanMs = millis();
  wifiServer.sendHeader("Location", "/");
  wifiServer.send(302, "text/plain", "Redirecting");
}

void handlePortalSave() {
  Serial.print("[WiFiSetup] HTTP SAVE /save from ");
  Serial.println(wifiServer.client().remoteIP());
  String ssid = wifiServer.arg("ssid");
  String pass = wifiServer.arg("pass");
  String tbtoken = wifiServer.arg("tbtoken");
  ssid.trim();
  tbtoken.trim();

  if (ssid.length() == 0) {
    wifiServer.send(400, "text/html; charset=utf-8", portalPageHtml("<span class='bad'>SSID khong duoc de trong.</span>"));
    return;
  }
  if (pass.length() > 0 && pass.length() < 8) {
    wifiServer.send(400, "text/html; charset=utf-8", portalPageHtml("<span class='bad'>Mat khau WPA/WPA2 phai tu 8 ky tu tro len. Neu WiFi open thi de trong.</span>"));
    return;
  }

  wifiPortalStepText = "3 Kiem tra WiFi";
  wifiPortalCandidateRssiDbm = -999;
#if WIFI_PORTAL_VALIDATE_SCAN_BEFORE_SAVE
  wifiPortalCandidateRssiDbm = lookupPortalCachedRssiForSsid(ssid);
  if (wifiPortalCandidateRssiDbm <= -998) {
    // Fallback only when the user typed SSID manually and it was not in the latest portal list.
    // Use AP+STA scan to keep the phone connected as much as possible.
    wifiPortalCandidateRssiDbm = scanRssiForSetupCandidate(ssid);
  }
  bool forceWeak = wifiServer.hasArg("forceWeak") && wifiServer.arg("forceWeak") == "1";
  if (wifiDoctorShouldBlockRssi(wifiPortalCandidateRssiDbm) && !forceWeak) {
    wifiLastFailure = wifiPortalCandidateRssiDbm <= -998 ? "ssid-not-visible-2g-before-save" : "wifi-too-weak-for-esp32";
    wifiPortalStepText = "WiFi qua yeu";
    wifiPortalUserHint = String("WiFi ") + ssid + String(" dang ") + String(wifiPortalCandidateRssiDbm) + String(" dBm - ") + wifiDoctorRssiLabel(wifiPortalCandidateRssiDbm) + String(". ") + wifiDoctorHumanAdvice(wifiPortalCandidateRssiDbm);
    Serial.print("[WiFiDoctor] BLOCK save. ");
    Serial.println(wifiPortalUserHint);
    wifiServer.send(200, "text/html; charset=utf-8", portalPageHtml(String("<span class='bad'>Khong luu WiFi nay: ") + htmlEscape(wifiPortalUserHint) + String("</span>")));
    return;
  }
  wifiPortalUserHint = String("WiFi ") + ssid + String(" = ") + String(wifiPortalCandidateRssiDbm) + String(" dBm - ") + wifiDoctorRssiLabel(wifiPortalCandidateRssiDbm) + String(". ") + wifiDoctorHumanAdvice(wifiPortalCandidateRssiDbm);
#endif

  saveWifiCredentials(ssid, pass);
  if (tbtoken.length() > 0) {
    if (isNonEmptyTokenString(tbtoken)) {
      saveThingsBoardToken(tbtoken);
      Serial.println("[WiFiSetup] Saved ThingsBoard token from portal.");
    } else {
      wifiServer.send(400, "text/html; charset=utf-8", portalPageHtml("<span class='bad'>ThingsBoard token khong hop le/placeholder.</span>"));
      return;
    }
  }
  wifiPortalStepText = "Da luu - reboot";
  Serial.print("[WiFiSetup] Saved new WiFi: ");
  Serial.println(ssid);

  wifiServer.send(200, "text/html; charset=utf-8", portalPageHtml("<span class='ok'>Da luu WiFi/Token. HP12 se tu khoi dong lai. Neu van fail, hay chon WiFi co RSSI manh hon -75 dBm.</span>"));
  scheduledRestartAtMs = millis() + WIFI_SETUP_RESTART_MS;
}

void safeStopStaBeforePortal() {
  mqttClient.disconnect();
  delay(10);

  // Cốt lõi sửa lỗi: không đổi config AP khi STA còn đang connecting.
  WiFi.disconnect(true, true);
  delay(WIFI_STA_STOP_SETTLE_MS);
  WiFi.mode(WIFI_OFF);
  delay(WIFI_STA_STOP_SETTLE_MS);
  WiFi.persistent(false);
}

void startWifiSetupPortal(const char* reason) {
#if WIFI_SETUP_PORTAL_ENABLED
  if (wifiPortalActive) return;

  wifiPortalActive = true;
  wifiPortalReason = String(reason ? reason : "unknown");
  wifiPortalStartedMs = millis();
  wifiPortalStepText = "1 Ket noi dien thoai";
  wifiPortalUserHint = "";
  wifiPortalCandidateRssiDbm = -999;
  wifiConnectStartedMs = 0;
  lastWifiRetryMs = 0;
  wifiConnectAnnounced = false;
  rpcSubscribed = false;

  Serial.print("[WiFiSetup] Starting portal. Reason: ");
  Serial.println(wifiPortalReason);
  wifiPortalApSsid = buildPortalApSsid();
  wifiPortalApPassword = effectivePortalApPassword();
  wifiPortalLastDiagMs = 0;
  wifiPortalLastStationCount = -1;

  safeStopStaBeforePortal();
  wifiPortalScanHtml = buildPortalScanListHtml();
  wifiPortalLastScanMs = millis();
  WiFi.mode(WIFI_OFF);
  delay(WIFI_STA_STOP_SETTLE_MS);

  IPAddress apIP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

#if WIFI_SETUP_FORCE_AP_ONLY
  WiFi.mode(WIFI_AP);
#else
  WiFi.mode(WIFI_AP_STA);
#endif
  applyWifiStabilityTuning();
  delay(100);
  WiFi.softAPConfig(apIP, gateway, subnet);

  bool apOk = false;
  WiFi.softAPsetHostname("HP12-SETUP");
  if (wifiPortalApPassword.length() >= 8) {
    apOk = WiFi.softAP(wifiPortalApSsid.c_str(), wifiPortalApPassword.c_str(), WIFI_SETUP_AP_CHANNEL, 0, WIFI_SETUP_AP_MAX_CONNECTIONS);
  } else {
    apOk = WiFi.softAP(wifiPortalApSsid.c_str(), (const char*)NULL, WIFI_SETUP_AP_CHANNEL, 0, WIFI_SETUP_AP_MAX_CONNECTIONS);
  }

  delay(150);

  dnsServer.stop();
  dnsServer.start(WIFI_SETUP_DNS_PORT, "*", apIP);

  wifiServer.on("/", HTTP_GET, handlePortalRoot);
  wifiServer.on("/setup", HTTP_GET, handlePortalRoot);
  wifiServer.on("/ping", HTTP_GET, [](){ wifiServer.send(200, "text/plain", "HP12-SETUP OK"); });
  wifiServer.on("/health", HTTP_GET, handlePortalHealth);
  wifiServer.on("/scan", HTTP_GET, handlePortalScanRefresh);
  wifiServer.on("/connect", HTTP_GET, handlePortalSave);
  wifiServer.on("/connect", HTTP_POST, handlePortalSave);
  wifiServer.on("/save", HTTP_GET, handlePortalSave);   // phone captive browser friendly
  wifiServer.on("/save", HTTP_POST, handlePortalSave);  // desktop/full browser friendly
  wifiServer.on("/favicon.ico", HTTP_GET, [](){ wifiServer.send(204, "text/plain", ""); });
  // Captive portal probes for Android/iOS/Windows. Returning the setup page makes phones show the sign-in page.
  wifiServer.on("/generate_204", HTTP_GET, handlePortalRoot);
  wifiServer.on("/gen_204", HTTP_GET, handlePortalRoot);
  wifiServer.on("/fwlink", HTTP_GET, handlePortalRoot);
  wifiServer.on("/connecttest.txt", HTTP_GET, handlePortalRoot);
  wifiServer.on("/ncsi.txt", HTTP_GET, handlePortalRoot);
  wifiServer.on("/hotspot-detect.html", HTTP_GET, handlePortalRoot);
  wifiServer.on("/library/test/success.html", HTTP_GET, handlePortalRoot);
  wifiServer.on("/success.txt", HTTP_GET, handlePortalRoot);
  wifiServer.onNotFound(handlePortalRoot);
  wifiServer.begin();

  currentPage = PAGE_WIFI;
  Serial.print("[WiFiSetup] Portal AP ");
  Serial.print(apOk ? "started" : "failed");
  Serial.print(" | SSID=");
  Serial.print(wifiPortalApSsid);
  Serial.print(" | PASS=");
  Serial.print(wifiPortalApPassword.length() ? wifiPortalApPassword : String("<open>"));
  Serial.print(" | CH=");
  Serial.print(WIFI_SETUP_AP_CHANNEL);
  Serial.print(" | IP=");
  Serial.println(WiFi.softAPIP());
#else
  (void)reason;
#endif
}

void updateWifiSetupPortalNonBlocking() {
  if (!wifiPortalActive) return;
  dnsServer.processNextRequest();
  wifiServer.handleClient();

  unsigned long now = millis();
  int stations = WiFi.softAPgetStationNum();
  if (stations != wifiPortalLastStationCount || now - wifiPortalLastDiagMs >= WIFI_PORTAL_PRINT_CLIENT_DIAGNOSTICS_MS) {
    wifiPortalLastStationCount = stations;
    wifiPortalLastDiagMs = now;
    Serial.print("[WiFiSetup] Portal alive | AP=");
    Serial.print(wifiPortalApSsid.length() ? wifiPortalApSsid : String(WIFI_SETUP_AP_SSID));
    Serial.print(" | clients=");
    Serial.print(stations);
    Serial.print(" | IP=");
    Serial.println(WiFi.softAPIP());
    if (stations == 0) {
      wifiPortalStepText = "Cho dien thoai vao AP";
      Serial.println("[WiFiSetup] Phone not connected yet. On phone: connect to HP12-SETUP AP, keep connection without Internet, open http://192.168.4.1");
    } else if (wifiPortalStepText.length() == 0 || wifiPortalStepText.startsWith("Cho ") || wifiPortalStepText.startsWith("1 ")) {
      wifiPortalStepText = "Mo 192.168.4.1";
    }
  }
}

void startWifiConnectionAttempt() {
  if (!hasActiveWifiCredentials()) return;

#if WIFI_SCAN_BEFORE_CONNECT
  // Khi mang thiết bị sang địa điểm mới: nếu SSID cũ không còn xuất hiện trên băng 2.4GHz,
  // không đợi timeout dài lặp lại; mở portal nhanh để nhập WiFi mới.
  // Sau khi đã từng online trong boot này thì KHÔNG tự mở portal, chỉ retry STA để giữ cloud.
  int bestRssi = scanBestRssiForSsid(activeWifiSsid);
#if WIFI_FAST_PORTAL_IF_SSID_NOT_FOUND
  if (!wifiConnectedAtLeastOnceThisBoot && bestRssi <= -999) {
    wifiLastFailure = "ssid-not-visible-2g";
    Serial.println("[WiFi] Saved SSID not visible on 2.4GHz. Opening setup portal for new location/WiFi.");
    startWifiSetupPortal("ssid-not-visible-2g");
    return;
  }
#endif
  if (bestRssi > -999 && bestRssi < WIFI_SCAN_MIN_RSSI_DBM) {
    wifiLastFailure = "ssid-very-weak";
    Serial.print("[WiFi] Warning: saved SSID is very weak, RSSI=");
    Serial.println(bestRssi);
  }
#if WIFI_OPEN_PORTAL_IMMEDIATELY_IF_WEAK
  if (!wifiConnectedAtLeastOnceThisBoot && bestRssi > -999 && bestRssi <= WIFI_WEAK_RSSI_PORTAL_DBM) {
    wifiLastFailure = "ssid-visible-weak-preconnect";
    Serial.print("[WiFi] Saved SSID is weak, RSSI=");
    Serial.print(bestRssi);
#if OTA_SAFE_WIFI_BOOT_ENABLED
    if (otaSafeWifiBoot && millis() < otaSafeWifiUntilMs && !WIFI_AUTO_PORTAL_ON_WEAK_RSSI_AFTER_OTA) {
      Serial.println(". OTA-safe boot: bypassing weak-RSSI portal and trying saved WiFi first.");
    } else
#endif
    {
      Serial.println(". Opening HP12-SETUP before connecting so phone can choose a better WiFi.");
      startWifiSetupPortal("ssid-visible-weak-preconnect");
      return;
    }
  }
#endif
#endif

  unsigned long now = millis();
  wifiConnectStartedMs = now;
  lastWifiRetryMs = now;
  wifiConnectAnnounced = true;
  wifiLastStatusText = "connecting";

  Serial.print("[WiFi] Connecting to ");
  Serial.print(activeWifiSsid);
  Serial.print(" | pass=");
  Serial.print(activeWifiPassword.length() > 0 ? "WPA/WPA2" : "OPEN");
  Serial.print(" | stored=");
  Serial.println(wifiUsingStoredCredentials ? "yes" : "fallback");

  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  applyWifiStabilityTuning();
  WiFi.setHostname(DEVICE_NAME);
  WiFi.setAutoReconnect(true);
  WiFi.disconnect(false, false);
  delay(WIFI_REJOIN_CLEAN_DISCONNECT_MS);
  WiFi.begin(activeWifiSsid.c_str(), activeWifiPassword.c_str());
}

void requestTimeSyncOnce() {
  if (timeSyncRequested) return;
  timeSyncRequested = true;
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  Serial.println("[NTP] Time sync requested.");
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

String extractJsonRawValue(const String &json, const char* key) {
  String pattern = "\"";
  pattern += key;
  pattern += "\"";
  int keyPos = json.indexOf(pattern);
  if (keyPos < 0) return "";
  int colonPos = json.indexOf(':', keyPos);
  if (colonPos < 0) return "";
  int start = colonPos + 1;
  while (start < (int)json.length() && isspace(json.charAt(start))) start++;
  int end = start;
  bool inString = false;
  int braces = 0;
  for (; end < (int)json.length(); end++) {
    char c = json.charAt(end);
    if (c == '"' && (end == 0 || json.charAt(end - 1) != '\\')) inString = !inString;
    if (!inString) {
      if (c == '{' || c == '[') braces++;
      if (c == '}' || c == ']') {
        if (braces == 0) break;
        braces--;
      }
      if (braces == 0 && c == ',') break;
    }
  }
  String v = json.substring(start, end);
  v.trim();
  if (v.startsWith("\"") && v.endsWith("\"") && v.length() >= 2) v = v.substring(1, v.length() - 1);
  return v;
}

float extractJsonFloatValue(const String &json, const char* key, float fallback) {
  String raw = extractJsonRawValue(json, key);
  raw.trim();
  if (raw.length() == 0) return fallback;
  return raw.toFloat();
}

String currentLocationWifiFingerprint() {
  if (WiFi.status() != WL_CONNECTED) return String("offline");
  String fp = WiFi.SSID();
  fp += "|";
  fp += WiFi.BSSIDstr();
  fp += "|";
  fp += WiFi.gatewayIP().toString();
  fp += "|";
  fp += WiFi.localIP().toString();
  return fp;
}

void resetLocationFetchForWifiIfNeeded() {
#if LOCATION_REFRESH_ON_WIFI_CHANGE
  if (WiFi.status() != WL_CONNECTED) return;
  String fp = currentLocationWifiFingerprint();
  if (fp == locationWifiFingerprint) return;

  locationWifiFingerprint = fp;
  locationFetched = false;
  locationFetchAttempt = 0;
  lastLocationAttemptMs = 0;
  publicIp = "";
  locationSource = "pending-ip";
  locationProvider = "none";
  locationFetchStatus = "wifi-changed-pending";

  // Không giữ tọa độ cũ khi sang WiFi/địa điểm khác; dùng fallback tạm thời
  // cho tới khi IP-location fetch thành công.
  initDefaultLocation();

  Serial.print("[LOC] WiFi fingerprint changed. Will refresh IP-location for SSID=");
  Serial.println(WiFi.SSID());
#endif
}

bool commitLocationFromJson(const String &body, const char* provider) {
  String city = "";
  String cc = "";
  String ip = "";
  float lat = NAN;
  float lon = NAN;
  bool ok = false;

  if (String(provider) == "ip-api") {
    ok = (extractJsonStringValue(body, "status") == "success");
    city = extractJsonStringValue(body, "city");
    cc = extractJsonStringValue(body, "countryCode");
    ip = extractJsonStringValue(body, "query");
    lat = extractJsonFloatValue(body, "lat", NAN);
    lon = extractJsonFloatValue(body, "lon", NAN);
  } else if (String(provider) == "ipwho.is") {
    String success = extractJsonRawValue(body, "success");
    success.toLowerCase();
    ok = (success == "true" || success == "1");
    city = extractJsonStringValue(body, "city");
    cc = extractJsonStringValue(body, "country_code");
    ip = extractJsonStringValue(body, "ip");
    lat = extractJsonFloatValue(body, "latitude", NAN);
    lon = extractJsonFloatValue(body, "longitude", NAN);
  } else if (String(provider) == "ipapi.co") {
    // ipapi.co không có trường status=success; coi là hợp lệ nếu có lat/lon.
    city = extractJsonStringValue(body, "city");
    cc = extractJsonStringValue(body, "country_code");
    ip = extractJsonStringValue(body, "ip");
    lat = extractJsonFloatValue(body, "latitude", NAN);
    lon = extractJsonFloatValue(body, "longitude", NAN);
    ok = (!isnan(lat) && !isnan(lon));
  }

  if (!ok || isnan(lat) || isnan(lon)) return false;

  ipCity = city;
  ipCountryCode = cc;
  publicIp = ip;
  latitude = lat;
  longitude = lon;
  locationProvider = String(provider);
  locationSource = "ip";
  locationFetchStatus = "success";
  locationFetched = true;
  locationLastSuccessMs = millis();

  // Vị trí là dữ liệu động theo WiFi/IP nên phải gửi lại attributes/telemetry.
  attributesSentOnce = false;
  forceFirstTelemetry = true;

  Serial.print("[LOC] Fetched by ");
  Serial.print(provider);
  Serial.print(": ");
  Serial.print(ipCity);
  Serial.print(", ");
  Serial.print(ipCountryCode);
  Serial.print(" | lat=");
  Serial.print(latitude, 6);
  Serial.print(" lon=");
  Serial.print(longitude, 6);
  if (publicIp.length() > 0) {
    Serial.print(" | publicIP=");
    Serial.print(publicIp);
  }
  Serial.println();
  return true;
}

bool fetchLocationViaHttpRaw(const char* provider, const char* host, const char* path) {
  WiFiClient client;
  client.setTimeout((uint16_t)((LOCATION_HTTP_TIMEOUT_MS + 999UL) / 1000UL));

  Serial.print("[LOC] raw HTTP connect ");
  Serial.print(host);
  Serial.print(path);
  Serial.println();

  if (!client.connect(host, 80)) {
    locationFetchStatus = String(provider) + ":tcp-connect-failed";
    Serial.print("[LOC] ");
    Serial.print(provider);
    Serial.println(" TCP connect failed. Will retry with another provider.");
    return false;
  }

  client.print(String("GET ") + path + " HTTP/1.0\r\n");
  client.print(String("Host: ") + host + "\r\n");
  client.print(String("User-Agent: ") + DEVICE_NAME + "/" + FIRMWARE_VERSION + "\r\n");
  client.print("Accept: application/json\r\n");
  client.print("Connection: close\r\n\r\n");

  String response;
  response.reserve(1800);
  unsigned long start = millis();
  while (millis() - start < LOCATION_HTTP_TIMEOUT_MS && (client.connected() || client.available())) {
    while (client.available()) {
      char c = (char)client.read();
      if (response.length() < 2200) response += c;
    }
    delay(1);
  }
  client.stop();

  int headerEnd = response.indexOf("\r\n\r\n");
  if (headerEnd < 0) {
    locationFetchStatus = String(provider) + ":no-http-body";
    Serial.print("[LOC] ");
    Serial.print(provider);
    Serial.println(" no usable HTTP body.");
    return false;
  }

  String header = response.substring(0, headerEnd);
  String body = response.substring(headerEnd + 4);
  if (header.indexOf(" 200 ") >= 0 || header.indexOf(" 200 OK") >= 0) {
    if (commitLocationFromJson(body, provider)) return true;
    locationFetchStatus = String(provider) + ":bad-json";
    Serial.print("[LOC] ");
    Serial.print(provider);
    Serial.println(" raw HTTP 200 but JSON payload was not usable.");
    return false;
  }

  int firstLineEnd = header.indexOf("\r\n");
  String firstLine = firstLineEnd > 0 ? header.substring(0, firstLineEnd) : header;
  locationFetchStatus = String(provider) + ":" + firstLine;
  Serial.print("[LOC] ");
  Serial.print(provider);
  Serial.print(" raw HTTP failed: ");
  Serial.println(firstLine);
  return false;
}

bool fetchLocationViaProvider(const char* provider, const char* url, bool secure) {
  HTTPClient http;
  http.setTimeout(LOCATION_HTTP_TIMEOUT_MS);
  http.setReuse(false);
  http.useHTTP10(true); // giảm lỗi chunked/stream timeout trên một số router/captive network

  bool began = false;
  WiFiClient plainClient;
  WiFiClientSecure secureClient;
  if (secure) {
    secureClient.setInsecure();
    began = http.begin(secureClient, url);
  } else {
    began = http.begin(plainClient, url);
  }

  if (!began) {
    locationFetchStatus = String(provider) + ":begin-failed";
    Serial.print("[LOC] ");
    Serial.print(provider);
    Serial.println(" begin failed.");
    return false;
  }

  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    http.end();
    if (commitLocationFromJson(body, provider)) return true;
    locationFetchStatus = String(provider) + ":bad-json";
    Serial.print("[LOC] ");
    Serial.print(provider);
    Serial.println(" returned HTTP 200 but payload was not usable.");
    return false;
  }

  String err = http.errorToString(code);
  http.end();
  locationFetchStatus = String(provider) + ":http-" + String(code);
  Serial.print("[LOC] ");
  Serial.print(provider);
  Serial.print(" failed, HTTP code: ");
  Serial.print(code);
  if (err.length() > 0) {
    Serial.print(" (");
    Serial.print(err);
    Serial.print(")");
  }
  Serial.println(". Will retry with another provider; keeping fallback/last safe location.");
  return false;
}

void fetchIpLocationOnce() {
#if !LOCATION_FETCH_ENABLED
  if (!locationFetched) {
    locationFetched = true;
    locationSource = "disabled";
    locationFetchStatus = "disabled";
  }
  return;
#else
  if (WiFi.status() != WL_CONNECTED) return;

  resetLocationFetchForWifiIfNeeded();
  if (locationFetched) return;

  unsigned long now = millis();
  unsigned long retryInterval = (locationFetchAttempt < LOCATION_MAX_FAST_ATTEMPTS)
                                  ? LOCATION_RETRY_INTERVAL_MS
                                  : LOCATION_RETRY_SLOW_INTERVAL_MS;
  if (lastLocationAttemptMs > 0 && now - lastLocationAttemptMs < retryInterval) return;
  lastLocationAttemptMs = now;

  const unsigned int providerIndex = locationFetchAttempt % 3;
  locationFetchAttempt++;

  Serial.print("[LOC] Fetch attempt ");
  Serial.print(locationFetchAttempt);
  Serial.print(" via ");

  if (providerIndex == 0) {
    Serial.println("ip-api raw-http");
    fetchLocationViaHttpRaw("ip-api", "ip-api.com", "/json/?fields=status,message,countryCode,city,lat,lon,query");
  } else if (providerIndex == 1) {
    Serial.println("ipwho.is https");
    fetchLocationViaProvider("ipwho.is", "https://ipwho.is/?fields=success,message,ip,country_code,city,latitude,longitude", true);
  } else {
    Serial.println("ipapi.co https");
    fetchLocationViaProvider("ipapi.co", "https://ipapi.co/json/", true);
  }
#endif
}



void updateWiFiNonBlocking() {
  if (wifiPortalActive) return;
  if (!wifiCredentialsLoaded) loadWifiCredentials();

  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiLocalIpUsable()) {
      unsigned long now = millis();
      if (wifiConnectedSinceMs == 0) wifiConnectedSinceMs = now;
      if (now - wifiConnectedSinceMs > WIFI_DHCP_GRACE_MS) {
        wifiLastFailure = "dhcp-no-ip";
        wifiLastStatusText = "dhcp-no-ip";
        Serial.println("[WiFi] Connected to AP but DHCP did not provide IP. Rejoining WiFi...");
        wifiConnectStartedMs = 0;
        WiFi.disconnect(false, false);
      }
      return;
    }

    if (!wifiConnectedAtLeastOnceThisBoot || !wifiConnectAnnounced) {
      wifiConnectedAtLeastOnceThisBoot = true;
      wifiConnectAnnounced = true;
      wifiConnectedSinceMs = millis();
      wifiConnectStartedMs = 0;
      wifiTimeoutCount = 0;
      wifiLastFailure = "none";
      wifiLastStatusText = String("CONNECTED:") + WiFi.SSID();
      mqttFirstAttemptAfterWifi = false;
      lastMqttRetryMs = 0;
      forceFirstTelemetry = FORCE_FIRST_SYNC_ENABLED;
      Serial.print("[WiFi] Connected. IP=");
      Serial.print(WiFi.localIP());
      Serial.print(" | RSSI=");
      Serial.print(WiFi.RSSI());
      Serial.print("dBm | Quality=");
      Serial.print(wifiSignalQualityPercent());
      Serial.print("% | Gateway=");
      Serial.println(WiFi.gatewayIP());
      requestTimeSyncOnce();
      if (otaSafeWifiBoot) {
        otaSafeWifiBoot = false;
        bootContext = "ota-safe-wifi-reconnected";
        Serial.println("[BOOT] OTA-safe WiFi boot completed: saved WiFi reconnected, normal auto-portal policy restored.");
      }
    }
    // Location phải là dữ liệu động theo WiFi/IP; không tắt, không bỏ cuộc sau lỗi -11.
    fetchIpLocationOnce();
    return;
  } else {
    wifiConnectedSinceMs = 0;
    wifiLastStatusText = wifiStatusText(WiFi.status());
  }

  if (!hasActiveWifiCredentials()) {
    wifiLastFailure = "no-credentials";
    startWifiSetupPortal("first-setup");
    return;
  }

  unsigned long now = millis();

  if (wifiConnectStartedMs > 0 && now - wifiConnectStartedMs >= WIFI_CONNECT_TIMEOUT_MS) {
    wifiTimeoutCount++;
    wifiConnectStartedMs = 0;
    WiFi.disconnect(false, false);

    unsigned int allowedTimeouts = wifiConnectedAtLeastOnceThisBoot ? WIFI_PORTAL_AFTER_DROP_TIMEOUTS : WIFI_PORTAL_AFTER_BOOT_TIMEOUTS;
    bool allowAutoPortal = allowedTimeouts > 0;
    wifiLastFailure = wifiConnectedAtLeastOnceThisBoot ? "reconnect-timeout" : "connect-timeout";

    Serial.print("[WiFi] Connect timeout ");
    Serial.print(wifiTimeoutCount);
    Serial.print("/");
    if (allowAutoPortal) Serial.print(allowedTimeouts);
    else Serial.print("retry-forever");
    Serial.print(" | status=");
    Serial.print(wifiStatusText(WiFi.status()));
    Serial.print(" | scan=");
    Serial.println(wifiLastScanSummary);

    // OTA-safe boot: sau khi update từ xa, tuyệt đối không mắc kẹt vào setup portal chỉ vì RSSI yếu.
    // Cho thiết bị một khoảng grace để reconnect WiFi cũ và quay lại ThingsBoard.
#if OTA_SAFE_WIFI_BOOT_ENABLED
    if (otaSafeWifiBoot && millis() < otaSafeWifiUntilMs) {
      Serial.println("[WiFi] OTA-safe boot: suppressing auto portal after timeout; retrying saved WiFi.");
      return;
    }
#endif

    // Nếu đã từng online trong boot này, ưu tiên retry STA mãi để không làm mất cloud/MQTT.
    // Portal tự động chỉ dùng khi boot ở địa điểm mới và không tìm thấy WiFi đã lưu.
    if (!wifiConnectedAtLeastOnceThisBoot && wifiLastBestRssiDbm > -999 && wifiLastBestRssiDbm <= WIFI_WEAK_RSSI_PORTAL_DBM) {
      wifiLastFailure = "ssid-visible-but-weak-or-unreachable";
      Serial.println("[WiFi] Saved SSID is visible but weak/unreachable. Opening setup portal to choose another WiFi.");
      startWifiSetupPortal("ssid-visible-weak-timeout");
    } else if (allowAutoPortal && wifiTimeoutCount >= allowedTimeouts) {
      startWifiSetupPortal(wifiConnectedAtLeastOnceThisBoot ? "wifi-dropped" : "wifi-not-found");
    }
    return;
  }

  if (wifiConnectStartedMs > 0) return; // đang chờ kết quả kết nối, không gọi WiFi.begin lặp lại
  if (lastWifiRetryMs > 0 && now - lastWifiRetryMs < WIFI_RETRY_INTERVAL_MS) return;

  startWifiConnectionAttempt();
}

// ============================================================================
// MQTT / TELEMETRY / RPC
// ============================================================================
String buildMqttClientId() {
  String clientId = MQTT_CLIENT_ID_PREFIX;
  clientId += WiFi.macAddress();
  clientId.replace(":", "");
  return clientId;
}

void sendDeviceAttributes() {
  if (!mqttClient.connected()) return;

  String payload = "{";
  bool first = true;
  jsonAddString(payload, first, "deviceName", DEVICE_NAME);
  jsonAddString(payload, first, "deviceModel", DEVICE_MODEL);
  jsonAddString(payload, first, "firmwareVersion", FIRMWARE_VERSION);
  jsonAddString(payload, first, "ipAddress", WiFi.localIP().toString());
  jsonAddString(payload, first, "macAddress", WiFi.macAddress());
  jsonAddString(payload, first, "wifiConnectedSsid", WiFi.status() == WL_CONNECTED ? WiFi.SSID() : String(""));
  jsonAddString(payload, first, "wifiConnectedBssid", wifiConnectedBssidText());
  jsonAddString(payload, first, "wifiGateway", WiFi.status() == WL_CONNECTED ? WiFi.gatewayIP().toString() : String(""));
  jsonAddInt(payload, first, "wifiRssi", WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : -999);
  jsonAddString(payload, first, "wifiQualityText", wifiQualityText());
  jsonAddString(payload, first, "resetReason", resetReasonText(esp_reset_reason()));
  jsonAddBool(payload, first, "lightSensorEnabled", LIGHT_SENSOR_ENABLED);
  jsonAddInt(payload, first, "dhtPin", DHT_PIN);
  jsonAddInt(payload, first, "lightAoPin", LIGHT_AO_PIN);
  jsonAddInt(payload, first, "oledSafeWidth", OLED_SAFE_W);
  jsonAddBool(payload, first, "rpcEnabled", RPC_ENABLED);
  jsonAddString(payload, first, "rpcMethods", "getStatus,getTelemetry,muteAlarm,setAlarmProfile,setTelemetryInterval,openSetupPortal,openWifiSetup,wifiSetup,restart,updateFirmware");
  jsonAddBool(payload, first, "otaEnabled", OTA_ENABLED);
  jsonAddString(payload, first, "otaMode", "rpc-updateFirmware-https-url");
  jsonAddBool(payload, first, "wifiSetupPortalEnabled", WIFI_SETUP_PORTAL_ENABLED);
  jsonAddBool(payload, first, "wifiUsingStoredCredentials", wifiUsingStoredCredentials);
  jsonAddString(payload, first, "city", ipCity);
  jsonAddString(payload, first, "countryCode", ipCountryCode);
  jsonAddFloat(payload, first, "latitude", latitude, 6, !isnan(latitude));
  jsonAddFloat(payload, first, "longitude", longitude, 6, !isnan(longitude));
  jsonAddString(payload, first, "publicIp", publicIp);
  jsonAddString(payload, first, "locationSource", locationSource);
  jsonAddString(payload, first, "locationProvider", locationProvider);
  jsonAddString(payload, first, "locationFetchStatus", locationFetchStatus);
  jsonAddString(payload, first, "locationBasis", "public-ip-from-current-wifi");
  jsonAddString(payload, first, "locationConfidence", locationConfidenceText());
  jsonAddString(payload, first, "locationSummary", locationSummaryText());
  jsonAddString(payload, first, "locationWifiSsid", WiFi.status() == WL_CONNECTED ? WiFi.SSID() : String(""));
  jsonAddInt(payload, first, "locationLastSuccessSec", locationLastSuccessMs / 1000);
  payload += "}";

  bool ok = mqttClient.publish("v1/devices/me/attributes", payload.c_str());
  Serial.print("[TB] Attributes ");
  Serial.println(ok ? "sent." : "failed.");
}

String buildTelemetryPayload() {
  String payload = "{";
  bool first = true;

  jsonAddFloat(payload, first, "temperature", tempC, 1, sensorHasValidData);
  jsonAddFloat(payload, first, "humidity", humidity, 1, sensorHasValidData);
  jsonAddFloat(payload, first, "heatIndex", heatIndexC, 1, sensorHasValidData);
  jsonAddFloat(payload, first, "lightPercent", lightPercent, 1, lightHasData);
  jsonAddInt(payload, first, "lightRaw", lightRaw);
  jsonAddFloat(payload, first, "focusScore", focusScore, 1, sensorHasValidData && !isnan(focusScore));

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
  jsonAddString(payload, first, "lastRpcMethod", lastRpcMethod);
  jsonAddString(payload, first, "lastRpcResult", lastRpcResult);
  jsonAddInt(payload, first, "lastRpcAtSec", lastRpcAtMs / 1000);
  jsonAddInt(payload, first, "telemetryIntervalMs", telemetryIntervalMs);

  jsonAddString(payload, first, "otaStatus", otaStatus);
  jsonAddString(payload, first, "otaTargetVersion", otaTargetVersion);
  jsonAddString(payload, first, "otaLastError", otaLastError);
  jsonAddBool(payload, first, "otaPending", otaPending);
  jsonAddBool(payload, first, "otaInProgress", otaInProgress);

  jsonAddBool(payload, first, "mqttConnected", mqttClient.connected());
  jsonAddString(payload, first, "mqttStatus", lastMqttStatusText);
  jsonAddInt(payload, first, "uptimeSec", millis() / 1000);
  jsonAddInt(payload, first, "wifiRssi", WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : -999);
  jsonAddInt(payload, first, "wifiSignalQuality", wifiSignalQualityPercent());
  jsonAddString(payload, first, "wifiQualityText", wifiQualityText());
  jsonAddString(payload, first, "wifiStatus", WiFi.status() == WL_CONNECTED ? String("CONNECTED") : String(wifiLastStatusText));
  jsonAddString(payload, first, "wifiConnectedSsid", WiFi.status() == WL_CONNECTED ? WiFi.SSID() : String(""));
  jsonAddString(payload, first, "wifiConnectedBssid", wifiConnectedBssidText());
  jsonAddString(payload, first, "wifiGateway", WiFi.status() == WL_CONNECTED ? WiFi.gatewayIP().toString() : String(""));
  jsonAddInt(payload, first, "wifiBestScanRssi", wifiLastBestRssiDbm);
  jsonAddString(payload, first, "wifiLastScan", wifiLastScanSummary);
  jsonAddString(payload, first, "wifiSsid", WiFi.status() == WL_CONNECTED ? WiFi.SSID() : String(""));
  jsonAddString(payload, first, "wifiIp", WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String(""));
  jsonAddBool(payload, first, "wifiPortalActive", wifiPortalActive);
  jsonAddString(payload, first, "wifiPortalSsid", wifiPortalApSsid);
  jsonAddString(payload, first, "wifiPortalIp", wifiPortalActive ? WiFi.softAPIP().toString() : String(""));
  jsonAddInt(payload, first, "wifiPortalClients", wifiPortalActive ? WiFi.softAPgetStationNum() : 0);
  jsonAddString(payload, first, "wifiLastFailure", wifiLastFailure);
  jsonAddInt(payload, first, "telemetrySeq", telemetrySeq);
  jsonAddString(payload, first, "firmwareVersion", FIRMWARE_VERSION);
  jsonAddString(payload, first, "city", ipCity);
  jsonAddString(payload, first, "countryCode", ipCountryCode);
  jsonAddFloat(payload, first, "latitude", latitude, 6, !isnan(latitude));
  jsonAddFloat(payload, first, "longitude", longitude, 6, !isnan(longitude));
  jsonAddString(payload, first, "publicIp", publicIp);
  jsonAddString(payload, first, "locationSource", locationSource);
  jsonAddString(payload, first, "locationProvider", locationProvider);
  jsonAddString(payload, first, "locationFetchStatus", locationFetchStatus);
  jsonAddString(payload, first, "locationBasis", "public-ip-from-current-wifi");
  jsonAddString(payload, first, "locationConfidence", locationConfidenceText());
  jsonAddString(payload, first, "locationSummary", locationSummaryText());
  jsonAddString(payload, first, "locationWifiSsid", WiFi.status() == WL_CONNECTED ? WiFi.SSID() : String(""));
  jsonAddInt(payload, first, "locationLastSuccessSec", locationLastSuccessMs / 1000);
  jsonAddInt(payload, first, "freeHeap", ESP.getFreeHeap());

  payload += "}";
  return payload;
}

String buildStatusJson() {
  String payload = "{";
  bool first = true;
  jsonAddBool(payload, first, "ok", true);
  jsonAddString(payload, first, "state", getStateCode());
  jsonAddString(payload, first, "stateText", getStateText());
  jsonAddFloat(payload, first, "focusScore", focusScore, 1, sensorHasValidData && !isnan(focusScore));
  jsonAddBool(payload, first, "wifiConnected", WiFi.status() == WL_CONNECTED);
  jsonAddBool(payload, first, "mqttConnected", mqttClient.connected());
  jsonAddString(payload, first, "mqttStatus", lastMqttStatusText);
  jsonAddBool(payload, first, "portalActive", wifiPortalActive);
  jsonAddString(payload, first, "portalSsid", wifiPortalApSsid);
  jsonAddString(payload, first, "portalIp", wifiPortalActive ? WiFi.softAPIP().toString() : String(""));
  jsonAddInt(payload, first, "portalClients", wifiPortalActive ? WiFi.softAPgetStationNum() : 0);
  jsonAddString(payload, first, "firmwareVersion", FIRMWARE_VERSION);
  jsonAddString(payload, first, "ip", WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String(""));
  jsonAddInt(payload, first, "wifiRssi", WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : -999);
  jsonAddInt(payload, first, "wifiSignalQuality", wifiSignalQualityPercent());
  jsonAddString(payload, first, "wifiQualityText", wifiQualityText());
  jsonAddString(payload, first, "wifiStatus", WiFi.status() == WL_CONNECTED ? String("CONNECTED") : String(wifiLastStatusText));
  jsonAddString(payload, first, "wifiConnectedSsid", WiFi.status() == WL_CONNECTED ? WiFi.SSID() : String(""));
  jsonAddString(payload, first, "wifiConnectedBssid", wifiConnectedBssidText());
  jsonAddString(payload, first, "wifiGateway", WiFi.status() == WL_CONNECTED ? WiFi.gatewayIP().toString() : String(""));
  jsonAddInt(payload, first, "wifiBestScanRssi", wifiLastBestRssiDbm);
  jsonAddString(payload, first, "wifiLastScan", wifiLastScanSummary);
  jsonAddString(payload, first, "locationSource", locationSource);
  jsonAddString(payload, first, "locationProvider", locationProvider);
  jsonAddString(payload, first, "locationFetchStatus", locationFetchStatus);
  jsonAddString(payload, first, "locationBasis", "public-ip-from-current-wifi");
  jsonAddString(payload, first, "locationConfidence", locationConfidenceText());
  jsonAddString(payload, first, "locationSummary", locationSummaryText());
  jsonAddFloat(payload, first, "latitude", latitude, 6, !isnan(latitude));
  jsonAddFloat(payload, first, "longitude", longitude, 6, !isnan(longitude));
  payload += "}";
  return payload;
}

void sendTelemetryNow(const char* reason) {
  if (!mqttClient.connected()) return;
  telemetrySeq++;
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

void markRpcResult(const char* method, const char* result) {
  lastRpcMethod = String(method ? method : "none");
  lastRpcResult = String(result ? result : "none");
  lastRpcAtMs = millis();
}

String extractRpcRequestId(const String &topic) {
  int slash = topic.lastIndexOf('/');
  if (slash < 0 || slash >= (int)topic.length() - 1) return "0";
  return topic.substring(slash + 1);
}

long extractFirstNumber(const String &text, long fallbackValue) {
  String digits = "";
  bool started = false;
  for (int i = 0; i < (int)text.length(); i++) {
    char c = text.charAt(i);
    if ((c == '-' && !started) || isdigit(c)) {
      digits += c;
      started = true;
    } else if (started) break;
  }
  if (digits.length() == 0 || digits == "-") return fallbackValue;
  return digits.toInt();
}

String stripOuterQuotes(String v) {
  v.trim();
  if (v.length() >= 2) {
    char a = v.charAt(0);
    char b = v.charAt(v.length() - 1);
    if ((a == '"' && b == '"') || (a == '\'' && b == '\'')) {
      v = v.substring(1, v.length() - 1);
      v.trim();
    }
  }
  return v;
}

String extractUrlFromRpc(const String &message, const String &paramsRaw) {
  // Compatible with old and new ThingsBoard RPC widgets.
  // Accepted payloads:
  //   {"method":"updateFirmware","params":{"url":"https://...bin"}}
  //   {"method":"updateFirmware","params":"https://...bin"}
  //   {"method":"updateFirmware","params":{"firmwareUrl":"https://...bin"}}
  const char* keys[] = {"url", "firmwareUrl", "firmware_url", "fwUrl", "fw_url", "otaUrl", "ota_url", "downloadUrl", "download_url", "binUrl", "bin_url"};
  for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
    String url = extractJsonStringValue(message, keys[i]);
    url = stripOuterQuotes(url);
    if (url.startsWith("http://") || url.startsWith("https://")) return url;
  }
  for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
    String url = extractJsonStringValue(paramsRaw, keys[i]);
    url = stripOuterQuotes(url);
    if (url.startsWith("http://") || url.startsWith("https://")) return url;
  }
  String p = stripOuterQuotes(paramsRaw);
  if (p.startsWith("http://") || p.startsWith("https://")) return p;
  return "";
}

void publishRpcResponse(const String &requestId, const String &payload) {
  if (!mqttClient.connected()) return;
  String topic = "v1/devices/me/rpc/response/";
  topic += requestId;
  bool ok = mqttClient.publish(topic.c_str(), payload.c_str());
  Serial.print("[RPC] Response ");
  Serial.println(ok ? "sent." : "failed.");
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
#if RPC_ENABLED
  String topicStr = String(topic);
  String message = "";
  message.reserve(length + 1);
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];

  String requestId = extractRpcRequestId(topicStr);
  String method = extractJsonStringValue(message, "method");
  String paramsRaw = extractJsonRawValue(message, "params");

  Serial.print("[RPC] Method="); Serial.print(method);
  Serial.print(" Params="); Serial.println(paramsRaw);

  if (method == "getStatus") {
    markRpcResult("getStatus", "ok");
    publishRpcResponse(requestId, buildStatusJson());
    return;
  }

  if (method == "getTelemetry") {
    sendTelemetryNow("rpc-getTelemetry");
    markRpcResult("getTelemetry", "sent");
    publishRpcResponse(requestId, "{\"success\":true,\"message\":\"telemetry sent\"}");
    return;
  }

  if (method == "openSetupPortal" || method == "openWifiSetup" || method == "wifiSetup") {
#if WIFI_SETUP_PORTAL_ENABLED
    markRpcResult(method.c_str(), "setup-portal-opening");
    publishRpcResponse(requestId, "{\"success\":true,\"message\":\"HP12-SETUP portal is opening. Connect phone to HP12-SETUP AP and open http://192.168.4.1. Old WiFi/token are preserved until a new WiFi is saved.\"}");
    Serial.println("[RPC] Opening HP12-SETUP portal by cloud command. Old WiFi and ThingsBoard token are preserved until new WiFi is saved.");
    openWifiSetupPortalAndClearWifi("rpc-open-setup-portal");
#else
    markRpcResult(method.c_str(), "portal-disabled");
    publishRpcResponse(requestId, "{\"success\":false,\"message\":\"WiFi setup portal disabled in config\"}");
#endif
    return;
  }

  if (method == "restart") {
    scheduledRestartAtMs = millis() + RPC_RESTART_DELAY_MS;
    markRpcResult("restart", "scheduled");
    publishRpcResponse(requestId, "{\"success\":true,\"message\":\"restart scheduled\"}");
    return;
  }

  if (method == "muteAlarm") {
    String p = paramsRaw; p.toLowerCase();
    bool mute = true;
    if (p.indexOf("false") >= 0 || p.indexOf("off") >= 0 || p.indexOf("unmute") >= 0) mute = false;
    if (p.indexOf("toggle") >= 0) mute = !alarmIsMuted();
    alarmMutedUntilMs = mute ? millis() + ALARM_MUTE_MS : 0;
    markRpcResult("muteAlarm", mute ? "muted" : "unmuted");
    publishRpcResponse(requestId, mute ? "{\"success\":true,\"alarmMuted\":true}" : "{\"success\":true,\"alarmMuted\":false}");
    return;
  }

  if (method == "setAlarmProfile") {
    String p = paramsRaw; p.toLowerCase();
    long n = extractFirstNumber(p, -1);
    bool ok = true;
    if (p.indexOf("quiet") >= 0 || p.indexOf("im") >= 0 || n == 0) alarmProfile = ALARM_QUIET;
    else if (p.indexOf("strong") >= 0 || p.indexOf("manh") >= 0 || n == 2) alarmProfile = ALARM_STRONG;
    else if (p.indexOf("normal") >= 0 || p.indexOf("vua") >= 0 || n == 1) alarmProfile = ALARM_NORMAL;
    else ok = false;

    if (ok) {
      markRpcResult("setAlarmProfile", getAlarmProfileText());
      String resp = "{\"success\":true,\"alarmProfile\":\"" + String(getAlarmProfileText()) + "\"}";
      publishRpcResponse(requestId, resp);
    } else {
      markRpcResult("setAlarmProfile", "bad-profile");
      publishRpcResponse(requestId, "{\"success\":false,\"message\":\"invalid alarm profile. Use 0/1/2 or IM/VUA/MANH\"}");
    }
    return;
  }

  if (method == "setTelemetryInterval") {
    long v = extractFirstNumber(paramsRaw, TELEMETRY_INTERVAL_MS);
    v = clampLong(v, TELEMETRY_INTERVAL_MIN_MS, TELEMETRY_INTERVAL_MAX_MS);
    telemetryIntervalMs = (unsigned long)v;
    markRpcResult("setTelemetryInterval", "ok");
    String resp = "{\"success\":true,\"telemetryIntervalMs\":" + String(telemetryIntervalMs) + "}";
    publishRpcResponse(requestId, resp);
    return;
  }

  if (method == "updateFirmware" || method == "firmwareUpdate" || method == "otaUpdate" || method == "startOta" || method == "ota") {
    if (otaPending || otaInProgress) {
      markRpcResult("updateFirmware", "busy");
      publishRpcResponse(requestId, "{\"success\":false,\"message\":\"OTA already pending or in progress\"}");
      return;
    }

    String url = extractUrlFromRpc(message, paramsRaw);
    String target = extractJsonStringValue(message, "version");
    if (target.length() == 0) target = extractJsonStringValue(paramsRaw, "version");
    if (target.length() == 0) target = "Latest";

#if OTA_REQUIRE_HTTPS
    if (!url.startsWith("https://")) {
      markRpcResult("updateFirmware", "bad-url");
      publishRpcResponse(requestId, "{\"success\":false,\"message\":\"missing or invalid firmware url. Use HTTPS url\"}");
      return;
    }
#else
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
      markRpcResult("updateFirmware", "bad-url");
      publishRpcResponse(requestId, "{\"success\":false,\"message\":\"missing or invalid firmware url\"}");
      return;
    }
#endif

    if (target == FIRMWARE_VERSION) {
      markRpcResult("updateFirmware", "same-version");
      publishRpcResponse(requestId, "{\"success\":false,\"message\":\"device already runs this firmware version\"}");
      return;
    }

    otaPending = true;
    otaPendingUrl = url;
    otaTargetVersion = target;
    otaStatus = "queued";
    otaLastError = "";
    markRpcResult("updateFirmware", "ota-queued");
    publishRpcResponse(requestId, "{\"success\":true,\"message\":\"OTA queued\"}");
    return;
  }

  markRpcResult(method.c_str(), "unknown-method");
  String resp = "{\"success\":false,\"message\":\"unknown RPC method: " + jsonEscape(method) + "\"}";
  publishRpcResponse(requestId, resp);
#else
  (void)topic; (void)payload; (void)length;
#endif
}

void updateMqttNonBlocking() {
  if (WiFi.status() != WL_CONNECTED || wifiPortalActive) {
    if (mqttClient.connected()) mqttClient.disconnect();
    rpcSubscribed = false;
    lastMqttStatusText = wifiPortalActive ? "portal-active" : "wifi-not-connected";
    mqttFirstAttemptAfterWifi = false;
    return;
  }

  if (activeTbTokenSource.length() == 0 || activeTbTokenSource == "none") loadThingsBoardToken();
  if (!isNonEmptyTokenString(activeTbToken)) {
    lastMqttStatusText = "token-missing";
    printMqttTokenHint(false);
    return;
  }

  if (mqttClient.connected()) {
    lastMqttStatusText = "connected";
    return;
  }

  unsigned long now = millis();
  bool firstAttempt = !mqttFirstAttemptAfterWifi;
  if (!firstAttempt && now - lastMqttRetryMs < MQTT_RETRY_INTERVAL_MS) return;

  mqttFirstAttemptAfterWifi = true;
  lastMqttRetryMs = now;

  String clientId = buildMqttClientId();
  lastMqttStatusText = "connecting";
  Serial.print("[MQTT] Connecting to ");
  Serial.print(TB_HOST);
  Serial.print(":");
  Serial.print(TB_PORT);
  Serial.print(" | clientId=");
  Serial.print(clientId);
  Serial.print(" | RSSI=");
  Serial.print(WiFi.RSSI());
  Serial.print(" | tokenLength=");
  Serial.print(activeTbToken.length());
  Serial.print(" | tokenSource=");
  Serial.println(activeTbTokenSource);

  bool ok = mqttClient.connect(clientId.c_str(), activeTbToken.c_str(), nullptr);
  if (ok) {
    lastMqttStatusText = "connected";
    Serial.println("[MQTT] Connected to ThingsBoard. Device should become ACTIVE now.");
    rpcSubscribed = false;
#if RPC_ENABLED
    rpcSubscribed = mqttClient.subscribe("v1/devices/me/rpc/request/+");
    Serial.print("[RPC] Subscribe ");
    Serial.println(rpcSubscribed ? "ok." : "failed.");
#endif
    attributesSentOnce = false;
    forceFirstTelemetry = FORCE_FIRST_SYNC_ENABLED;
  } else {
    int rc = mqttClient.state();
    lastMqttStatusText = String("failed-") + String(rc) + "-" + mqttStateText(rc);
    Serial.print("[MQTT] Failed, rc=");
    Serial.print(rc);
    Serial.print(" (");
    Serial.print(mqttStateText(rc));
    Serial.println(")");

    if (rc == MQTT_CONNECT_UNAUTHORIZED || rc == MQTT_CONNECT_BAD_CREDENTIALS) {
      Serial.println("[MQTT] CHECK: token in secrets.h does not match the ThingsBoard device HP12 access token, or the device was deleted/recreated.");
    } else if (rc == MQTT_CONNECTION_TIMEOUT || rc == MQTT_CONNECT_FAILED) {
      Serial.println("[MQTT] CHECK: network/router may block outbound MQTT port 1883, or TB_HOST/TB_PORT is unreachable.");
    }
  }
}

void sendTelemetryNonBlocking() {
  if (!mqttClient.connected()) return;

  unsigned long now = millis();
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
  if (wifiPortalActive) return;

  updateWiFiNonBlocking();
  updateMqttNonBlocking();

  if (mqttClient.connected()) mqttClient.loop();
  sendTelemetryNonBlocking();
}

// ============================================================================
// OTA
// ============================================================================
void setOtaError(const String &err) {
  otaStatus = "error";
  otaLastError = err;
  otaInProgress = false;
  otaPending = false;
  otaVisualHoldUntilMs = millis() + OTA_OLED_ERROR_HOLD_MS;
  markRpcResult("updateFirmware", "error");
  Serial.print("[OTA] ERROR: ");
  Serial.println(err);
  drawOtaScreenNow("OTA bi loi", err.c_str(), "Kiem tra URL .bin");
  sendTelemetryNow("ota-error");
}

bool otaIsRedirectCode(int code) {
  // Use numeric codes to stay compatible across ESP32 core/HTTPClient versions.
  return code == 301 || code == 302 || code == 303 || code == 307 || code == 308;
}

String otaRootFromUrl(const String &url) {
  int schemeEnd = url.indexOf("://");
  if (schemeEnd < 0) return "";
  int hostStart = schemeEnd + 3;
  int pathStart = url.indexOf('/', hostStart);
  if (pathStart < 0) return url;
  return url.substring(0, pathStart);
}

String otaBasePathFromUrl(const String &url) {
  int q = url.indexOf('?');
  String clean = (q >= 0) ? url.substring(0, q) : url;
  int slash = clean.lastIndexOf('/');
  if (slash < 0) return otaRootFromUrl(url) + "/";
  return clean.substring(0, slash + 1);
}

String otaResolveRedirectUrl(const String &baseUrl, String location) {
  location.trim();
  if (location.length() == 0) return "";
  if (location.startsWith("http://") || location.startsWith("https://")) return location;
  if (location.startsWith("//")) return String("https:") + location;
  if (location.startsWith("/")) return otaRootFromUrl(baseUrl) + location;
  return otaBasePathFromUrl(baseUrl) + location;
}

bool otaBeginHttp(HTTPClient &http, WiFiClient &plainClient, WiFiClientSecure &secureClient, const String &url) {
  http.setTimeout(OTA_HTTP_TIMEOUT_MS);
  http.setReuse(false);
  http.setUserAgent(String(DEVICE_NAME) + "/" + String(FIRMWARE_VERSION) + " ESP32-OTA");
  const char *headerKeys[] = {"Location", "Content-Type", "Content-Length"};
  http.collectHeaders(headerKeys, 3);

  if (url.startsWith("https://")) {
#if OTA_ALLOW_INSECURE_TLS
    secureClient.setInsecure();
#endif
    return http.begin(secureClient, url);
  }

#if OTA_REQUIRE_HTTPS
  return false;
#else
  if (url.startsWith("http://")) return http.begin(plainClient, url);
  return false;
#endif
}

bool performOtaUpdate(const String &url) {
#if OTA_ENABLED
  if (WiFi.status() != WL_CONNECTED) {
    setOtaError("wifi-not-connected");
    return false;
  }

  String currentUrl = stripOuterQuotes(url);
  if (!currentUrl.startsWith("http://") && !currentUrl.startsWith("https://")) {
    setOtaError("bad-url");
    return false;
  }

#if OTA_REQUIRE_HTTPS
  if (!currentUrl.startsWith("https://")) {
    setOtaError("https-required");
    return false;
  }
#endif

  otaPending = false;
  otaInProgress = true;
  otaStatus = "downloading";
  otaLastError = "";
  Serial.println("[OTA] Starting firmware update...");
  Serial.print("[OTA] URL: ");
  Serial.println(currentUrl);
  drawOtaScreenNow("FIRMWARE OTA", "Dang tai firmware", "!! KHONG NGAT DIEN !!");
  sendTelemetryNow("ota-start");

  HTTPClient http;
  WiFiClient plainClient;
  WiFiClientSecure secureClient;
  int httpCode = -1;
  bool readyToDownload = false;

  for (int redirectCount = 0; redirectCount <= OTA_REDIRECT_LIMIT; redirectCount++) {
    if (!otaBeginHttp(http, plainClient, secureClient, currentUrl)) {
      setOtaError(currentUrl.startsWith("http://") ? "https-required-or-http-begin-failed" : "http-begin-failed");
      return false;
    }

    Serial.print("[OTA] HTTP GET: ");
    Serial.println(currentUrl);
    httpCode = http.GET();
    Serial.print("[OTA] HTTP code: ");
    Serial.println(httpCode);

    if (httpCode == HTTP_CODE_OK) {
      readyToDownload = true;
      break;
    }

    if (otaIsRedirectCode(httpCode)) {
      String location = http.header("Location");
      if (location.length() == 0) {
        // Some ESP32 HTTPClient versions expose the redirect URL via Location only after collectHeaders.
        http.end();
        setOtaError("redirect-without-location:" + String(httpCode));
        return false;
      }
      String nextUrl = otaResolveRedirectUrl(currentUrl, location);
      Serial.print("[OTA] Redirect ");
      Serial.print(httpCode);
      Serial.print(" -> ");
      Serial.println(nextUrl);
      http.end();
      if (nextUrl.length() == 0 || nextUrl == currentUrl) {
        setOtaError("bad-redirect-url");
        return false;
      }
      currentUrl = nextUrl;
      continue;
    }

    String err = "http-code-" + String(httpCode);
    http.end();
    setOtaError(err);
    return false;
  }

  if (!readyToDownload) {
    http.end();
    setOtaError("too-many-redirects");
    return false;
  }

  String contentType = http.header("Content-Type");
  int contentLength = http.getSize();
  Serial.print("[OTA] Final URL: ");
  Serial.println(currentUrl);
  Serial.print("[OTA] Content-Type: ");
  if (contentType.length()) Serial.println(contentType); else Serial.println("unknown");
  Serial.print("[OTA] Content-Length: ");
  Serial.println(contentLength);

  if (contentType.indexOf("text/html") >= 0) {
    http.end();
    setOtaError("url-returned-html-not-bin");
    return false;
  }

  otaStatus = "writing";
  drawOtaScreenNow("FIRMWARE OTA", "Dang ghi firmware", "!! KHONG NGAT DIEN !!");

  bool canBegin = Update.begin(contentLength > 0 ? contentLength : UPDATE_SIZE_UNKNOWN);
  if (!canBegin) {
    String err = "update-begin-failed:" + String(Update.getError());
    http.end();
    setOtaError(err);
    return false;
  }

  WiFiClient *stream = http.getStreamPtr();
  size_t written = Update.writeStream(*stream);
  Serial.print("[OTA] Written: ");
  Serial.println(written);

  if (contentLength > 0 && written != (size_t)contentLength) {
    String err = "size-mismatch:" + String(written) + "/" + String(contentLength);
    http.end();
    Update.abort();
    setOtaError(err);
    return false;
  }

  if (!Update.end(true)) {
    String err = "update-end-failed:" + String(Update.getError());
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
  otaInProgress = false;
  otaStatus = "success";
  markRpcResult("updateFirmware", "success");
  Serial.println("[OTA] Update success. Restarting...");
  markOtaSafeWifiBootNext();
  drawOtaScreenNow("Cap nhat thanh cong", otaTargetVersion.c_str(), "Dang reset HP12...");
  sendTelemetryNow("ota-success");
  delay(OTA_OLED_SUCCESS_HOLD_MS);
  ESP.restart();
  return true;
#else
  setOtaError("ota-disabled");
  (void)url;
  return false;
#endif
}

void updateOtaNonBlocking() {
  if (!otaPending || otaInProgress) return;
  performOtaUpdate(otaPendingUrl);
}



bool parseWifiSerialCommand(const String &cmdIn, String &ssid, String &pass) {
  String cmd = cmdIn;
  cmd.trim();
  int eq = cmd.indexOf('=');
  if (eq < 0) eq = cmd.indexOf(':');
  if (eq < 0) {
    int sp = cmd.indexOf(' ');
    if (sp > 0) eq = sp;
  }
  if (eq < 0) return false;

  String head = cmd.substring(0, eq);
  head.trim();
  head.toLowerCase();
  if (!(head == "setwifi" || head == "wifi" || head == "savewifi" || head == "setwifi2g")) return false;

  String body = cmd.substring(eq + 1);
  body.trim();
  int sep = body.indexOf('|');
  if (sep < 0) sep = body.indexOf(',');
  if (sep < 0) sep = body.indexOf(';');
  if (sep < 0) {
    ssid = body;
    pass = "";
  } else {
    ssid = body.substring(0, sep);
    pass = body.substring(sep + 1);
  }
  ssid.trim();
  pass.trim();
  if ((ssid.startsWith("\"") && ssid.endsWith("\"")) || (ssid.startsWith("'") && ssid.endsWith("'"))) ssid = ssid.substring(1, ssid.length() - 1);
  if ((pass.startsWith("\"") && pass.endsWith("\"")) || (pass.startsWith("'") && pass.endsWith("'"))) pass = pass.substring(1, pass.length() - 1);
  return ssid.length() > 0;
}

void saveWifiFromSerialAndRestart(const String &ssid, const String &pass) {
  if (ssid.length() == 0) {
    Serial.println("[SERIAL] SETWIFI failed: SSID empty.");
    return;
  }
  if (pass.length() > 0 && pass.length() < 8) {
    Serial.println("[SERIAL] SETWIFI failed: WPA/WPA2 password must be empty/open or at least 8 chars.");
    return;
  }
  Serial.print("[SERIAL] Saving WiFi from Serial. SSID=");
  Serial.print(ssid);
  Serial.print(" | pass=");
  Serial.println(pass.length() > 0 ? "WPA/WPA2" : "OPEN");
  saveWifiCredentials(ssid, pass);
  wifiLastFailure = "serial-setwifi";
  Serial.println("[SERIAL] WiFi saved. HP12 will reboot and connect to the new WiFi. ThingsBoard token is kept.");
  scheduledRestartAtMs = millis() + 800;
}

void updateSerialCommandsNonBlocking() {
#if WIFI_SERIAL_COMMAND_ENABLED
  static String line;
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      line.trim();
      String rawCmd = line;
      String cmd = line;
      cmd.trim();
      String cmdLower = cmd;
      cmdLower.toLowerCase();
      line = "";
      if (cmd.length() == 0) return;

      String newSsid, newPass;
      if (parseWifiSerialCommand(rawCmd, newSsid, newPass)) {
        saveWifiFromSerialAndRestart(newSsid, newPass);
      } else if (cmdLower == "setup" || cmdLower == "portal" || cmdLower == "wifi" || cmdLower == "resetwifi" || cmdLower == "reset_wifi") {
        Serial.println("[SERIAL] SETUP command received. Opening HP12-SETUP; preserving old WiFi until new save succeeds.");
        openWifiSetupPortalAndClearWifi("serial-setup-command");
      } else if (cmdLower == "status") {
        Serial.print("[SERIAL] WiFi="); Serial.print(wifiStatusText(WiFi.status()));
        Serial.print(" | portal="); Serial.print(wifiPortalActive ? "YES" : "NO");
        Serial.print(" | savedSSID="); Serial.print(activeWifiSsid);
        Serial.print(" | MQTT="); Serial.print(mqttClient.connected() ? "CONNECTED" : "OFFLINE");
        Serial.print(" | token="); Serial.println(tokenPreview(activeTbToken));
      } else if (cmdLower == "reboot" || cmdLower == "restart") {
        Serial.println("[SERIAL] Reboot command received.");
        scheduledRestartAtMs = millis() + 300;
      } else {
        Serial.print("[SERIAL] Unknown command: "); Serial.println(cmd);
        Serial.println("[SERIAL] Commands: SETUP, STATUS, REBOOT, SETWIFI=SSID|PASSWORD");
      }
      return;
    }
    if (line.length() < 180) line += c;
  }
#endif
}

void updateScheduledRestartNonBlocking() {
  if (scheduledRestartAtMs > 0 && millis() >= scheduledRestartAtMs) {
    Serial.println("[SYS] Restarting...");
    delay(80);
    ESP.restart();
  }
}

// ============================================================================
// SETUP / LOOP
// ============================================================================
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

  allOutputsOff();

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  applyWifiStabilityTuning();
  WiFi.setAutoReconnect(true);
  initDefaultLocation();
  loadWifiCredentials();
  consumeOtaSafeWifiBootFlag();

  mqttClient.setServer(TB_HOST, TB_PORT);
  mqttClient.setCallback(onMqttMessage);
  mqttClient.setBufferSize(MQTT_BUFFER_SIZE);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(8);

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
  Serial.println(" WIFI SETUP PORTAL started.");
  Serial.print("DHT PIN: GPIO"); Serial.println(DHT_PIN);
  Serial.print("LIGHT AO PIN: GPIO"); Serial.println(LIGHT_AO_PIN);
  Serial.print("NAV BUTTON: GPIO"); Serial.println(BUTTON_NAV_PIN);
  Serial.print("ADVICE BUTTON: GPIO"); Serial.println(BUTTON_ADVICE_PIN);
  Serial.print("BLUE LED: GPIO"); Serial.println(ONBOARD_BLUE_LED_PIN);
  Serial.println("Blue LED self-test enabled.");
  Serial.println("Basis: Temp + RH + Heat Index + relative light; proxy only.");
  Serial.println("Cloud: WiFi + MQTT ThingsBoard non-blocking, anti-spam telemetry.");
  Serial.println("WiFi setup: Preferences memory + HP12-SETUP portal if boot WiFi fails.");
  Serial.println("Hold NAV + ADVICE for 5.5s to open HP12-SETUP; single-button fallback is disabled to protect OLED button UX.");
  Serial.println("Serial command: type SETUP then Enter to force HP12-SETUP from Arduino Serial Monitor.");
  Serial.println("WiFi fix: stop STA before AP portal; after online, retry STA forever on drops.");
  Serial.println("LOC: dynamic IP-location enabled; retries across providers, refreshes on WiFi change.");
  Serial.println("WiFi hardening: VN country ch1-13, scan-before-connect, fast portal if saved SSID is absent.");
  Serial.println("MQTT fix: restored ThingsBoard credential path; immediate connect after WiFi; explicit token/rc diagnostics.");
  Serial.println("OTA fix retained: RPC OTA restored + OTA-safe WiFi boot after remote update; GitHub redirects supported.");
  Serial.println("OTA-safe WiFi: after OTA reboot, suppress auto setup portal during grace window; try saved WiFi first even if RSSI is weak.");
  Serial.println("Credential fix: token can come from Preferences or local secrets.h; no factory token embedded.");
  Serial.println("WiFi UX v1.13.19: OLED shows connected SSID/RSSI/IP/location status; portal explains IP-location basis.");
  Serial.println("Serial fallback: type SETWIFI=YourSSID|YourPassword then Enter to change WiFi without portal.");
  Serial.println("Button UX fix v1.13.17: short NAV/ADVICE clicks work again; setup hold no longer consumes normal OLED tab control.");
  printMqttTokenHint(true);
  Serial.println("====================================");
}

void loop() {
  updateSerialCommandsNonBlocking();
  updateWifiSetupTriggerNonBlocking();
  if (!wifiPortalActive) updateButtonsNonBlocking();

  readSensorNonBlocking();
  readLightNonBlocking();
  updateOledNonBlocking();
  updateEnvironmentLedAndBuzzerNonBlocking();
  updateBlueLedNonBlocking();
  updateCloudNonBlocking();
  updateOtaNonBlocking();
  updateScheduledRestartNonBlocking();

  // Không delay dài. Không while chặn.
}
