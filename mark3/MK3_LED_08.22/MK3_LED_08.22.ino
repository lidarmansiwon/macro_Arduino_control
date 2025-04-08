/// 2024.05.02 
/// Author: SIWON, SEONHYUK, SEONHYUN
/// This code is the control code for MARK3 ships.
/// Vessel configuration: 2 thrusters, LED, physical emergency power supply, rc_2 receiver, os1
/// New controller 

#include <Servo.h>
#include <Adafruit_NeoPixel.h>

#define RECV_mode 2  // channel 6. Auto & RC mode
#define Throttle2 12 // channel . rudder
#define Throttle0 13 // channel 1. thruster(PORT)
#define Throttle1 11 // channel 2. thruster(STBD)

byte Thr_pin1 = 8; //port
byte Thr_pin2 = 9; //stbd

String DataToESC;    //ESC에 전달할 PWM
String DataTOSend;   //아두이노가 보낼 신호
String RecieveData;  //아두이노가 받은 PWM
String DataFromRC;   // RC수신기에서 보낸 PWM
String TimeNow;
int Status = 1;  // ROS2 파싱 X

int thr_min = 1250;
int thr_max = 1750;
int thr_min_auto = 1100;
int thr_max_auto = 1900;
Servo thr1;
Servo thr2;

//LED
int num_LED_F_B = 10;
int num_LED_L_R = 30;

#define LED_PIN_F  4
#define LED_PIN_B  5
#define LED_PIN_L  6
#define LED_PIN_R  7

Adafruit_NeoPixel strip_F = Adafruit_NeoPixel(num_LED_F_B, LED_PIN_F, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_B = Adafruit_NeoPixel(num_LED_F_B, LED_PIN_B, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_L = Adafruit_NeoPixel(num_LED_L_R, LED_PIN_L, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_R = Adafruit_NeoPixel(num_LED_L_R, LED_PIN_R, NEO_GRB + NEO_KHZ800);

// 타이머 변수
unsigned long previousMillisBlink = 0;
unsigned long previousMillisWipe = 0;

// 타이밍 간격
const long intervalBlink = 2000; // 점멸 간격 (500ms)
const long intervalWipe = 2000;   // LED 하나씩 켜기 간격 (50ms)

// LED 상태 변수
bool blinkState = false;
int currentWipeIndex_L = 0;
int currentWipeIndex_R = 0;
int port_pwm; int stbd_pwm;
void setup() {
  pinMode(RECV_mode, INPUT);
  pinMode(Throttle2, INPUT);
  pinMode(Throttle1, INPUT);
  pinMode(Throttle0, INPUT);

  thr1.attach(Thr_pin1);
  thr2.attach(Thr_pin2);
  thr2.writeMicroseconds(1500);
  thr1.writeMicroseconds(1500);
  delay(1000);
  Serial.begin(500000);

  strip_F.begin();
  strip_F.show(); // Initialize all pixels to 'off'
  
  strip_B.begin();
  strip_B.show();
  
  strip_L.begin();
  strip_L.show();
  
  strip_R.begin();
  strip_R.show();
  
}

void loop() {
  unsigned long currentMillis = millis();

  float sum1 = 0;
  float sum2 = 0;
  float sum3 = 0;
  float sum4 = 0;

  int cnt = 0;
  int count = 2;  // 이동평균필터의 분모
  int avg1, avg2, avg3, avg4;
  while (cnt < count) {
    sum1 += pulseIn(Throttle1, HIGH, 50000);
    sum2 += pulseIn(RECV_mode, HIGH, 50000);
    sum3 += pulseIn(Throttle2, HIGH, 50000);
    sum4 += pulseIn(Throttle0, HIGH, 50000);

    cnt++;
  }
  avg1 = sum1 / count; //EM Stop 1500
  avg2 = sum2 / count; // Mode switch 1200 1500 1800
  avg3 = sum3 / count; //Rud
  avg4 = sum4 /count; // Thr
  
  int rudder_abs = 1500-avg3;
  
  if (rudder_abs>0){
    port_pwm = avg4+abs(rudder_abs);
    stbd_pwm = avg4-abs(rudder_abs);
  } else if (rudder_abs<0){
    port_pwm = avg4-abs(rudder_abs);
    stbd_pwm = avg4+abs(rudder_abs);
  } else {
    port_pwm = 1500;
    stbd_pwm = 1500;
  }
   
  int a;
  DataFromRC = String(port_pwm) + "," + String(avg1) + "," + String(avg3) + "," + String(stbd_pwm);

  

  //시리얼 통신으로 데이터 송신
  if (Serial.availableForWrite()) {
    DataTOSend = DataToESC + "," + String(Status) + "," + DataFromRC ;
    
    Serial.println(DataTOSend);
  }

  if (Status == 0) {
    if (avg1 <1100) {  // 비상정지 모드
    a = 1;            // Stop mode
    }
    else {
    a = 2;            //RC mode
    }
  }
  else{
    if (avg1 <1200) {  // 비상정지 모드
    a = 1;            // Stop mode
    }
    else if (1200 <= avg1 <= 1600){
    a = 2;            //RC mode
    }
    else if (1600 < avg1){
    a = 3;            // Auto mode
  }

  }
  if (currentMillis - previousMillisBlink >= intervalBlink) {
    previousMillisBlink = currentMillis;
    if (a==1) { 
      strip_F.fill(strip_F.Color(255, 0, 0)); // 빨간색
      strip_B.fill(strip_B.Color(255, 0, 0)); // 녹색
    }
    else if (a==2) {
      strip_F.fill(strip_F.Color(255, 255, 0)); // 빨간색
      strip_B.fill(strip_B.Color(255, 255, 0)); // 녹색 
    }
    else {
      strip_F.fill(strip_F.Color(0, 255, 0)); // 빨간색
      strip_B.fill(strip_B.Color(0, 255, 0)); // 녹색 

    }
    strip_F.show();
    strip_B.show();
  }
  if (currentMillis - previousMillisWipe >= intervalWipe) {
    previousMillisWipe = currentMillis;
    
    if (a==1) {
    strip_L.setPixelColor(currentWipeIndex_L, strip_L.Color(255, 0, 0)); // 파란색
    strip_L.setPixelColor(currentWipeIndex_L+1, strip_L.Color(255, 0, 0));
    strip_L.setPixelColor(currentWipeIndex_L+2, strip_L.Color(255, 0, 0));
    strip_L.show();

    strip_R.setPixelColor(currentWipeIndex_R, strip_R.Color(255, 0, 0)); // 노란색
    strip_R.setPixelColor(currentWipeIndex_R+1, strip_R.Color(255, 0, 0));
    strip_L.setPixelColor(currentWipeIndex_R+2, strip_R.Color(255, 0, 0));
    strip_R.show();

    currentWipeIndex_L+=3;
    if (currentWipeIndex_L >= strip_L.numPixels()) {
      currentWipeIndex_L = 0; // Reset index to start over
      strip_L.clear(); 
    }
    
    currentWipeIndex_R+=3;
    if (currentWipeIndex_R >= strip_R.numPixels()) {
      currentWipeIndex_R = 0; // Reset index to start over
      strip_R.clear(); 
    }
    }
    
    else if (a==2) {
    strip_L.setPixelColor(currentWipeIndex_L, strip_L.Color(255, 255, 0)); // 파란색
    strip_L.setPixelColor(currentWipeIndex_L+1, strip_L.Color(255, 255, 0));
    strip_L.setPixelColor(currentWipeIndex_L+2, strip_L.Color(255, 255, 0));
    strip_L.show();

    strip_R.setPixelColor(currentWipeIndex_R, strip_R.Color(255, 255, 0)); // 노란색
    strip_R.setPixelColor(currentWipeIndex_R+1, strip_R.Color(255, 255, 0));
    strip_L.setPixelColor(currentWipeIndex_L+2, strip_R.Color(255, 255, 0));
    strip_R.show();

    currentWipeIndex_L+=3;
    if (currentWipeIndex_L >= strip_L.numPixels()) {
      currentWipeIndex_L = 0; // Reset index to start over
      strip_L.clear(); 
    }
    
    currentWipeIndex_R+=3;
    if (currentWipeIndex_R >= strip_R.numPixels()) {
      currentWipeIndex_R = 0; // Reset index to start over
      strip_R.clear(); 
    }
    }
    
    else {
    strip_L.setPixelColor(currentWipeIndex_L, strip_L.Color(0, 255, 0)); // 파란색
    strip_L.setPixelColor(currentWipeIndex_L+1, strip_L.Color(0, 255, 0)); // 파란색
    strip_L.setPixelColor(currentWipeIndex_L+2, strip_L.Color(0, 255, 0));
    strip_L.show();

    strip_R.setPixelColor(currentWipeIndex_R, strip_R.Color(0, 255, 0)); // 노란색
    strip_R.setPixelColor(currentWipeIndex_R+1, strip_R.Color(0, 255, 0));
    strip_R.setPixelColor(currentWipeIndex_R+2, strip_R.Color(0, 255, 0));
    strip_R.show();

    currentWipeIndex_L+=3;
    if (currentWipeIndex_L >= strip_L.numPixels()) {
      currentWipeIndex_L = 0; // Reset index to start over
      strip_L.clear(); 
    }
    
    currentWipeIndex_R+=3;
    if (currentWipeIndex_R >= strip_R.numPixels()) {
      currentWipeIndex_R = 0; // Reset index to start over
      strip_R.clear(); 
    }
    }  
    
    }
    //시리얼 통신으로 데이터 수신
  if (Serial.available() > 0) {
    String RecieveData = Serial.readStringUntil('\n');  // 줄 바꿈 문자('\n')까지 읽음
    
    // 문자열을 파싱하여 숫자로 변환
    int value1, value2; float SerialTime;
    sscanf(RecieveData.c_str(), "%d,%d,%d", &value1, &value2,&SerialTime);
    value1 = constrain(value1, thr_min_auto, thr_max_auto);
    value2 = constrain(value2, thr_min_auto, thr_max_auto);
    TimeNow = SerialTime;
    thr1.writeMicroseconds(value2) ;
    thr2.writeMicroseconds(value1);

    //DataToESC = String(value1) + "," + String(value2) + "," + String(value3);
    DataToESC = String(value1) + "," + String(value2);
    Status = 1;  //ROS2 파싱중
  }
  else {
    // 수신된 데이터가 없을 때 기본 PWM 값을 사용
    port_pwm = constrain(port_pwm, thr_min, thr_max);
    stbd_pwm = constrain(stbd_pwm, thr_min, thr_max);
    thr1.writeMicroseconds(port_pwm);
    thr2.writeMicroseconds(stbd_pwm);
    DataToESC = String(1500) ;
    Status = 0;  //ROS2 파싱 X

  }
  }
