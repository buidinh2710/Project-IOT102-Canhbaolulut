#include <LiquidCrystal.h> //điều khiển màn hình LCD 1602 dùng 6 chân.
#include <MD_Parola.h> //điều khiển LED Matrix MAX7219 dạng chạy chữ.
#include <MD_MAX72xx.h>//điều khiển LED Matrix MAX7219 dạng chạy chữ.
#include <SPI.h>//giao tiếp SPI (cho LED Matrix). -> SPI là giao tiếp tốc độ cao giúp Arduino gửi dữ liệu nhanh cho LED Matrix.
#include <SoftwareSerial.h> //tạo cổng Serial mềm để dùng Bluetooth HC-05 trên chân A4, A5.

//    LCD1602 
LiquidCrystal LCD(13, 12, 11, 10, 9, 8); //Khởi tạo màn hình LCD 1602.
//Thứ tự chân: RS → 13, E → 12, D4 → 11, D5 → 10, D6 → 9, D7 → 8.

//    LED Matrix (8x32 = 4 module MAX7219) 
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW //Thư viện MD_MAX72XX và MD_Parola sẽ điều khiển LED Matrix theo chuẩn FC-16.
#define MAX_DEVICES 4 


//Khai báo chân SPI analog dùng cho matrix
#define DATA_PIN A0
#define CS_PIN A1
#define CLK_PIN A2

//Đây là dòng khởi tạo đối tượng matrix
MD_Parola matrix = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES); //tham số

//    Bluetooth HC-05 (dùng A4, A5) 
SoftwareSerial BT(A4, A5);  // khai báo cái j nói ra: 
//A4 = RX (nhận)
//A5 = TX (gửi)
//Tạo cổng serial mềm tên BT để giao tiếp với HC-05.


//    Cảm biến siêu âm + LED + buzzer
const int TRIG_PIN = 2; //gửi xung
const int ECHO_PIN = 7; // nhận tín hiệu
const int buzzer = 3; // chân điều kiểu loa kêu tắt
//tạo biến hằng (constant) để đặt tên cho chân cắm trên Arduino:


//    Quang trở (LDR)
#define LDR_PIN A3 //chân analog đọc ánh sáng môi trường
int brightness = 3;//dùng chỉnh độ sáng LED matrix
int lastBrightness = 3; //lưu giá trị cũ để tránh update liên tục
String prevState = ""; //để tránh gửi trùng dữ liệu qua Bluetooth

float cm = 0; //khoảng cách đo được
unsigned long lastBlink = 0;//thời điểm lần cuối nháy LED
bool blinkState = false; // trạng thái chớp/tắt

//    Hàm đo khoảng cách trả về cm (chuẩn)
float distanceCM() {
  digitalWrite(TRIG_PIN, LOW); //chân gửi xung đang tắt
  delayMicroseconds(2); //Chờ 2 micro-giây để ổn định.
  digitalWrite(TRIG_PIN, HIGH); // gửi xung nhịp
  delayMicroseconds(10); // gửi xong 10 micro-giây
  digitalWrite(TRIG_PIN, LOW); // ngừng gửi xung nhịp

  //pulseIn(pin, value, timeout)
  //pulseIn(chân digital để đọc tín hiệu, mức cần đo: HIGH hoặc LOW, thời gian tối đa chờ (microseconds). Nếu hết thời gian → trả về 0)
  long duration = pulseIn(ECHO_PIN, HIGH, 40000); //đo thời gian ECHO ở mức HIGH (trả về thời gian chân đó ở mức HIGH)
  //ECHO_PIN đang high nhận đc xung gửi về thì chuyển thành low
  
  if (duration == 0) return 0;
  return duration * 0.0343 / 2; //0.0343 cm/µs = tốc độ âm thanh
}

//    Gửi dữ liệu sang điện thoại
void sendToPhone(float cm, String state, int ldrValue, int brightness) {
  BT.print("DIST:");// print() -> đt nhận đc qua bluetube và hiển thị ra ..
  BT.print(cm, 1); // 1 chữ số sau dấu phẩy
  BT.print("cm | STATE:");
  BT.print(state);
  BT.print(" | LDR:");
  BT.print(ldrValue);
  BT.print(" | BR:");
  BT.println(brightness);
}
//Gửi thông tin (khoảng cách, trạng thái, giá trị LDR, độ sáng) về điện thoại qua Bluetooth HC-05

//    Xử lý lệnh từ điện thoại
void handleBluetoothCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();  //Tự động chuyển thành chữ HOA

  //Khi điện thoại gửi "DIST" → trả về khoảng cách hiện tại
  if (cmd == "DIST") {
    BT.print("DIST: ");
    BT.print(cm, 1);
    BT.print(" cm");
    return;
  }

  if (cmd == "SAFE" || cmd == "WARN" || cmd == "SOS") {
    prevState = cmd; //Cập nhật trạng thái hệ thống vào prevState
    matrix.displayClear();// Xoá màn hình Matrix
    matrix.displayText(cmd.c_str(), PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    //matrix.displayText(
    //cmd.c_str(),   // 1. Chuỗi cần hiển thị
    //PA_CENTER,     // 2. Căn lề (center)
    //0,             // 3. Tốc độ cuộn (0 = không cuộn) -- tốc đọ chạy chữ
    //0,             // 4. Thời gian dừng (pause time) = 0 -- chữ hiện ra ngay lập tức, không delay.
    //PA_PRINT,      // 5. Kiểu hiển thị
    //PA_NO_EFFECT   // 6. Hiệu ứng xuất hiện
    //);
    matrix.displayAnimate();
    return;
  }

  if (cmd == "BUZON") {
    tone(buzzer, 1200); //Bật loa với tần số 1200 Hz.
    return;
  }

  if (cmd == "BUZOFF") {
    noTone(buzzer); // tắt loa
    return;
  }

  if (cmd.startsWith("LED:")) { //-> Điều khiển độ sáng Matrix: “LED:x”
    int value = cmd.substring(4).toInt(); // tách để lấy số ở cuối
    value = constrain(value, 0, 15); // Giới hạn 0–15 (MAX7219 chỉ hỗ trợ 0–15)
    brightness = value;
    matrix.setIntensity(brightness); // Set độ sáng bằng matrix.setIntensity()
    return;
  }

  if (cmd.startsWith("TEXT:")) {
    String text = cmd.substring(5);
    matrix.displayClear();
    matrix.displayScroll(text.c_str(), PA_CENTER, PA_SCROLL_LEFT, 25);
    //Cho chữ chạy từ phải sang trái (PA_SCROLL_LEFT)
    //Tốc độ scroll = 25ms/step
    return;
  }

  if (cmd == "BR") {
    BT.print("BR:");
    BT.println(brightness);
    return;
  }
}

//===================== SETUP =========================//
void setup() {// là hàm chạy 1 lần duy nhất, dùng để khởi tạo trc chương trình chính chạy
  pinMode(TRIG_PIN, OUTPUT);// cấu hình chân TRIG_PIN là gửi xung 
  pinMode(ECHO_PIN, INPUT);// cấu hình chân ECHO_PIN là nhận tín hiệu
  pinMode(6, OUTPUT);  // LED đỏ
  pinMode(5, OUTPUT);  // LED vàng
  pinMode(4, OUTPUT);  // LED xanh
  pinMode(buzzer, OUTPUT); // → để bật / tắt âm báo
  pinMode(LDR_PIN, INPUT); // ->đọc ánh sáng (0–1023)

  //Khởi tạo Serial và Bluetooth
  Serial.begin(9600); // tốc độ truyền dữ liệu 9600, giúo dùng đc các lệnh Serial.print(), Serial.println(), Serial.read(), Serial.available()
  BT.begin(9600); //để giao tiếp với HC-05
  BT.println("Bluetooth Connected!");

  //Khởi tạo LCD 1602
  LCD.begin(16, 2);// khởi tạo LCD 16 cột × 2 dòng
  LCD.setCursor(0, 0);//Con trỏ đặt tại (0,0) = dòng 1 – cột 1
  LCD.print("Water Level:");//Hiển thị dòng chữ ban đầu "Water Level:"

  //Khởi tạo Matrix
  matrix.begin(); //khởi động thư viện Parola
  matrix.setIntensity(3);//độ sáng ban đầu (0–15)
  matrix.displayClear();
  matrix.displayText("READY", PA_CENTER, 0, 1000, PA_PRINT, PA_NO_EFFECT);//1000 ms
  matrix.displayAnimate();
}


//===================== LOOP =========================//
void loop() {
  //   LDR- quang trở
  int ldrValue = analogRead(LDR_PIN); //Đọc giá trị ánh sáng từ cảm biến LDR
  ldrValue = constrain(ldrValue, 0, 1023);// ép giá trị về 0 -> 1023
  brightness = map(ldrValue, 0, 1023, 15, 1);//Chuyển đổi giá trị ánh sáng thành độ sáng LED Matrix:
  matrix.setIntensity(brightness);//Gán độ sáng mới cho LED Matrix MAX7219.

  //   Đo khoảng cách
  cm = distanceCM();

  LCD.setCursor(0, 1);
  LCD.print("     ");// để xóa LCD, xóa nd hiển thị cũ từ vòng lặp trước
  LCD.setCursor(0, 1);
  LCD.print(cm, 1);
  LCD.print("cm ");

  String state;

  //   Phân loại mức nước
  if (cm >= 20) {
    state = "SAFE";
    digitalWrite(6, LOW);
    digitalWrite(5, LOW);
    digitalWrite(4, HIGH);// đèn tương ứng với chân 4 sẽ sáng
    noTone(buzzer);
  } else if (cm >= 10) {
    state = "WARN";
    digitalWrite(6, LOW);
    digitalWrite(5, HIGH);
    digitalWrite(4, LOW);
    noTone(buzzer);
  } else {
    state = "SOS";
    digitalWrite(6, HIGH);
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    tone(buzzer, 1000);
    delay(200); //200 ms = 0.2 giây
    noTone(buzzer);
  }

  //   Khi trạng thái thay đổi
  if (state != prevState) {
    matrix.displayClear();

    if (state == "SAFE") {
      matrix.displayScroll("SAFE", PA_CENTER, PA_SCROLL_LEFT, 25);
    } else if (state == "WARN") {
      matrix.displayText("WARN", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
      matrix.displayAnimate();
    } else if (state == "SOS") {
      matrix.displayText("SOS", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
      matrix.displayAnimate();
    }

    prevState = state;
    lastBlink = millis();
    blinkState = false;

    //   Chỉ gửi dữ liệu khi "STATE" thay đổi
    sendToPhone(cm, state, ldrValue, brightness);
  }

  //   Hiệu ứng nhấp nháy theo trạng thái
  if (state == "SAFE") {
    if (matrix.displayAnimate()) {
      matrix.displayScroll("SAFE", PA_CENTER, PA_SCROLL_LEFT, 25);
    }
  } else if (state == "WARN") {
    if (millis() - lastBlink > 1000) {
      blinkState = !blinkState;
      matrix.setIntensity(blinkState ? brightness / 4 : brightness);
      lastBlink = millis();
    }
  } else if (state == "SOS") {
    if (millis() - lastBlink > 300) {
      blinkState = !blinkState;
      matrix.setIntensity(blinkState ? brightness / 6 : brightness);
      lastBlink = millis();
    }
  }

  //   Nhận lệnh Bluetooth
  if (BT.available()) {//Kiểm tra xem cổng Bluetooth HC-05 có dữ liệu gửi đến hay không.
    String cmd = BT.readStringUntil('\n');//Đọc một dòng lệnh đầy đủ từ điện thoại đến khi gặp ký tự xuống dòng \n.
    handleBluetoothCommand(cmd);
  }

  if (BT.available()) {
    String cmd = BT.readStringUntil('\n');
    handleBluetoothCommand(cmd);
  }

  delay(100);
}