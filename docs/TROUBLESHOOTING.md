# HP12 Troubleshooting

Tài liệu này giúp người dùng xử lý nhanh các lỗi thường gặp khi sử dụng HP12.

---

## 1. HP12 không vào được WiFi

Nguyên nhân thường gặp:

- WiFi quá yếu với ESP32.
- WiFi không phải 2.4GHz.
- Sai mật khẩu WiFi.
- Router không cho thiết bị lạ kết nối.
- Mạng có trang đăng nhập phụ.
- HP12 đặt ở vị trí nhiễu sóng hoặc bị che anten.

Cách xử lý:

1. Chọn WiFi 2.4GHz.
2. Ưu tiên mạng có RSSI từ -75 dBm trở lên.
3. Không chọn mạng dưới -83 dBm.
4. Kiểm tra lại mật khẩu.
5. Đặt HP12 gần router hơn.
6. Thử bằng hotspot điện thoại.

Hotspot test đề xuất:

Tên WiFi: HP12TEST  
Mật khẩu: 12345678

Nếu HP12 vào được hotspot điện thoại, nghĩa là thiết bị và firmware đang hoạt động; vấn đề nằm ở mạng WiFi tại địa điểm đó.

---

## 2. HP12 cứ quay lại màn hình setup

Dấu hiệu:

- HP12 phát lại mạng HP12-SETUP.
- OLED báo Portal: ACTIVE.
- Sau khi nhập WiFi mới, thiết bị vẫn quay lại setup.

Nguyên nhân thường gặp:

- WiFi được chọn quá yếu.
- Sai mật khẩu WiFi.
- Router không cấp được địa chỉ IP.
- Có WiFi nhưng không có Internet.
- Có Internet nhưng không kết nối được ThingsBoard.
- Token ThingsBoard sai hoặc thiếu.

Cách xử lý:

1. Xem lỗi gần nhất trên OLED hoặc trên trang setup.
2. Chọn WiFi mạnh hơn.
3. Không chọn mạng dưới -83 dBm.
4. Nhập lại mật khẩu thật chậm và kiểm tra kỹ.
5. Thử hotspot điện thoại để loại trừ lỗi thiết bị.

---

## 3. Điện thoại vào WiFi được nhưng HP12 không vào được

Điều này có thể xảy ra vì điện thoại có anten mạnh hơn ESP32.

Điện thoại báo WiFi tốt không có nghĩa là HP12 cũng kết nối ổn định.

Nguyên nhân thường gặp:

- Sóng đủ mạnh với điện thoại nhưng yếu với ESP32.
- WiFi dùng 5GHz hoặc band steering.
- Router dùng chung tên cho 2.4GHz và 5GHz.
- Mạng công cộng giới hạn thiết bị IoT.
- Mạng có đăng nhập web/captive portal.

Cách xử lý:

1. Chọn WiFi 2.4GHz riêng.
2. Dùng SSID có tên rõ như 2G, 2.4G hoặc IoT.
3. Đặt HP12 gần router hơn.
4. Tránh đặt ESP32 sát dây nguồn, module nguồn, kim loại hoặc bó dây dày.

---

## 4. Có WiFi nhưng ThingsBoard không có dữ liệu

Dấu hiệu:

- HP12 đã có IP.
- WiFi có vẻ đã kết nối.
- ThingsBoard vẫn không thấy dữ liệu mới.

Nguyên nhân thường gặp:

- ThingsBoard Access Token sai.
- Token bị bỏ trống.
- MQTT chưa kết nối.
- WiFi có Internet nhưng chặn cổng MQTT.
- ThingsBoard đang giới hạn tạm thời do gửi quá nhiều message.

Cách xử lý:

1. Kiểm tra token trong secrets.h.
2. Kiểm tra Access Token của device HP12 trên ThingsBoard.
3. Trên OLED kiểm tra MQTT: OK hay MQTT: NO.
4. Chờ vài phút nếu ThingsBoard báo rate limit.
5. Không bấm OTA hoặc reset liên tục trong thời gian ngắn.

---

## 5. ThingsBoard báo Rate limits exceeded

Ý nghĩa:

HP12 hoặc quá trình test đã gửi quá nhiều message lên ThingsBoard trong thời gian ngắn.

Nguyên nhân thường gặp:

- Reboot nhiều lần.
- OTA nhiều lần.
- MQTT reconnect liên tục.
- Gửi telemetry/attributes quá dày.
- Dashboard và RPC được test liên tục.

Cách xử lý:

1. Tạm ngưng test vài phút.
2. Không bấm OTA liên tục.
3. Để thiết bị chạy ổn định.
4. Kiểm tra lại telemetry interval trong firmware nếu lỗi lặp lại thường xuyên.

---

## 6. Location không chính xác

Location của HP12 được lấy theo public IP của WiFi đang dùng.

Đây là vị trí gần đúng theo mạng Internet, không phải GPS chính xác.

Có thể dùng location để biết:

- HP12 đang dùng mạng nào.
- Public IP hiện tại là gì.
- Thiết bị có đổi mạng hoặc đổi khu vực không.

Không nên dùng location để kết luận chính xác:

- Thiết bị đang ở phòng nào.
- Thiết bị đang ở xưởng nào.
- Thiết bị chắc chắn ở đúng nhà máy nào.

Muốn xác nhận địa điểm vật lý, nên kết hợp thêm:

- Site ID.
- Tên địa điểm đã đăng ký.
- SSID WiFi.
- QR/NFC xác nhận địa điểm.
- Thông tin lắp đặt ban đầu.

---

## 7. OTA không cập nhật được

Nguyên nhân thường gặp:

- HP12 đang offline.
- WiFi yếu hoặc mất kết nối.
- ThingsBoard token sai.
- GitHub Release chưa có file HP12.ino.bin.
- Link OTA sai.
- Firmware cũ chưa xử lý redirect GitHub.
- ThingsBoard RPC không gửi được đến thiết bị.

Cách xử lý:

1. Kiểm tra HP12 có Active trên ThingsBoard không.
2. Kiểm tra WiFi có ổn định không.
3. Kiểm tra GitHub Release có file HP12.ino.bin không.
4. Kiểm tra RPC params.
5. Không bấm OTA nhiều lần liên tục.
6. Nếu HP12 đang Inactive, cần khôi phục WiFi trước rồi mới OTA được.

---

## 8. OLED không chuyển tab khi bấm nút

Nguyên nhân thường gặp:

- Thiết bị đang ở chế độ WiFi setup.
- Nút đang bị giữ quá lâu.
- Logic setup đang ưu tiên chế độ cấu hình.
- Nút hoặc dây nối bị lỏng.

Cách xử lý:

1. Thoát khỏi chế độ setup.
2. Reboot lại HP12.
3. Bấm NAV ngắn một lần để kiểm tra chuyển tab.
4. Kiểm tra dây nút NAV và ADVICE.
5. Nếu vẫn lỗi, kiểm tra lại firmware version.

---

## 9. Đèn hoặc còi báo bất thường

Nguyên nhân thường gặp:

- Môi trường đang vượt ngưỡng cảnh báo.
- Heat Index cao.
- Độ ẩm cao.
- Ánh sáng quá thấp.
- Cảm biến đọc sai tạm thời.
- Còi chưa được mute.

Cách xử lý:

1. Xem trạng thái trên OLED.
2. Kiểm tra nhiệt độ, độ ẩm, heat index.
3. Đợi vài chu kỳ đo để cảm biến ổn định.
4. Bấm NAV 3 lần để mute/unmute alarm nếu cần.
5. Không đặt cảm biến sát nguồn nhiệt hoặc luồng gió bất thường.

---

## 10. Khi nào cần reset hoặc cấu hình lại?

Cần cấu hình lại khi:

- Mang HP12 đến địa điểm mới.
- WiFi cũ không còn dùng được.
- Muốn đổi sang mạng WiFi mạnh hơn.
- HP12 báo Portal: ACTIVE.
- ThingsBoard không nhận dữ liệu do mất mạng.

Cách mở setup:

Giữ NAV + ADVICE trong khoảng 5.5 giây.

Sau đó dùng điện thoại kết nối vào HP12-SETUP và mở:

http://192.168.4.1
