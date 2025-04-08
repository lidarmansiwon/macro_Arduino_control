#include <Servo.h>
#include <Adafruit_NeoPixel.h>

// Define Reciver pin

#define RECV_btn 2  // engine Stop
#define RECV_mode 3 // RC & Auto
#define RECV_PWM1 4
#define RECV_PWM2 9

// Define LED Status
#define LED_Pin 6
#define NUMPIXELS 35
Adafruit_NeoPixel pixels(NUMPIXELS, LED_Pin, NEO_GRB + NEO_KHZ800);
// Define motor pin
byte Servo_pin1 = 8;
byte Servo_pin2 = 7;

byte DC1_PWM = 6;
byte DC_Interput = 14;  //Interupt 핀 모터 드라이버 1,2 점프
byte DC1_Dir = 15;

byte DC2_PWM = 5;
byte DC2_Dir = 17;

byte Thr_pin = 10;

//#define Relay_1 11  //전자 릴레이
//#define Relay_2 12  //전자 릴레이 2

int Relay_1 = 11;
int Relay_2 = 12;

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
Servo thr;
Servo servo1;
Servo servo2;

void setup() {
  pixels.begin();

  pinMode(RECV_btn, INPUT);
  pinMode(RECV_mode, INPUT);
  pinMode(RECV_PWM1, INPUT);
  pinMode(RECV_PWM2, INPUT);

  pinMode(Relay_1, OUTPUT);
  pinMode(Relay_2, OUTPUT);
  pinMode(DC_Interput, OUTPUT);
  pinMode(DC1_PWM, OUTPUT);
  pinMode(DC2_PWM, OUTPUT);
  pinMode(DC1_Dir, OUTPUT);
  pinMode(DC2_Dir, OUTPUT);
  digitalWrite(DC1_Dir,HIGH);
  digitalWrite(DC2_Dir,HIGH);
  digitalWrite(Relay_1, LOW);
  digitalWrite(Relay_2, LOW);
  digitalWrite(DC_Interput, LOW);  // DC Motor를 사용하려면 LOW

  thr.attach(Thr_pin);
  servo1.attach(Servo_pin1);
  servo2.attach(Servo_pin2);
  thr.writeMicroseconds(1500);
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
    sum1 += pulseIn(RECV_btn, HIGH, 50000);
    sum2 += pulseIn(RECV_mode, HIGH, 50000);
    sum3 += pulseIn(RECV_PWM1, HIGH, 50000);
    sum4 += pulseIn(RECV_PWM2, HIGH, 50000);

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
    int value1, value2, value3, value4, value5;
    sscanf(RecieveData.c_str(), "%d,%d,%d,%d,%d", &value1, &value2, &value3, &value4, &value5);
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
    thr.writeMicroseconds(value1);
    servo1.write(value2 + 90);
    servo2.write(value3 + 90);
    if (value4 >=0) {
      
      digitalWrite(DC1_Dir,HIGH);
    }
    else {
      digitalWrite(DC1_Dir,LOW);
      }
    if (value5 >=0) {
      digitalWrite(DC2_Dir,LOW);
    }
    else {
      digitalWrite(DC2_Dir,HIGH);
      }

    if (abs(value4) <= 12) {
      value4 = 0;
    }
    else if (abs(value5) <= 12) {
      value5 = 0;
    }
     
    analogWrite(DC1_PWM,abs(value4)); 
    analogWrite(DC2_PWM,abs(value5)); 
    DataToESC = String(value1) + "," + String(value2) + "," + String(value3) + "," + String(value4) + "," + String(value5);
    Status = 1;  //ROS2 파싱중
  } 
  else {
    // 수신된 데이터가 없을 때 기본 PWM 값을 사용
    avg3 = constrain(avg3, thr_min, thr_max);

    if (a == 1) {
      digitalWrite(Relay_1, HIGH);  // LOW가 OPEN 일 경우 LOW로 변경
      digitalWrite(Relay_2, HIGH);
      setColor(redColor);
     
    } else {
      digitalWrite(Relay_1, LOW);  // LOW가 OPEN 일 경우 LOW로 변경
      digitalWrite(Relay_2, LOW);
      setColor(yellowColor);
    }
    //setColor(yellowColor );
    thr.writeMicroseconds(avg3);
    servo1.write(avg3 + 90);
    servo2.write(avg3 + 90);
    avg4 = map(avg4, 1100,1900,-255,255) ; 

    if (avg4 > 10) {
      digitalWrite(DC2_Dir,LOW);
      digitalWrite(DC1_Dir,HIGH);
    }
    else if (abs(avg4) <= 12) {
      avg4 = 0;
    }
    else {
      digitalWrite(DC1_Dir,LOW);
      digitalWrite(DC2_Dir,HIGH);
      }
      
    analogWrite(DC2_PWM,abs(avg4)); 
    analogWrite(DC1_PWM,abs(avg4)); 
    
    DataToESC = String(1500) + "," + String(ServoDefalut1) + "," + String(ServoDefalut2) +"," + String(0) + "," + String(0);
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
