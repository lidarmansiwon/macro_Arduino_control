/// 2024.05.02 
/// Author: SIWON, SEONHYUK, SEONHYUN
/// This code is the control code for MAST_2 ships.
/// Vessel configuration: 2 thrusters, LED, relay emergency power system, orin_no.2, 2 servo motor, new receiver

#include <Servo.h>
#include <Adafruit_NeoPixel.h>

// Define Reciver pin

#define RECV_btn 3   // channel 5. emergency on&off
#define RECV_mode 2  // channel 1. Auto & RC mode
#define RECV_PWM1 12 // channel 2. servo motor
#define RECV_PWM2 13 // channel 3. thruster

// Define LED Status
#define LED_Pin 7
#define NUMPIXELS 10
Adafruit_NeoPixel pixels(NUMPIXELS, LED_Pin, NEO_GRB + NEO_KHZ800);
// Define motor pin
byte Servo_pin1 = 4; //port
byte Servo_pin2 = 5; //stbd

byte Thr_pin1 = 8; //port
byte Thr_pin2 = 9; //stbd

#define Relay_1 11

uint32_t redColor = pixels.Color(255, 0, 0);       // 빨간색
uint32_t greenColor = pixels.Color(0, 255, 0);     // 녹색
uint32_t blueColor = pixels.Color(0, 0, 255);      // 파란색
uint32_t yellowColor = pixels.Color(255, 255, 0);  // 노란색

String DataToESC;    //ESC에 전달할 PWM
String DataTOSend;   //아두이노가 보낼 신호
String RecieveData;  //아두이노가 받은 PWM
String DataFromRC;   // RC수신기에서 보낸 PWM
int ServoDefalut1 = 90;
int ServoDefalut2 = 90;
int Status = 1;  // ROS2 파싱 X

int thr_min = 1250;
int thr_max = 1750;
Servo thr1;
Servo thr2;
Servo servo1;
Servo servo2;
//const float alpha = 0.4;
//float filter_RC_1 = 0;
//float filter_RC_2 = 0;
//float filter_RC_3 = 0;
//float filter_RC_4 = 0;
void setup() {
  pixels.begin();
  pixels.setBrightness(15);
  pinMode(RECV_btn, INPUT);
  pinMode(RECV_mode, INPUT);
  pinMode(RECV_PWM1, INPUT);
  pinMode(RECV_PWM2, INPUT);

  pinMode(Relay_1, OUTPUT);
  digitalWrite(Relay_1, HIGH);
  thr1.attach(Thr_pin1,1100,1900);
  thr2.attach(Thr_pin2,1100,1900);
  servo1.attach(Servo_pin1);
  servo2.attach(Servo_pin2);
  thr2.writeMicroseconds(1500);
  thr1.writeMicroseconds(1500);
  servo1.write(ServoDefalut1);
  servo2.write(ServoDefalut2);
  delay(1000);
  Serial.begin(57600);
}

void loop() {
  float sum1 = 0;
  float sum2 = 0;
  float sum3 = 0;
  float sum4 = 0;

  int cnt = 0;
  int count = 2;  // 이동평균필터의 분모
  int avg1, avg2, avg3, avg4;
  while (cnt < count) {
    sum1 += pulseIn(RECV_btn,  HIGH, 50000);
    sum2 += pulseIn(RECV_mode, HIGH, 50000);
    sum3 += pulseIn(RECV_PWM1, HIGH, 50000);
    sum4 += pulseIn(RECV_PWM2, HIGH, 50000);

    cnt++;
  }
  avg1 = sum1 / count;
  avg2 = sum2 / count;
  avg3 = sum3 / count;
  avg4 = sum4 / count;
  //avg1 = alpha * sum1 +(1-alpha) * avg1;
  //avg2 = alpha * sum2 +(1-alpha) * avg2;
  //avg3 = alpha * sum3 +(1-alpha) * avg3;
  //avg4 = alpha * sum4 +(1-alpha) * avg4;
  int a;
  DataFromRC = String(avg1) + "," + String(avg2) + "," + String(avg3) + "," + String(avg4);
  if (avg1 > 1600) {  // 비상정지 모드

    //setColor(redColor);
    a = 1;


  } else {
    if (avg2 < 1600) {
      a = 2; //setColor()
    
    }
    else {

    a = 3; //setColor()
    }
  }
  //시리얼 통신으로 데이터 수신
  if (Serial.available() > 0) {
    String RecieveData = Serial.readStringUntil('\n');  // 줄 바꿈 문자('\n')까지 읽음
    // 문자열을 파싱하여 숫자로 변환
    int value1, value2, value3;
    sscanf(RecieveData.c_str(), "%d,%d,%d", &value1, &value2, &value3);
    value1 = constrain(value1, thr_min, thr_max);
    if (a == 1) {
      setColor(redColor);
    } else {
      if (a == 2) {
      setColor(greenColor);
      } else {
        setColor(yellowColor);
      }
    }
    //setColor(greenColor);
    thr1.writeMicroseconds(value1);
    thr2.writeMicroseconds(value1);
    servo1.write(value2 + 90);
    servo2.write(value3 + 90);
    DataToESC = String(value1) + "," + String(value2) + "," + String(value3);
    Status = 1;  //ROS2 파싱중
  } 
  else {
    // 수신된 데이터가 없을 때 기본 PWM 값을 사용
    avg4 = constrain(avg4, thr_min, thr_max);

    if (a == 1) {
      digitalWrite(Relay_1, HIGH);  // LOW가 OPEN 일 경우 LOW로 변경
      //digitalWrite(Relay_2, HIGH);
      setColor(redColor);
     
    } else {
      digitalWrite(Relay_1, LOW);  // LOW가 OPEN 일 경우 LOW로 변경
      //digitalWrite(Relay_2, LOW);
      setColor(yellowColor);
    }
    //setColor(yellowColor );
    thr1.writeMicroseconds(avg4);
    thr2.writeMicroseconds(avg4);
    avg3 = map(avg3, 1100,1900,-90,90);
    
    servo1.write(avg3 + 90);
    servo2.write(avg3 + 90);
    
    DataToESC = String(1500) + "," + String(ServoDefalut1) + "," + String(ServoDefalut2);
    Status = 1;  //ROS2 파싱 X

  }

  //시리얼 통신으로 데이터 송신
  if (Serial.availableForWrite()) {
    DataTOSend = DataToESC + "," + String(Status) + "," + DataFromRC;
    Serial.println(DataTOSend);
  }
}
// LED에 색상을 설정하는 함수
void setColor(uint32_t color) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, color);  // 지정된 색상으로 설정
  }
  pixels.show();  // 변경사항 적용
}
