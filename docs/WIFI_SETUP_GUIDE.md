# HP12 WiFi Setup Guide

Tài liệu này hướng dẫn người dùng đổi WiFi cho HP12 bằng điện thoại.

---

## 1. Khi nào cần đổi WiFi?

Cần đổi WiFi khi:

- Mang HP12 đến địa điểm mới.
- WiFi cũ không còn dùng được.
- Muốn chuyển sang mạng WiFi mạnh hơn.
- HP12 báo mất kết nối mạng.
- ThingsBoard không còn nhận dữ liệu từ HP12.

---

## 2. Cách mở chế độ cài WiFi

Trên thiết bị HP12, giữ đồng thời hai nút:

NAV + ADVICE

trong khoảng 5.5 giây.

Sau đó HP12 sẽ phát ra mạng WiFi setup có tên dạng:

HP12-SETUP-xxxx

Ví dụ:

HP12-SETUP-2DF4

---

## 3. Kết nối điện thoại vào HP12-SETUP

Trên điện thoại:

1. Mở phần WiFi.
2. Chọn mạng HP12-SETUP-xxxx.
3. Nhập mật khẩu setup mặc định:

12345678

Nếu điện thoại báo mạng này không có Internet, hãy chọn:

Vẫn kết nối  
hoặc  
Use without Internet  
hoặc  
Keep WiFi connection

Lưu ý: điện thoại phải tiếp tục giữ kết nối với HP12-SETUP.

---

## 4. Mở trang cài WiFi

Sau khi điện thoại đã kết nối vào HP12-SETUP, mở trình duyệt và vào địa chỉ:

http://192.168.4.1

Nếu trình duyệt không tự mở, hãy gõ thủ công địa chỉ trên.

---

## 5. Chọn WiFi mới cho HP12

Trong trang HP12 WiFi Setup:

1. Chọn WiFi mới trong danh sách quét được.
2. Chỉ chọn WiFi 2.4GHz.
3. Ưu tiên WiFi có sóng mạnh.
4. Nhập mật khẩu WiFi mới.
5. Bấm Lưu & Kết nối WiFi mới.

Quan trọng:

Không chọn WiFi mới trong phần cài đặt WiFi của điện thoại.

Điện thoại chỉ dùng để vào mạng HP12-SETUP.  
WiFi mới phải được chọn trong trang 192.168.4.1 của HP12.

---

## 6. Chuẩn WiFi phù hợp cho HP12

HP12 dùng ESP32, nên chỉ hỗ trợ WiFi 2.4GHz.

Chuẩn sóng khuyến nghị:

| RSSI | Đánh giá | Khuyến nghị |
|---|---|---|
| -30 đến -55 dBm | Rất mạnh | Rất tốt |
| -56 đến -67 dBm | Tốt | Phù hợp IoT |
| -68 đến -75 dBm | Tạm ổn | Có thể dùng |
| -76 đến -82 dBm | Yếu | Không khuyến nghị |
| Dưới -83 dBm | Rất yếu | Không nên dùng |

Với HP12, nên chọn WiFi từ -75 dBm trở lên.

---

## 7. Vì sao điện thoại vào được WiFi nhưng HP12 không vào được?

Điện thoại có anten mạnh hơn ESP32.

Vì vậy có trường hợp điện thoại báo WiFi tốt, nhưng HP12 vẫn thấy WiFi yếu hoặc không kết nối ổn định.

Nguyên nhân thường gặp:

- WiFi quá yếu với ESP32.
- WiFi chỉ hỗ trợ 5GHz.
- Router dùng chung tên cho 2.4GHz và 5GHz.
- Mạng có trang đăng nhập phụ.
- Sai mật khẩu WiFi.
- Router chặn thiết bị IoT.
- HP12 đặt gần dây nguồn, module nguồn, kim loại hoặc vùng nhiễu.

---

## 8. Dấu hiệu đổi WiFi thành công

Sau khi lưu WiFi mới, HP12 sẽ tự khởi động lại.

Trên OLED nên thấy:

SSID: tên WiFi mới  
IP: địa chỉ IP nội bộ  
RSSI: cường độ sóng  
MQTT: OK  
LOC: trạng thái định vị theo IP  

Trên ThingsBoard, thiết bị HP12 sẽ chuyển sang trạng thái Active.

---

## 9. Nếu HP12 vẫn quay lại setup

Nếu nhập WiFi xong nhưng HP12 vẫn quay lại HP12-SETUP, nguyên nhân thường là:

- WiFi quá yếu.
- Sai mật khẩu.
- Router không cấp IP.
- WiFi có kết nối nhưng không có Internet.
- MQTT/ThingsBoard bị chặn.
- Token ThingsBoard sai hoặc thiếu.

Cách xử lý nhanh:

1. Chọn WiFi mạnh hơn.
2. Không chọn mạng dưới -83 dBm.
3. Kiểm tra lại mật khẩu.
4. Thử hotspot điện thoại với tên đơn giản:

HP12TEST

mật khẩu:

12345678

Nếu HP12 vào được hotspot điện thoại, nghĩa là thiết bị và code đang hoạt động; vấn đề nằm ở mạng WiFi tại địa điểm đó.

---

## 10. Tóm tắt thao tác nhanh

Muốn đổi WiFi:

1. Giữ NAV + ADVICE trong 5.5 giây.
2. Kết nối điện thoại vào HP12-SETUP-xxxx.
3. Mở http://192.168.4.1.
4. Chọn WiFi 2.4GHz đủ mạnh.
5. Nhập mật khẩu.
6. Bấm Lưu & Kết nối WiFi mới.
7. Chờ HP12 reboot và kết nối ThingsBoard.
