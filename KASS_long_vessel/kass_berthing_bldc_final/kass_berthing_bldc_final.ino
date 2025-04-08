#include <Servo.h>
#include <Adafruit_NeoPixel.h>

// Define Reciver pin

#define RECV_btn 2  //ch3
#define RECV_mode 3 //ch2
#define RECV_PWM1 4 //ch4
#define RECV_PWM2 9 //ch1

// Define LED Status
#define LED_Pin 13
#define NUMPIXELS 35
Adafruit_NeoPixel pixels(NUMPIXELS, LED_Pin, NEO_GRB + NEO_KHZ800);

byte bldc1_pin = 5 ;
byte bldc2_pin = 6;

byte Thr_pin = 10;

uint32_t redColor = pixels.Color(255, 0, 0);       // 빨간색
uint32_t greenColor = pixels.Color(0, 255, 0);     // 녹색
uint32_t blueColor = pixels.Color(0, 0, 255);      // 파란색
uint32_t yellowColor = pixels.Color(255, 255, 0);  // 노란색

String DataToESC;    //ESC에 전달할 PWM
String DataTOSend;   //아두이노가 보낼 신호
String RecieveData;  //아두이노가 받은 PWM
String DataFromRC;   // RC수신기에서 보낸 PWM
int Status = 1;  // ROS2 파싱 X

int thr_min = 1350;
int thr_max = 1650;
int bldc_min = 1100;
int bldc_max = 1900;
Servo thr;
Servo bldc1;
Servo bldc2;

void setup() {
  pixels.begin();

  pinMode(RECV_btn, INPUT);
  pinMode(RECV_mode, INPUT);
  pinMode(RECV_PWM1, INPUT);
  pinMode(RECV_PWM2, INPUT);

  thr.attach(Thr_pin);
  thr.writeMicroseconds(1500);
  bldc1.attach(bldc1_pin);
  bldc2.attach(bldc2_pin);
  bldc1.writeMicroseconds(1500);
  bldc2.writeMicroseconds(1500);
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
  int r_bldc_minus ;
  while (cnt < count) {
    sum1 += pulseIn(RECV_btn, HIGH, 50000); // on/off(red&yellow)
    sum2 += pulseIn(RECV_mode, HIGH, 50000);// RC&AUTO
    sum3 += pulseIn(RECV_PWM1, HIGH, 50000);// Throttle
    sum4 += pulseIn(RECV_PWM2, HIGH, 50000);// Rudder

    cnt++;
  }
  avg1 = sum1 / count;
  avg2 = sum2 / count;
  avg3 = sum3 / count;
  avg4 = sum4 / count;
  int a;
  DataFromRC = String(avg1) + "," + String(avg2) + "," + String(avg3) + "," + String(avg4);
  if (avg1 > 1600) {  // 비상정지 모드

    //setColor(redColor);
    a = 1;


  } else {
    if (avg2 > 1800) {
      a = 2;

    }
    else {

      a = 3;
    }
  }
  //시리얼 통신으로 데이터 수신
  if (Serial.available() > 0) {
    String RecieveData = Serial.readStringUntil('\n');  // 줄 바꿈 문자('\n')까지 읽음
    // 문자열을 파싱하여 숫자로 변환
    int value1, value4, value5;
    sscanf(RecieveData.c_str(), "%d,%d,%d", &value1, &value4, &value5);
    value1 = constrain(value1, thr_min, thr_max);
    value4 = constrain(value4, bldc_min, bldc_max);
    value5 = constrain(value5, bldc_min, bldc_max);
    r_bldc_minus = 1500 - value5 ;
    if (r_bldc_minus >= 0) {
      value5 = 1500 + abs(r_bldc_minus);
    }
    else {
      value5 = 1500 - abs(r_bldc_minus);
    }

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
    thr.writeMicroseconds(value1);

    bldc1.writeMicroseconds(value4);
    bldc2.writeMicroseconds(value5);

    DataToESC = String(value1) + "," + String(value4) + "," + String(value5);
    Status = 1;  //ROS2 파싱중
  }
  else {
    // 수신된 데이터가 없을 때 기본 PWM 값을 사용
    avg3 = constrain(avg3, thr_min, thr_max);
    avg4 = constrain(avg4, bldc_min, bldc_max);

    if (a == 1) {
      setColor(redColor);

    } else {
      setColor(yellowColor);
    }
    //setColor(yellowColor );
    thr.writeMicroseconds(avg3);

    bldc1.writeMicroseconds(avg4);

    r_bldc_minus = 1500 - avg4;
    if (r_bldc_minus >= 0) {
      avg4 = 1500 + abs(r_bldc_minus);
    }
    else {
      avg4 = 1500 - abs(r_bldc_minus);
    }

    bldc2.writeMicroseconds(avg4);

    DataToESC = String(1500) + "," + String(0) + "," + String(0);
    Status = 1;  //ROS2 파싱 X

  }

  //시리얼 통신으로 데이터 송신
  if (Serial.availableForWrite()) {
    DataTOSend = DataToESC + "," + String(Status) + "," + DataFromRC;
    Serial.println(DataTOSend);
  }
  //delay(100);
}

// LED에 색상을 설정하는 함수
void setColor(uint32_t color) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, color);  // 지정된 색상으로 설정
  }
  pixels.show();  // 변경사항 적용
}
