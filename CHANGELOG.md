# Changelog

Tài liệu này ghi lại các thay đổi quan trọng của HP12 theo từng phiên bản.

---

## v1.13.17 - Stable Phone WiFi Setup, OTA-Safe Boot & Button UX Fix

### Mục tiêu

Ổn định trải nghiệm vận hành thực tế khi HP12 được mang đến nhiều địa điểm khác nhau, cần đổi WiFi bằng điện thoại, cập nhật OTA và giữ kết nối ThingsBoard.

### Thay đổi chính

- Cải thiện luồng cài WiFi bằng điện thoại qua `HP12-SETUP`.
- Giữ ổn định token ThingsBoard khi đổi WiFi.
- Khôi phục kết nối ThingsBoard sau khi đổi WiFi hoặc reboot.
- Sửa lỗi OTA bị ảnh hưởng bởi GitHub redirect.
- Sửa trải nghiệm nút `NAV` / `ADVICE` để không ảnh hưởng chuyển tab OLED.
- Bổ sung cơ chế OTA-safe boot để hạn chế rơi vào vòng lặp setup sau cập nhật.

### Lưu ý

- OTA chỉ hoạt động khi HP12 đang Active trên ThingsBoard.
- Không upload `secrets.h` lên GitHub.
- Nếu đổi WiFi, người dùng cần mở setup bằng `NAV + ADVICE` trong 5.5 giây.

---

## v1.13.16 - Source Update: OTA-Safe WiFi Boot

### Thay đổi chính

- Cải thiện cơ chế reconnect WiFi sau reboot.
- Bổ sung kiểm tra token ThingsBoard.
- Tăng độ ổn định khi thiết bị đổi mạng hoặc mất mạng tạm thời.

---

## v1.6.0 - ThingsBoard Stable Bridge

### Thay đổi chính

- Thêm kết nối MQTT đến ThingsBoard.
- Thêm gửi telemetry định kỳ.
- Thêm gửi attributes của thiết bị.
- Giữ OLED, cảm biến, LED, buzzer và nút bấm tiếp tục hoạt động khi mất mạng.

---

## v1.5.x - Local Comfort Device

### Thay đổi chính

- Thêm màn hình OLED local dashboard.
- Thêm đọc cảm biến DHT22.
- Thêm tính toán Heat Index.
- Thêm cảm biến ánh sáng LDR.
- Thêm Focus Score.
- Thêm logic LED 3 màu.
- Thêm buzzer cảnh báo.
- Thêm nút NAV và ADVICE.
