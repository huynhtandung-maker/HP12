#pragma once

/*
  =============================================================================
  PCODE HP12 - DATA INTELLIGENCE EDITION (MASTER CONFIGURATION)
  =============================================================================
  Tài liệu cấu hình tập trung cho toàn bộ hệ thống HP12.
  Mọi thay đổi về phần cứng, mạng, ngưỡng y sinh, và trải nghiệm người dùng (UX)
  đều phải được thực hiện tại đây. KHÔNG sửa trực tiếp trong file HP12.ino.
  =============================================================================
*/

// ============================================================================
// 1. THÔNG TIN ĐỊNH DANH THIẾT BỊ (FIRMWARE INFO)
// ============================================================================
#define DEVICE_NAME        "HP12_Edge"          // Tên hiển thị của thiết bị trên Cloud/Mạng
#define DEVICE_MODEL       "PCODE_DataNode_V1"  // Phân loại model phần cứng
#define FIRMWARE_VERSION   "v1.12.3"            // WiFi State Machine Fix + DeepWork Tropical Tuning


// ============================================================================
// 2. CẤU HÌNH CHÂN PHẦN CỨNG (PIN CONFIGURATION)
// Phù hợp cho board ESP32 30 chân tiêu chuẩn (ESP32-WROOM / ESP32-U)
// ============================================================================

// --- Cảm biến Nhiệt Ẩm (DHT22) ---
#define DHT_PIN            14       // Chân data của DHT22 nối vào GPIO 14
#define DHT_TYPE           DHT22    // Loại cảm biến (DHT11, DHT21, DHT22)

// --- Màn hình OLED SSD1306 (Giao tiếp I2C) ---
#define OLED_SDA_PIN       21       // Chân truyền dữ liệu I2C (SDA)
#define OLED_SCL_PIN       22       // Chân xung nhịp I2C (SCL)
#define OLED_WIDTH         128      // Chiều rộng màn hình (pixel)
#define OLED_HEIGHT        64       // Chiều cao màn hình (pixel)
#define OLED_ADDR          0x3C     // Địa chỉ I2C mặc định của màn hình OLED
#define OLED_RESET         -1       // Chân reset của OLED (-1 nếu không có chân reset rời)

// --- Cảm biến Ánh sáng (LDR LM393) ---
#define LIGHT_SENSOR_ENABLED       1    // 1: Bật cảm biến ánh sáng | 0: Tắt
#define LIGHT_AO_PIN               34   // Chân đọc tín hiệu Analog (Đo cường độ sáng)
#define LIGHT_DO_PIN               35   // Chân đọc tín hiệu Digital
#define LIGHT_USE_DIGITAL_DO       0    // 0: Chỉ dùng Analog để đo % ánh sáng
#define LIGHT_ADC_BRIGHT_IS_HIGH   0    // 0: Ánh sáng mạnh -> Điện áp ADC thấp (Tùy module LM393)

// --- Các Nút Bấm Vật Lý (Buttons) ---
#define BUTTON_NAV_PIN             27   // Nút chuyển Tab / Mute còi (Nối GPIO 27 với GND)
#define BUTTON_NAV_ACTIVE_LOW      1    // 1: Bấm nút -> Mức LOW (GND)

#define BUTTON_ADVICE_PIN          13   // Nút xem lời khuyên y tế (Nối GPIO 13 với GND)
#define BUTTON_ADVICE_ACTIVE_LOW   1    // 1: Bấm nút -> Mức LOW (GND)

// --- Hệ thống Cảnh báo Âm thanh (Buzzer) ---
#define BUZZER_PIN                 26   // Chân điều khiển Còi chip
#define BUZZER_ACTIVE_LOW          0    // 0: Cấp mức CAO (HIGH) còi mới kêu

// --- Đèn LED Môi trường (Environmental LEDs) ---
#define LED_GREEN_PIN              25   // LED Xanh lá (Trạng thái Tối ưu)
#define LED_YELLOW_PIN             32   // LED Vàng (Trạng thái Cảnh báo)
#define LED_RED_PIN                33   // LED Đỏ (Trạng thái Nguy hiểm)
#define LED_ACTIVE_LOW             1    // 1: Cấp mức LOW đèn sáng (Kích kiểu Sink current)

// --- Đèn LED Tín hiệu Bo mạch (Onboard Blue LED) ---
#define ONBOARD_BLUE_LED_ENABLED    1      // 1: Bật đèn nháy báo nhịp tim hệ thống trên ESP32
#define ONBOARD_BLUE_LED_PIN        2      // Chân mặc định của LED xanh dương ESP32
#define ONBOARD_BLUE_LED_ACTIVE_LOW 0      // 0: Sáng khi cấp mức CAO
#define BLUE_LED_SELF_TEST_MS       8000UL // Nháy liên tục trong 8 giây lúc Boot để test GPIO


// ============================================================================
// 3. TẦN SUẤT ĐỌC & HIỂN THỊ (TIMING & PERFORMANCE)
// ============================================================================

#define SENSOR_READ_INTERVAL_MS      2000UL  // Cứ 2 giây đọc DHT22 một lần (DHT22 đọc nhanh hơn sẽ lỗi)
#define LIGHT_READ_INTERVAL_MS       500UL   // Cứ 0.5 giây đọc ánh sáng một lần
#define OLED_UPDATE_INTERVAL_MS      160UL   // Cập nhật màn hình ~6 FPS (Đủ mượt, tiết kiệm CPU)
#define LED_UPDATE_INTERVAL_MS       80UL    // Cập nhật đèn LED mỗi 80ms
#define BLUE_LED_UPDATE_INTERVAL_MS  60UL    // Cập nhật LED nhịp tim mỗi 60ms
#define SCROLL_UPDATE_INTERVAL_MS    150UL   // Tốc độ cuộn chữ ngang màn hình

// --- Cấu hình Trải nghiệm Nút bấm (Button UX) ---
#define BUTTON_DEBOUNCE_MS           35UL    // Chống dội phím (Lọc nhiễu tĩnh điện 35ms)
#define BUTTON_CLICK_WINDOW_MS       450UL   // Cửa sổ thời gian chờ để đếm Double/Triple Click
#define BUTTON_LONG_PRESS_MS         2500UL  // Bấm giữ 2.5 giây được tính là Long Press
#define HELP_PAGE_DURATION_MS        8000UL  // Trang "Hướng dẫn" tự tắt sau 8 giây

// --- Cấu hình Còi báo (Alarm) ---
#define ALARM_MUTE_MS                600000UL // Khi bấm tắt còi, sẽ im lặng trong 10 phút


// ============================================================================
// 4. MẠNG, THỜI GIAN & PORTAL WIFI (NETWORK, NTP & SETUP)
// ============================================================================

// --- Kết nối WiFi ---
#define WIFI_CONNECT_TIMEOUT_MS      45000UL // Chờ xin IP tối đa 45s (Cho phép tương thích Router Doanh nghiệp/Mesh)
#define WIFI_RETRY_INTERVAL_MS       15000UL // Ép nối lại mỗi 15 giây nếu rớt mạng
#define WIFI_RADIO_SETTLE_MS         180UL   // Đợi WiFi stack ổn định sau disconnect/mode switch
#define WIFI_PORTAL_RADIO_RESET_MS   260UL   // Đợi lâu hơn khi chuyển từ STA sang AP portal
#define WIFI_BEGIN_REISSUE_GUARD_MS  1000UL  // Khoảng đệm trước WiFi.begin để tránh gọi chồng STA config

// --- Cấu hình WiFi Setup Portal (Khi mất mạng) ---
#define WIFI_SETUP_PORTAL_ENABLED    1             // 1: Bật Portal
#define WIFI_SETUP_AP_SSID           "HP12-SETUP"  // Tên WiFi thiết bị tự phát ra để cài đặt
#define WIFI_SETUP_AP_PASSWORD       ""            // Mật khẩu WiFi (Trống = Không mật khẩu)
#define WIFI_SETUP_DNS_PORT          53            // Cổng DNS chặn chuyển hướng Captive Portal
#define WIFI_SETUP_PREF_NAMESPACE    "hp12wifi"    // Tên phân vùng lưu WiFi trong NVS Flash
#define WIFI_SETUP_RESTART_MS        2500UL        // Đợi 2.5s rồi Reset mạch sau khi lưu WiFi
#define WIFI_SETUP_TRIGGER_HOLD_MS   5500UL        // Nhấn giữ 2 nút 5.5s để ép xóa WiFi cũ

// --- Thời gian thực (NTP Time) ---
#define NTP_SERVER                   "pool.ntp.org" // Cụm máy chủ thời gian chuẩn quốc tế
#define GMT_OFFSET_SEC               (7 * 3600)     // Múi giờ Việt Nam (GMT+7)
#define DAYLIGHT_OFFSET_SEC          0              // Không dùng Giờ Mùa Hè (DST)


// ============================================================================
// 5. MÁY CHỦ ĐÁM MÂY (THINGSBOARD CLOUD)
// ============================================================================

#define MQTT_CLIENT_ID_PREFIX        "HP12_Edge_" // Tiền tố ClientID nối vào Broker MQTT
#define MQTT_BUFFER_SIZE             1024         // Mở rộng bộ đệm lên 1KB để chứa JSON Payload lớn
#define MQTT_RETRY_INTERVAL_MS       10000UL      // Nếu rớt Cloud, thử nối lại mỗi 10 giây
#define TELEMETRY_INTERVAL_MS        30000UL      // Cứ 30 giây gửi dữ liệu môi trường lên Cloud 1 lần
#define FORCE_FIRST_SYNC_ENABLED     1            // 1: Gửi ngay dữ liệu lên Cloud lập tức khi có mạng
#define ATTRIBUTES_RESEND_MS         300000UL     // Cứ 5 phút gửi lại IP, Tọa độ GPS lên Cloud


// ============================================================================
// 6. CẬP NHẬT TỪ XA (OTA) & ĐIỀU KHIỂN RPC
// ============================================================================

#define RPC_ENABLED                  1       // 1: Nhận lệnh từ xa qua ThingsBoard
#define RPC_RESTART_DELAY_MS         1200UL  // Độ trễ 1.2s trước khi thực thi lệnh Reset từ Web
#define OTA_ENABLED                  1       // 1: Cho phép Nạp Code Không Dây (OTA)
#define OTA_REQUIRE_HTTPS            1       // 1: Bắt buộc dùng HTTPS bảo mật để kéo Firmware
#define OTA_ALLOW_INSECURE_TLS       1       // 1: Bỏ qua check SSL để tiết kiệm RAM trên ESP32
#define OTA_HTTP_TIMEOUT_MS          20000UL // Chờ tải file Code tối đa 20 giây
#define OTA_OLED_SUCCESS_HOLD_MS     2500UL  // Giữ màn hình báo "Thành công" 2.5s
#define OTA_OLED_ERROR_HOLD_MS       2500UL  // Giữ màn hình báo "Lỗi" 2.5s


// ============================================================================
// 7. NGƯỠNG TIỆN NGHI CHO DEEP WORK TRONG KHÍ HẬU NÓNG ẨM
// Tinh thần: không lấy "mát kiểu châu Âu" làm chuẩn duy nhất.
// GREEN  = còn hỗ trợ tập trung sâu.
// YELLOW = bắt đầu hao năng lượng nhận thức, cần can thiệp nhẹ.
// RED    = stress nhiệt rõ, cần hành động ngay.
// Đây là ngưỡng thực dụng cho MVP, không thay thế chuẩn đo lường y tế/công thái học chính thức.
// ============================================================================

// --- Vùng XANH: hỗ trợ làm việc sâu trong bối cảnh nhiệt đới ---
#define TEMP_GOOD_MIN      24.5   // Dưới mức này dễ lạnh/khô do máy lạnh, vẫn nên cảnh báo nhẹ
#define TEMP_GOOD_MAX      30.0   // Vùng xanh nhiệt đới: không bắt buộc phải "mát lạnh"
#define HUM_GOOD_MIN       42.0   // Quá khô dễ khó chịu mắt/mũi khi dùng máy lạnh lâu
#define HUM_GOOD_MAX       72.0   // Nới cho Việt Nam; >72% bắt đầu bí, dính, giảm tập trung

// --- Vùng VÀNG: vẫn làm việc được nhưng đang hao năng lượng nhận thức ---
#define TEMP_WARN          31.5   // Bắt đầu nóng rõ; nên quạt, thông gió hoặc giảm tải nhiệt
#define HUM_WARN           78.0   // Ẩm cao; dễ bí, mệt, khó thoát mồ hôi
#define HEAT_INDEX_WARN    32.0   // Cảm nhận nhiệt bắt đầu kéo giảm sự tỉnh táo

// --- Vùng ĐỎ: stress nhiệt rõ, cần hành động ngay ---
#define TEMP_DANGER        34.0   // Nóng cao trong phòng làm việc; không phù hợp deep work kéo dài
#define HUM_DANGER         88.0   // Rất ẩm; cảnh báo mạnh, đặc biệt khi đi kèm nhiệt cao
#define HEAT_INDEX_DANGER  38.0   // Cảm nhận nhiệt báo động cho hiệu suất nhận thức

// --- Mốc tham chiếu Heat Index y sinh ---
// Dùng để trừ điểm FocusScore theo stress nhiệt cảm nhận.
#define HEAT_INDEX_MED_CAUTION_C     32.0   // Thận trọng: bắt đầu mệt/giảm tỉnh táo
#define HEAT_INDEX_MED_DANGER_C      39.0   // Nguy hiểm: stress nhiệt rõ, giảm mạnh khả năng tập trung
#define HEAT_INDEX_MED_EXTREME_C     51.0   // Cực nguy hiểm: không phù hợp ở lâu

// --- Thang điểm ánh sáng cho làm việc tập trung ---
// LDR chỉ là chỉ báo tương đối, cần calibrate theo vị trí đặt cảm biến.
#define LIGHT_DARK_PCT       28.0 // Quá tối: dễ mỏi mắt, buồn ngủ, giảm tỉnh táo
#define LIGHT_FOCUS_MIN_PCT  40.0 // Bắt đầu đủ sáng cho đọc/viết/làm việc màn hình
#define LIGHT_FOCUS_MAX_PCT  78.0 // Vùng sáng tốt, ít gây chói
#define LIGHT_GLARE_PCT      90.0 // Quá sáng/chói: dễ lóa màn hình, căng mắt

// --- Bộ lọc nhiễu phần cứng (Outlier Rejection) ---
#define SENSOR_TEMP_MIN    -10.0  // Nhiệt độ < -10°C -> Báo lỗi mạch
#define SENSOR_TEMP_MAX    60.0   // Nhiệt độ > 60°C -> Báo lỗi mạch
#define SENSOR_HUM_MIN     0.0    // Độ ẩm < 0% -> Báo lỗi
#define SENSOR_HUM_MAX     100.0  // Độ ẩm > 100% -> Báo lỗi
#define SENSOR_HUM_SUSPECT 99.9   // Báo đúng 99.9% liên tục -> Đọng nước trên cảm biến


// ============================================================================
// 8. TRẢI NGHIỆM THÍNH GIÁC & THỊ GIÁC (ACOUSTIC & VISUAL RHYTHM)
// ============================================================================

#define BUZZER_PROFILE_START         1  // 0=Im lặng, 1=Văn phòng, 2=Cảnh báo mạnh

// --- Nguyên tắc âm thanh ---
// Deep work không chấp nhận còi lặt vặt. Còi chỉ xuất hiện khi cần hành động.
// VÀNG: ưu tiên LED + OLED, không phá tập trung.
// ĐỎ: bíp ngắn, thưa, đủ nhắc nhưng không gây hoảng.
// ĐỎ MẠNH: bíp 3 nhịp khi stress nhiệt cao hoặc lỗi nghiêm trọng.

// --- Profile VÀNG: cảnh báo mềm, mặc định không kêu ---
#define BUZZER_WARN_ENABLED          0        // 0: im lặng ở WARN; 1: bật tick rất nhẹ
#define BUZZER_WARN_PERIOD_MS        60000UL  // Nếu bật: 60 giây tick 1 lần
#define BUZZER_WARN_ON_MS            35UL     // Tick siêu ngắn, chỉ để xác nhận trạng thái

// --- Profile ĐỎ: cảnh báo văn phòng, chống alarm fatigue ---
#define BUZZER_NORMAL_PERIOD_MS      22000UL  // 22 giây lặp một cụm bíp
#define BUZZER_NORMAL_ON_MS          55UL     // Bíp ngắn, sắc, không kéo dài
#define BUZZER_NORMAL_GAP_MS         130UL    // Nghỉ giữa 2 bíp
#define BUZZER_NORMAL_COUNT          2        // 2 bíp = cần can thiệp môi trường

// --- Profile ĐỎ MẠNH: stress nhiệt rõ / cần xử lý ngay ---
#define BUZZER_STRONG_PERIOD_MS      8000UL   // 8 giây lặp một cụm
#define BUZZER_STRONG_ON_MS          75UL     // Bíp rõ hơn
#define BUZZER_STRONG_GAP_MS         110UL    // Nhịp gấp hơn
#define BUZZER_STRONG_COUNT          3        // 3 bíp = ưu tiên xử lý ngay

// --- Nhịp điệu đèn LED môi trường ---
// XANH: chỉ nên sáng khi thật sự còn hỗ trợ tập trung.
// VÀNG: nhịp chậm để báo "đang lệch chuẩn, cần chỉnh".
// ĐỎ: nhịp nhanh hơn, nhưng không nhấp nháy quá gắt gây khó chịu.
// LỖI CẢM BIẾN: nhịp riêng để phân biệt với nóng/ẩm.
#define LED_START_BLINK_MS           700UL    // Boot: nháy vừa phải, không gắt
#define LED_WARN_BLINK_MS            1400UL   // WARN: nhịp chậm, dễ chịu, không gây loạn thị giác
#define LED_DANGER_BLINK_MS          320UL    // DANGER: đủ khẩn cấp nhưng không "nháy điên"
#define LED_SENSOR_BLINK_MS          650UL    // Sensor error: nhịp riêng, dễ nhận ra

// --- Nhịp tim bo mạch (Onboard Blue LED Heartbeat) ---
// LED xanh dương chỉ báo firmware còn sống/cloud-loop không bị treo.
#define BLUE_LED_ALIVE_PERIOD_MS     3500UL   // 3.5 giây nháy một cụm
#define BLUE_LED_PULSE_ON_MS         60UL     // Chớp ngắn
#define BLUE_LED_PULSE_GAP_MS        120UL    // Nháy đúp kiểu thiết bị mạng/server


// ============================================================================
// 9. CẤU HÌNH GIAO DIỆN MÀN HÌNH (OLED UX/UI)
// ============================================================================

// --- Thước đo thanh ngang (Line Gauge Scale) ---
// Thu hẹp biên để OLED 0.96 inch đọc trực quan hơn; không kéo tới 52°C trừ khi cần y tế ngoài trời.
#define HEAT_BAR_MIN_C               24.0    // Heat Index thấp, dễ chịu
#define HEAT_BAR_MAX_C               44.0    // Đủ bao phủ vùng nóng ẩm trong phòng
#define TEMP_BAR_MIN_C               22.0    // Biên lạnh thực dụng cho phòng làm việc
#define TEMP_BAR_MAX_C               36.0    // Biên nóng trong nhà cần can thiệp
#define HUM_BAR_MIN_PCT              35.0    // Biên ẩm thấp thực dụng
#define HUM_BAR_MAX_PCT              95.0    // Biên ẩm cao trước mức nghi lỗi/đọng nước
#define LIGHT_BAR_MIN_PCT            0.0     // Tối
#define LIGHT_BAR_MAX_PCT            100.0   // Sáng/chói

// --- Chấm điểm Hiệu suất Nhận thức (Focus Score 0-100) ---
#define FOCUS_SCORE_MIN              0.0     
#define FOCUS_SCORE_MAX              100.0   
#define FOCUS_SCORE_WARN             65.0    // Dưới 65: bắt đầu hao năng lượng tập trung
#define FOCUS_SCORE_DANGER           35.0    // Dưới 35: nên dừng deep work, xử lý môi trường trước

// --- Vùng an toàn hiển thị OLED (Safe Viewport) ---
// Chống tràn viền chữ trên các màn OLED 0.96inch
#define OLED_SAFE_X                  2       // Thụt lề trái 2px
#define OLED_SAFE_W                  116     // Chiều rộng an toàn
#define OLED_SAFE_RIGHT              (OLED_SAFE_X + OLED_SAFE_W - 1)
#define OLED_GAUGE_X                 6       // Thụt lề thước đo
#define OLED_GAUGE_W                 108     // Rộng thước đo