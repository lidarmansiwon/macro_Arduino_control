/// 2024.05.02 
/// 2024.09.21
/// Author: SIWON, SuNHYucK, SEONHYUN
/// This code is the control code for MARK3 ships.
/// Vessel configuration: 2 thrusters, LED, physical emergency power supply, rc_2 receiver, os1
/// New controller 

#include <Servo.h>
#include <Adafruit_NeoPixel.h>

#define RECV_mode 4  // channel 6. Auto & RC mode
#define RECV_mode2 2 // channel 5. Emergency STOP

#define COPY_PIN1 3   // 7 --> Send RECV_mode data to Main Arduino  
#define COPY_PIN2 8   // ?? --> Sebd EMS_mode data to Main Arduino
#define RELAY_PIN4 11  // 릴레이 제어 핀
//#define RELAY_PIN3 10
//#define RELAY_PIN2 9
//#define RELAY_PIN1 8


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
int a;
// 타이밍 간격
const long intervalBlink = 10; // 점멸 간격 (500ms)
const long intervalWipe = 50;   // LED 하나씩 켜기 간격 (50ms)

// LED 상태 변수
bool blinkState = false;
int currentWipeIndex_L = 0;
int currentWipeIndex_R = 0;
int port_pwm; int stbd_pwm;

Servo Copy1;
Servo Copy2;
void setup() {
  pinMode(RECV_mode, INPUT);
  pinMode(RECV_mode2, INPUT);
  pinMode(COPY_PIN1, OUTPUT); // 3pin OUTPUT
  pinMode(COPY_PIN2, OUTPUT);
  pinMode(RELAY_PIN4, OUTPUT);  // 릴레이 제어 핀 설정
//  pinMode(RELAY_PIN3, OUTPUT);
//  pinMode(RELAY_PIN2, OUTPUT);
//  pinMode(RELAY_PIN1, OUTPUT);
  Copy1.attach(COPY_PIN1);
  Copy2.attach(COPY_PIN2);
  delay(1000);
  Serial.begin(57600);

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

  int sum1 = 0;
  int sum2 = 0;
  int sum3 = 0;
  int sum4 = 0;

  int cnt = 0;
  int count = 2;  // 이동평균필터의 분모
  int avg1, avg2, avg3, avg4;
  while (cnt < count) {
    
    sum2 += pulseIn(RECV_mode2, HIGH, 50000);
    sum1 += pulseIn(RECV_mode, HIGH, 50000);
    cnt++;
  }
  
  avg1 = int(sum1 / count);
  avg2 = int(sum2 / count); // EMS

  Copy1.writeMicroseconds(avg1);
  Copy2.writeMicroseconds(avg2);

  Serial.print(avg2);
  
//  Serial.println(a);

  
  if (avg2 <1600){
    a = 1;
  }
  else {
  if (avg1 >1400){
    a=2;
  }
  if (avg1 >1700){
    a =3 ;
  }
  }
  
// Serial.println(a);
// Serial.println(avg1);

   // 릴레이 제어 로직
  if (a == 2 || a == 3) {
    digitalWrite(RELAY_PIN4, HIGH);  // 릴레이 ON (5V 공급)
//    digitalWrite(RELAY_PIN3, HIGH);
//    digitalWrite(RELAY_PIN2, HIGH);
//    digitalWrite(RELAY_PIN1, HIGH);
    Serial.println(a);
  } else if (a == 1) {
    digitalWrite(RELAY_PIN4, LOW);  // 릴레이 OFF (전류 차단)
///}
    Serial.println(avg1);
  }
  
   // a가 1 또는 2일 때 LED 전체 켜기
  if (a == 1) {
    strip_F.fill(strip_F.Color(255, 0, 0)); // F 전면 LED 빨간색
    strip_B.fill(strip_B.Color(255, 0, 0)); // B 후면 LED 빨간색
    strip_L.fill(strip_L.Color(255, 0, 0)); // L 좌측 LED 빨간색
    strip_R.fill(strip_R.Color(255, 0, 0)); // R 우측 LED 빨간색

    strip_F.show();
    strip_B.show();
    strip_L.show();
    strip_R.show();
  }

  else if (a==2) {
    strip_F.fill(strip_F.Color(0, 255, 0)); // F 전면 LED 빨간색
    strip_B.fill(strip_B.Color(0, 255, 0)); // B 후면 LED 빨간색
    strip_L.fill(strip_L.Color(0, 255, 0)); // L 좌측 LED 빨간색
    strip_R.fill(strip_R.Color(0, 255, 0)); // R 우측 LED 빨간색

    strip_F.show();
    strip_B.show();
    strip_L.show();
    strip_R.show();  
  }

  // a가 3일 때만 LED 깜빡임
  else if (a == 3) {
    if (currentMillis - previousMillisBlink >= intervalBlink) {
      previousMillisBlink = currentMillis;

      strip_F.fill(strip_F.Color(255, 255, 0)); // AUTO 모드에서 전면 LED 노란색
      strip_B.fill(strip_B.Color(255, 255, 0)); // AUTO 모드에서 후면 LED 노란색

      strip_F.show();
      strip_B.show();
    }

    if (currentMillis - previousMillisWipe >= intervalWipe) {
      previousMillisWipe = currentMillis;

      // 좌우 LED 깜빡임 구현
      strip_L.setPixelColor(currentWipeIndex_L, strip_L.Color(255, 255, 0)); 
      strip_R.setPixelColor(currentWipeIndex_R, strip_R.Color(255, 255, 0)); 

      strip_L.show();
      strip_R.show();

      currentWipeIndex_L += 3;
      if (currentWipeIndex_L >= strip_L.numPixels()) {
        currentWipeIndex_L = 0;  // 인덱스 리셋
        strip_L.clear();
      }

      currentWipeIndex_R += 3;
      if (currentWipeIndex_R >= strip_R.numPixels()) {
        currentWipeIndex_R = 0;  // 인덱스 리셋
        strip_R.clear();
      }
    }
  }
}
