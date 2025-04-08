/// 2024.05.02
/// EDIT 09.21 ## add Relay 24V
/// EDIT 2025.03.13 ## del ALL Relay 
/// Author: SIWON, SEONHYUK, SEONHYUN
/// This code is the control code for MARK3 ships.
/// Vessel configuration: 2 thrusters, LED, physical emergency power supply, rc_2 receiver, os1
/// New controller 

#include <Servo.h>

#define RECV_mode 2  // Send Mode data from PWM control Arduino

#define Throttle2 12 // channel . rudder
#define Throttle0 13 // channel 1. thruster(STBS)
#define Throttle1 11 // channel 2. em

byte Thr_pin1 = 8; //port
byte Thr_pin2 = 9; //stbd

String DataToESC;    //ESC에 전달할 PWM
String DataTOSend;   //아두이노가 보낼 신호
String RecieveData;  //아두이노가 받은 PWM
String DataFromRC;   // RC수신기에서 보낸 PWM
int Status = 1;  // ROS2 파싱 X

int thr_min = 1250;
int thr_max = 1750;
int thr_min_auto = 1100;
int thr_max_auto = 1900;
Servo thr1;
Servo thr2;
Servo Copy1; Servo Copy2;
int port_pwm; int stbd_pwm;
void setup() {
  pinMode(RECV_mode, INPUT);
  pinMode(Throttle2, INPUT);
  pinMode(Throttle1, INPUT);
  pinMode(Throttle0, INPUT);
//  pinMode(COPY_PIN1, OUTPUT); // 7pin OUTPUT
//  pinMode(COPY_PIN2, OUTPUT);
  thr1.attach(Thr_pin1);
  thr2.attach(Thr_pin2);
  thr2.writeMicroseconds(1500);
  thr1.writeMicroseconds(1500);
//  Copy1.attach(COPY_PIN1);
//  Copy2.attach(COPY_PIN2);
  delay(1000);
  Serial.begin(57600);
  
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

//  // RECV_mode COPY_PIN 7 
//  Copy1.writeMicroseconds(avg2);
//  Copy2.writeMicroseconds(avg1);
    
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
  port_pwm = ((port_pwm + 5) / 10) * 10; 
  stbd_pwm = ((stbd_pwm + 5) / 10) * 10;  

  DataFromRC = String(port_pwm) + "," + String(avg2) + "," + String(avg3) + "," + String(stbd_pwm);

  

  //시리얼 통신으로 데이터 송신
  if (Serial.availableForWrite()) {
    DataTOSend = DataToESC + "," + String(Status) + "," + DataFromRC ;
    
    Serial.println(DataTOSend);
  }


    //시리얼 통신으로 데이터 수신
  if (Serial.available() > 0) {
    String RecieveData = Serial.readStringUntil('\n');  // 줄 바꿈 문자('\n')까지 읽음
    
    // 문자열을 파싱하여 숫자로 변환
    int value1, value2;
    sscanf(RecieveData.c_str(), "%d,%d,%d", &value1, &value2);
    value1 = constrain(value1, thr_min_auto, thr_max_auto);
    value2 = constrain(value2, thr_min_auto, thr_max_auto);
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
