# HP12 User Guide

## 1. HP12 dùng để làm gì?

HP12 giúp theo dõi môi trường làm việc theo thời gian thực, gồm nhiệt độ, độ ẩm, heat index, ánh sáng tương đối và trạng thái kết nối mạng.

Thiết bị giúp trả lời câu hỏi:

> Không gian này đang hỗ trợ hay đang làm giảm khả năng tập trung sâu?

## 2. Ý nghĩa màn hình OLED

Màn hình OLED hiển thị nhanh các trạng thái chính:

- Nhiệt độ
- Độ ẩm
- Heat Index
- Focus Score
- Trạng thái cảnh báo
- Trạng thái WiFi
- Trạng thái ThingsBoard
- Phiên bản firmware

## 3. Ý nghĩa đèn LED

- Đèn xanh: không gian còn phù hợp để tập trung.
- Đèn vàng: điều kiện bắt đầu bất lợi, nên chú ý.
- Đèn đỏ: môi trường gây stress nhiệt hoặc cần xử lý ngay.

## 4. Cách dùng nút bấm

| Thao tác | Chức năng |
|---|---|
| Bấm `NAV` 1 lần | Chuyển tab OLED |
| Bấm `NAV` 2 lần | Mở trang Help |
| Bấm `NAV` 3 lần | Bật/tắt cảnh báo âm thanh |
| Bấm `ADVICE` 1 lần | Xem khuyến nghị |
| Bấm `ADVICE` 2 lần | Xem cơ sở đánh giá |
| Bấm `ADVICE` 3 lần | Xem trạng thái WiFi |
| Giữ `NAV + ADVICE` 5.5 giây | Mở vùng cài đặt WiFi |

## 5. Khi nào cần đổi WiFi?

Cần đổi WiFi khi:

- Mang HP12 đến địa điểm mới.
- WiFi cũ không còn dùng được.
- Muốn chuyển sang mạng WiFi mạnh hơn.
- ThingsBoard không nhận dữ liệu do thiết bị mất mạng.

## 6. Location trên HP12 là gì?

Location của HP12 được lấy theo public IP của mạng WiFi đang dùng.

Đây là vị trí gần đúng theo mạng Internet, không phải GPS chính xác.

Có thể dùng location để biết:

- Thiết bị đang dùng mạng nào.
- Public IP hiện tại là gì.
- Thiết bị có đổi mạng hoặc đổi khu vực hay không.

Không nên dùng location IP để khẳng định chính xác thiết bị đang ở phòng nào, xưởng nào hoặc nhà máy nào.

## 7. Không nên làm gì?

- Không dùng WiFi quá yếu.
- Không dùng WiFi 5GHz-only.
- Không upload `secrets.h` lên GitHub.
- Không bấm OTA liên tục nhiều lần.
- Không đặt ESP32 sát nguồn nhiễu, dây nguồn dày hoặc vật kim loại.
