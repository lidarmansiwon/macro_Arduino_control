//// 2024.11.06 --> 2025.3.13 
/// Author: SIWON, SEONHYUK, SEONHYUN
/// This code is the control code for MARK3 ships(MARK3 original ver to MARK3).
/// Vessel configuration: 2 thrusters, physical emergency power supply, NX8 receiver, os1
/// 

#include <Servo.h>

#define RECV_mode 4  // channel 5. Auto & RC mode
#define RUDDER 6 // channel 2. rudder
#define ThrottleP 7 // channel 3. thruster(PORT)
#define ThrottleS 5 // channel 4. thruster(STBD)

byte Thr_pin1 = 8; //port
byte Thr_pin2 = 9; //stbd

String DataToESC;    //ESC에 전달할 PWM
String DataTOSend;   //아두이노가 보낼 신호
String RecieveData;  //아두이노가 받은 PWM
String DataFromRC;   // RC수신기에서 보낸 PWM

int Status = 1;  // ROS2 파싱 X

int thr_min = 1250;
int thr_max = 1750;
Servo thrP;
Servo thrS;

void setup() {
  pinMode(RECV_mode, INPUT);
  pinMode(RUDDER, INPUT);
  pinMode(ThrottleP, INPUT);
  pinMode(ThrottleS, INPUT);

  thrP.attach(Thr_pin1);
  thrS.attach(Thr_pin2);
  thrP.writeMicroseconds(1500);
  thrS.writeMicroseconds(1500);
  delay(1000);
  Serial.begin(115200);
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
    sum1 += pulseIn(RECV_mode, HIGH, 50000);
    sum2 += pulseIn(RUDDER, HIGH, 50000);
    sum3 += pulseIn(ThrottleP, HIGH, 50000);
    sum4 += pulseIn(ThrottleS, HIGH, 50000);

    cnt++;
  }
  avg1 = sum1 / count;
  avg2 = sum2 / count;
  avg3 = sum3 / count;
  avg4 = sum4 / count;
  int a;
  DataFromRC = String(avg1) + "," + String(avg2) + "," + String(avg3) + "," + String(avg4);
  if (avg1 > 1600) {  // 비상정지 모드


    a = 1;            // RC mode


  } else {

    a = 2;            // Auto mode
  } 
  //시리얼 통신으로 데이터 수신
//  Serial.println(DataFromRC);
  if (Serial.available() > 0) {
    String RecieveData = Serial.readStringUntil('\n');  // 줄 바꿈 문자('\n')까지 읽음
    // 문자열을 파싱하여 숫자로 변환
    int value1, value2;
    sscanf(RecieveData.c_str(), "%d,%d", &value1, &value2);
    value1 = constrain(value1, thr_min, thr_max);
    value2 = constrain(value2, thr_min, thr_max);

    thrP.writeMicroseconds(value2) ;
    thrS.writeMicroseconds(value1);

    //DataToESC = String(value1) + "," + String(value2) + "," + String(value3);
    DataToESC = String(value1) + "," + String(value2);
    Status = 1;  //ROS2 파싱중
  }
  else {
    // 수신된 데이터가 없을 때 기본 PWM 값을 사용
    avg3 = constrain(avg3, thr_min, thr_max);
    avg4 = constrain(avg4, thr_min, thr_max);

    thrP.writeMicroseconds(avg3);
    thrS.writeMicroseconds(avg4);
    
    DataToESC = String(1500) + "," + String(1500);
    Status = 1;  //ROS2 파싱 X

  }

  //시리얼 통신으로 데이터 송신
  if (Serial.availableForWrite()) {
    DataTOSend = DataToESC + "," + String(Status) + "," + DataFromRC;
    Serial.println(DataTOSend);
  }
}
