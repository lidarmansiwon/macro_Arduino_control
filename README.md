# macro_Arduino_control  
This repository is made to organize the macro_Arduino_control code.  
이 저장소는 macro_Arduino_control 코드를 정리하기 위해 만들어짐.

## 1. Macro Arduino Control 에 관하여  
### About Macro Arduino Control
- 부경대학교 해양제어로보틱스 연구실(MACRO)에서 사용됨.  
- Used in the Marine Control Robotics Laboratory (MACRO) at Pukyong National University.
- USV 제어를 위한 Arduino 코드.  
- Arduino code for controlling USVs (Unmanned Surface Vehicles).
- 3대의 자율운항 선박 제어용으로 작성됨.  
- Designed to control three autonomous surface vessels.
- MARK3_HULL_Hole, MARK4_Line_HULL, MARK5_NEW_HULL 코드 포함.  
- Includes codes for MARK3_HULL_Hole, MARK4_Line_HULL, and MARK5_NEW_HULL vessels.

## 2. 선박별 코드 설명  
### Code Description by Vessel Type

### MARK3_HULL_Hole
- 구멍 뚫린 선박용 코드.  
- Code for vessel with a perforated hull.
- 구형 조종기 사용.  
- Uses an older model controller.

### MARK4_Line_HULL
- 2024 PASS 대회 참가 선박.  
- Vessel for the 2024 PASS competition.
- NX6 조종기 사용.  
- Uses the NX6 controller.
- PORT, STBD PWM과 RUDDER PWM 사용하여 기동.  
- Operates with PORT, STBD, and RUDDER PWM.
- 채널 5번으로 Auto / RC 모드 전환.  
- Channel 5 switches between Auto and RC modes.

## 3. 향후 추가 예정  
### Upcoming Additions
- MARK5_NEW_HULL 코드 설명 추가 예정.  
- Description for MARK5_NEW_HULL will be added.

---

## 📂 Repository Structure
macro_Arduino_control/ │ ├── MARK3_HULL_Hole/ │ └── Arduino code for MARK3 │ ├── MARK4_Line_HULL/ │ └── Arduino code for MARK4 │ ├── MARK5_NEW_HULL/ │ └── (To be updated) │ └── README.md


## 🔧 Requirements
- Arduino IDE
- 호환되는 Arduino 보드 (Uno, Nano 등).  
- Compatible Arduino board (Uno, Nano, etc.)
- 조종기: 구형 조종기, NX6 사용.  
- Controllers: Legacy controller, NX6.


## 📞 Contact
- 부경대학교 해양제어로보틱스 연구실 (MACRO)  
- Marine Control Robotics Laboratory (MACRO), Pukyong National University

