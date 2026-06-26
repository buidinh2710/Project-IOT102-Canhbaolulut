# Hệ thống giám sát và cảnh báo mực nước thông minh

## Giới thiệu

Đây là dự án xây dựng **hệ thống giám sát mực nước thông minh** sử dụng **Arduino Uno**. Hệ thống đo khoảng cách từ cảm biến siêu âm để xác định mực nước, sau đó hiển thị kết quả trên LCD, LED Matrix và phát cảnh báo bằng đèn LED, còi buzzer. Ngoài ra, hệ thống còn hỗ trợ giao tiếp Bluetooth với điện thoại để theo dõi và điều khiển từ xa.

---

# Chức năng chính

* Đo mực nước bằng cảm biến siêu âm HC-SR04.
* Hiển thị khoảng cách trên màn hình LCD 1602.
* Hiển thị trạng thái trên LED Matrix MAX7219.
* Tự động điều chỉnh độ sáng LED Matrix theo ánh sáng môi trường bằng quang trở (LDR).
* Cảnh báo bằng đèn LED:

  * Đèn xanh: Mực nước an toàn.
  * Đèn vàng: Mực nước ở mức cảnh báo.
  * Đèn đỏ: Mực nước nguy hiểm.
* Phát âm thanh cảnh báo bằng buzzer khi mực nước ở mức nguy hiểm.
* Giao tiếp Bluetooth với điện thoại thông qua HC-05.
* Nhận lệnh điều khiển từ điện thoại:

  * Hiển thị khoảng cách hiện tại.
  * Bật/Tắt buzzer.
  * Điều chỉnh độ sáng LED Matrix.
  * Hiển thị chữ chạy trên LED Matrix.
* Gửi dữ liệu (khoảng cách, trạng thái, độ sáng...) về điện thoại.

---

# Linh kiện sử dụng

* Arduino Uno
* Cảm biến siêu âm HC-SR04
* Màn hình LCD1602
* LED Matrix MAX7219 (4 module)
* Module Bluetooth HC-05
* Quang trở (LDR)
* 03 LED (Đỏ, Vàng, Xanh)
* Buzzer
* Breadboard
* Dây nối

---

# Sơ đồ kết nối

| Thiết bị        | Chân Arduino |
| --------------- | ------------ |
| HC-SR04 Trigger | D2           |
| HC-SR04 Echo    | D7           |
| Buzzer          | D3           |
| LED Xanh        | D4           |
| LED Vàng        | D5           |
| LED Đỏ          | D6           |
| LCD RS          | D13          |
| LCD E           | D12          |
| LCD D4          | D11          |
| LCD D5          | D10          |
| LCD D6          | D9           |
| LCD D7          | D8           |
| Matrix DIN      | A0           |
| Matrix CS       | A1           |
| Matrix CLK      | A2           |
| LDR             | A3           |
| Bluetooth RX    | A4           |
| Bluetooth TX    | A5           |

---

# Nguyên lý hoạt động

1. Cảm biến HC-SR04 liên tục đo khoảng cách đến mặt nước.
2. Arduino xử lý khoảng cách nhận được.
3. Hiển thị khoảng cách lên màn hình LCD.
4. Phân loại trạng thái theo khoảng cách:

| Khoảng cách | Trạng thái      |
| ----------- | --------------- |
| ≥ 20 cm     | SAFE (An toàn)  |
| 10 – <20 cm | WARN (Cảnh báo) |
| <10 cm      | SOS (Nguy hiểm) |

5. LED và buzzer sẽ hoạt động tương ứng với từng trạng thái.
6. LED Matrix hiển thị trạng thái hoặc chữ chạy.
7. Quang trở tự động điều chỉnh độ sáng của LED Matrix.
8. Bluetooth gửi dữ liệu về điện thoại và nhận lệnh điều khiển.

---

# Các lệnh Bluetooth hỗ trợ

| Lệnh          | Chức năng                     |
| ------------- | ----------------------------- |
| DIST          | Hiển thị khoảng cách hiện tại |
| SAFE          | Hiển thị SAFE                 |
| WARN          | Hiển thị WARN                 |
| SOS           | Hiển thị SOS                  |
| BUZON         | Bật buzzer                    |
| BUZOFF        | Tắt buzzer                    |
| BR            | Xem độ sáng hiện tại          |
| LED:x         | Điều chỉnh độ sáng (0-15)     |
| TEXT:Nội dung | Hiển thị chữ chạy             |

Ví dụ:

```text
TEXT:XIN CHAO
LED:10
DIST
BUZON
```

---

# Thư viện sử dụng

* LiquidCrystal
* MD_Parola
* MD_MAX72xx
* SPI
* SoftwareSerial

---

# Hướng dẫn chạy chương trình

1. Cài đặt Arduino IDE.
2. Cài đặt các thư viện cần thiết.
3. Kết nối các linh kiện theo sơ đồ chân.
4. Nạp chương trình vào Arduino Uno.
5. Ghép nối Bluetooth HC-05 với điện thoại.
6. Mở ứng dụng Bluetooth Terminal.
7. Gửi các lệnh để điều khiển và theo dõi hệ thống.

---

# Cấu trúc thư mục

```text
SmartWaterLevel/
│
├── sketch_sep23a.ino
├── README.md
├── images/
│   ├── mo_hinh.png
│   ├── so_do_thiet_ke.png
│   └── demo.gif
```

---

# Hướng phát triển

* Điều khiển máy bơm tự động.
* Gửi cảnh báo qua Internet.
* Lưu lịch sử mực nước.
* Kết nối với ứng dụng Android.
* Sử dụng ESP32 để giám sát từ xa qua WiFi.

---

# Tác giả

Dự án được thực hiện nhằm nghiên cứu và ứng dụng Arduino trong việc giám sát, cảnh báo mực nước và giao tiếp Bluetooth.
