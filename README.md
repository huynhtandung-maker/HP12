# HP12 - Workspace Comfort IoT Device

HP12 là thiết bị IoT dùng ESP32 để theo dõi chất lượng không gian làm việc: nhiệt độ, độ ẩm, heat index, ánh sáng tương đối, trạng thái WiFi và kết nối ThingsBoard.

Mục tiêu của HP12 là giúp đánh giá không gian có đang hỗ trợ hay đang làm giảm khả năng tập trung sâu hay không.

## Dùng nhanh

1. Cấp nguồn cho HP12.
2. Xem trạng thái trên màn hình OLED.
3. Bấm `NAV` để chuyển tab màn hình.
4. Bấm `ADVICE` để xem khuyến nghị.
5. Muốn đổi WiFi, giữ `NAV + ADVICE` trong khoảng 5.5 giây.
6. Kết nối điện thoại vào `HP12-SETUP`.
7. Mở `http://192.168.4.1`.
8. Chọn WiFi 2.4GHz đủ mạnh, nhập mật khẩu và lưu.

## Tài liệu hướng dẫn

- [Hướng dẫn sử dụng](docs/USER_GUIDE.md)
- [Hướng dẫn cài đặt WiFi](docs/WIFI_SETUP_GUIDE.md)
- [Lỗi thường gặp](docs/TROUBLESHOOTING.md)
- [Hướng dẫn cập nhật OTA](docs/OTA_GUIDE.md)

## Lưu ý bảo mật

Không upload các file chứa thông tin nhạy cảm:

- `secrets.h`
- mật khẩu WiFi thật
- ThingsBoard Access Token thật

Chỉ dùng `secrets.example.h` để làm mẫu cấu hình.
