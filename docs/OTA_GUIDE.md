# HP12 OTA Update Guide

Tài liệu này hướng dẫn cập nhật firmware HP12 từ xa thông qua ThingsBoard và GitHub Release.

---

## 1. OTA là gì?

OTA là cập nhật firmware từ xa, không cần cắm cáp USB vào thiết bị.

Luồng hoạt động:

HP12 nhận lệnh từ ThingsBoard  
→ tải file firmware từ GitHub Release  
→ tự cập nhật  
→ reboot  
→ kết nối lại WiFi  
→ gửi lại phiên bản firmware mới lên ThingsBoard

---

## 2. Điều kiện để OTA hoạt động

OTA chỉ hoạt động khi:

- HP12 đang Active trên ThingsBoard.
- HP12 đang có WiFi ổn định.
- ThingsBoard token đúng.
- GitHub Release có file HP12.ino.bin.
- RPC method đúng.
- URL firmware đúng.
- Nguồn cấp cho HP12 ổn định trong lúc cập nhật.

Nếu HP12 đang Inactive, không thể cập nhật OTA từ xa.

---

## 3. Khi nào không thể OTA?

Không thể OTA nếu:

- HP12 mất WiFi.
- HP12 đang kẹt ở HP12-SETUP.
- HP12 không kết nối được ThingsBoard.
- Thiết bị bị tắt nguồn.
- GitHub Release chưa có file .bin.
- Link OTA sai.

Trong trường hợp này cần khôi phục WiFi hoặc cấp nguồn lại trước.

---

## 4. Cách xuất file firmware .bin

Trong Arduino IDE:

1. Mở project HP12.
2. Chọn đúng board ESP32.
3. Vào Sketch.
4. Chọn Export Compiled Binary.

File cần dùng là:

HP12.ino.bin

Chỉ upload đúng file này lên GitHub Release.

Không upload nhầm các file:

- merged.bin
- bootloader.bin
- partitions.bin
- .elf
- .map

---

## 5. Cách tạo GitHub Release

Trên GitHub:

1. Vào repository HP12.
2. Chọn Releases.
3. Chọn Draft a new release.
4. Tạo tag phiên bản, ví dụ:

v1.13.19

5. Đặt tiêu đề release.
6. Upload asset:

HP12.ino.bin

7. Publish release.

Lưu ý:

Nếu dùng link latest, release mới nhất phải chứa đúng file HP12.ino.bin.

---

## 6. URL OTA khuyến nghị

URL dùng cho OTA:

https://github.com/huynhtandung-maker/HP12/releases/latest/download/HP12.ino.bin

Ưu điểm:

- Luôn trỏ đến release mới nhất.
- Không cần đổi URL mỗi lần ra phiên bản mới.
- Phù hợp với nút cập nhật latest trên ThingsBoard.

Lưu ý:

Không nên cố định version trong RPC nếu đang dùng link latest.

---

## 7. Cấu hình RPC trên ThingsBoard

RPC method:

updateFirmware

RPC params khuyến nghị:

{
  "url": "https://github.com/huynhtandung-maker/HP12/releases/latest/download/HP12.ino.bin"
}

Nếu cần ghi chú phiên bản cho người quản trị, có thể thêm version, nhưng firmware nên ưu tiên URL:

{
  "url": "https://github.com/huynhtandung-maker/HP12/releases/latest/download/HP12.ino.bin",
  "version": "v1.13.19"
}

---

## 8. Trình tự cập nhật OTA đúng

1. Kiểm tra HP12 đang Active trên ThingsBoard.
2. Kiểm tra WiFi ổn định.
3. Kiểm tra release mới đã có HP12.ino.bin.
4. Bấm nút OTA trên dashboard.
5. Chờ HP12 tải firmware.
6. Chờ thiết bị reboot.
7. Kiểm tra HP12 kết nối lại ThingsBoard.
8. Kiểm tra firmwareVersion đã đổi sang phiên bản mới.

---

## 9. Dấu hiệu OTA thành công

Trên ThingsBoard hoặc OLED cần thấy:

firmwareVersion = phiên bản mới

Ví dụ:

firmwareVersion = v1.13.19

Ngoài ra có thể thấy:

- otaStatus chuyển về IDLE.
- otaLastResult báo success hoặc ok.
- Thiết bị reboot rồi Active lại.
- Telemetry tiếp tục gửi lên ThingsBoard.

---

## 10. Lỗi OTA thường gặp

| Hiện tượng | Nguyên nhân thường gặp | Cách xử lý |
|---|---|---|
| Không bấm OTA được | HP12 đang offline | Khôi phục WiFi trước |
| OTA không tải được file | Thiếu HP12.ino.bin trên Release | Upload lại đúng asset |
| OTA lỗi 302 | Link GitHub bị redirect | Dùng firmware có hỗ trợ redirect |
| OTA xong rơi vào setup | WiFi yếu hoặc boot chưa an toàn | Dùng firmware có OTA-safe boot |
| Version không đổi | Upload nhầm file .bin cũ | Xuất lại binary và upload lại |
| ThingsBoard vẫn Inactive | HP12 chưa kết nối lại MQTT | Kiểm tra WiFi và token |
| OTA bị ngắt giữa chừng | Nguồn yếu hoặc WiFi chập chờn | Đảm bảo nguồn và WiFi ổn định |

---

## 11. Nguyên tắc an toàn khi OTA

- Không OTA khi WiFi yếu.
- Không OTA khi nguồn không ổn định.
- Không bấm OTA liên tục.
- Không upload nhầm file binary.
- Không để release latest trỏ về bản lỗi.
- Luôn kiểm tra version sau khi cập nhật.
- Không đưa secrets.h lên GitHub.

---

## 12. Nếu OTA thất bại thì làm gì?

Nếu OTA thất bại nhưng HP12 vẫn online:

1. Kiểm tra otaLastResult.
2. Kiểm tra URL OTA.
3. Kiểm tra GitHub Release asset.
4. Bấm OTA lại sau vài phút.

Nếu OTA thất bại và HP12 offline:

1. Khôi phục WiFi bằng HP12-SETUP nếu có thể.
2. Nếu không vào được WiFi, cần người tại chỗ hỗ trợ.
3. Nếu vẫn không được, cần nạp lại bằng cáp USB.

---

## 13. Checklist trước khi publish release OTA

Trước khi phát hành release mới:

- Đã test bằng Arduino IDE.
- Đã copy đúng secrets.h khi build nội bộ.
- Đã Export Compiled Binary.
- File binary tên đúng là HP12.ino.bin.
- Release tag đúng.
- Release latest không trỏ nhầm bản lỗi.
- Dashboard ThingsBoard RPC vẫn dùng đúng URL.
- Sau OTA, firmwareVersion cập nhật đúng.

---

## 14. Tóm tắt nhanh

OTA thành công cần đủ 4 điều kiện:

1. HP12 đang Active trên ThingsBoard.
2. WiFi ổn định.
3. GitHub Release có đúng HP12.ino.bin.
4. RPC updateFirmware trỏ đúng URL.

Nếu thiếu một trong bốn điều kiện này, OTA có thể thất bại.
