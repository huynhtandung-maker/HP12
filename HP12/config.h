#pragma once

/*
  HP12 - CONFIG

  Du an: Thiet bi danh gia tien nghi khong gian lam viec
  Board: ESP32 30 chan / ESP32-U
  Sensor: DHT22 + LDR LM393
  Display: OLED 0.96 inch SSD1306 I2C

  HP12 v1.5.0:
  - UI lam lai theo huong "dashboard toi gian"
  - Chart chuyen thanh line-gauge ngang: khong so tren than chart, khong nhieu vach gay roi
  - Moi tab chi tap trung 1 y chinh
  - Nut ADVICE rieng: hien loi khuyen dang cuon, khong tran dong
  - Onboard blue LED co self-test 8 giay de de kiem tra
  - Code chu giai ro de de bao tri
*/

// =========================
// FIRMWARE INFO
// =========================

#define DEVICE_NAME        "HP12"
#define DEVICE_MODEL       "ESP32_HP12"
#define FIRMWARE_VERSION   "v1.8.0"

// =========================
// PIN CONFIG
// =========================

// DHT22
#define DHT_PIN            14
#define DHT_TYPE           DHT22

// OLED SSD1306 I2C
#define OLED_SDA_PIN       21
#define OLED_SCL_PIN       22
#define OLED_WIDTH         128
#define OLED_HEIGHT        64
#define OLED_ADDR          0x3C
#define OLED_RESET         -1

// LDR LM393: VCC->3V3, GND->GND, AO->GPIO34, DO->GPIO35 optional
#define LIGHT_SENSOR_ENABLED       1
#define LIGHT_AO_PIN               34
#define LIGHT_DO_PIN               35
#define LIGHT_USE_DIGITAL_DO       0

// Ban da test: de 0 la dung voi module cua ban.
#define LIGHT_ADC_BRIGHT_IS_HIGH   0

// LDR la muc sang tuong doi, KHONG phai lux chuan.
#define LIGHT_DARK_PCT             25.0
#define LIGHT_FOCUS_MIN_PCT        35.0
#define LIGHT_FOCUS_MAX_PCT        78.0
#define LIGHT_GLARE_PCT            88.0

// Button NAV: doi cac tab du lieu
#define BUTTON_NAV_PIN             27
#define BUTTON_NAV_ACTIVE_LOW      1

// Button ADVICE: vao/cycle loi khuyen
// Dau noi: 1 chan nut -> GPIO13, chan con lai -> GND
#define BUTTON_ADVICE_PIN          13
#define BUTTON_ADVICE_ACTIVE_LOW   1

// Buzzer
#define BUZZER_PIN                 26
#define BUZZER_ACTIVE_LOW          0

// 3 LED moi truong
#define LED_GREEN_PIN              25
#define LED_YELLOW_PIN             32
#define LED_RED_PIN                33
#define LED_ACTIVE_LOW             1

// Onboard blue LED tren nhieu board ESP32 la GPIO2.
// Neu 8 giay dau khong thay LED nhay, thu doi ACTIVE_LOW = 1.
// Neu van khong thay, board co the khong co LED dieu khien duoc -> ENABLED = 0.
#define ONBOARD_BLUE_LED_ENABLED    1
#define ONBOARD_BLUE_LED_PIN        2
#define ONBOARD_BLUE_LED_ACTIVE_LOW 0
#define BLUE_LED_SELF_TEST_MS       8000UL

// =========================
// TIMING CONFIG
// =========================

#define SENSOR_READ_INTERVAL_MS      2000UL
#define LIGHT_READ_INTERVAL_MS       500UL
#define OLED_UPDATE_INTERVAL_MS      160UL
#define LED_UPDATE_INTERVAL_MS       80UL
#define BLUE_LED_UPDATE_INTERVAL_MS  60UL
#define SCROLL_UPDATE_INTERVAL_MS    150UL

// Button UX
#define BUTTON_DEBOUNCE_MS           35UL
#define BUTTON_CLICK_WINDOW_MS       450UL
#define BUTTON_LONG_PRESS_MS         2500UL
#define HELP_PAGE_DURATION_MS        8000UL

// Alarm mute
#define ALARM_MUTE_MS                600000UL   // 10 phut

// Du phong cho giai doan ThingsBoard
#define WIFI_RETRY_INTERVAL_MS       5000UL
#define MQTT_RETRY_INTERVAL_MS       10000UL
#define TELEMETRY_INTERVAL_MS        30000UL

// =========================
// THINGSBOARD / CLOUD CONFIG
// =========================
//
// Force first sync: gui ngay 1 goi dau tien sau khi WiFi + MQTT ket noi.
// Anti-spam: sau goi dau, telemetry chi gui theo TELEMETRY_INTERVAL_MS.
#define FORCE_FIRST_SYNC_ENABLED      1
#define ATTRIBUTES_RESEND_MS          300000UL   // gui lai attributes moi 5 phut
#define MQTT_CLIENT_ID_PREFIX         "HP12_"
#define MQTT_BUFFER_SIZE              1024

// =========================
// RPC CONTROL CONFIG
// =========================
//
// v1.7.0 chi them RPC an toan truoc.
// OTA firmware update se lam rieng o v1.8.0 de tranh rui ro.
#define RPC_ENABLED                   1
#define RPC_RESTART_DELAY_MS          1200UL
#define TELEMETRY_INTERVAL_MIN_MS     15000UL
#define TELEMETRY_INTERVAL_MAX_MS     600000UL

// =========================
// COMFORT THRESHOLD
// Ban thuc dung cho phong lam viec khi hau nong am
// =========================

// Vung ly tuong de lam viec sau
#define TEMP_GOOD_MIN      24.0
#define TEMP_GOOD_MAX      27.5

#define HUM_GOOD_MIN       40.0
#define HUM_GOOD_MAX       65.0

// Bat dau lech vung tap trung
#define TEMP_WARN          29.0
#define HUM_WARN           70.0
#define HEAT_INDEX_WARN    31.0

// Nguong kho chiu / can hanh dong cua HP12 cho lam viec tri oc
#define TEMP_DANGER        32.5
#define HUM_DANGER         80.0
#define HEAT_INDEX_DANGER  36.0

// Moc tham chieu Heat Index theo vung rui ro nhiet/thoi tiet.
// HP12 canh bao som hon vi muc tieu la tap trung lam viec.
#define HEAT_INDEX_MED_CAUTION_C     32.0
#define HEAT_INDEX_MED_DANGER_C      39.0
#define HEAT_INDEX_MED_EXTREME_C     51.0

// Loc du lieu loi
#define SENSOR_TEMP_MIN    -10.0
#define SENSOR_TEMP_MAX    60.0
#define SENSOR_HUM_MIN     0.0
#define SENSOR_HUM_MAX     100.0
#define SENSOR_HUM_SUSPECT 99.9

// =========================
// BUZZER PROFILE
// =========================

#define BUZZER_PROFILE_START         1

#define BUZZER_WARN_ENABLED          0
#define BUZZER_WARN_PERIOD_MS        15000UL
#define BUZZER_WARN_ON_MS            45UL

#define BUZZER_NORMAL_PERIOD_MS      6000UL
#define BUZZER_NORMAL_ON_MS          70UL
#define BUZZER_NORMAL_GAP_MS         180UL
#define BUZZER_NORMAL_COUNT          2

#define BUZZER_STRONG_PERIOD_MS      3000UL
#define BUZZER_STRONG_ON_MS          80UL
#define BUZZER_STRONG_GAP_MS         160UL
#define BUZZER_STRONG_COUNT          3

// =========================
// LED PATTERN
// =========================

#define LED_WARN_BLINK_MS            700UL
#define LED_DANGER_BLINK_MS          250UL
#define LED_SENSOR_BLINK_MS          1000UL
#define LED_START_BLINK_MS           900UL

// Onboard blue LED heartbeat
#define BLUE_LED_ALIVE_PERIOD_MS     3500UL
#define BLUE_LED_PULSE_ON_MS         120UL
#define BLUE_LED_PULSE_GAP_MS        180UL

// =========================
// LINE-GAUGE SCALE
// =========================

#define HEAT_BAR_MIN_C               24.0
#define HEAT_BAR_MAX_C               52.0

#define TEMP_BAR_MIN_C               18.0
#define TEMP_BAR_MAX_C               38.0

#define HUM_BAR_MIN_PCT              20.0
#define HUM_BAR_MAX_PCT              100.0

#define LIGHT_BAR_MIN_PCT            0.0
#define LIGHT_BAR_MAX_PCT            100.0

#define FOCUS_SCORE_MIN              0.0
#define FOCUS_SCORE_MAX              100.0
#define FOCUS_SCORE_WARN             70.0
#define FOCUS_SCORE_DANGER           45.0


// =========================
// OTA UPDATE CONFIG
// =========================
//
// OTA qua RPC updateFirmware.
// v1.8.0 yeu cau firmware URL la HTTPS co the tai truc tiep tu ESP32.
// Neu dung GitHub Release asset, repo/asset phai truy cap duoc cong khai
// hoac phai co co che proxy/token an toan rieng.
#define OTA_ENABLED                   1
#define OTA_ALLOW_INSECURE_TLS        1
#define OTA_HTTP_TIMEOUT_MS           20000UL
#define OTA_RESTART_DELAY_MS          1200UL
#define OTA_REQUIRE_HTTPS             1
