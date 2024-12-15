#include <NewPing.h>

// 초음파 센서 핀 설정
#define TRIG_PIN1 8 // 초음파센서1 TRIG 핀
#define ECHO_PIN1 9 // 초음파센서1 ECHO 핀
#define TRIG_PIN2 6 // 초음파센서2 TRIG 핀
#define ECHO_PIN2 7 // 초음파센서2 ECHO 핀

// 환경에 따른 변경 값
#define CUP_H 24 // 컵 인식 높이
#define CUP_PER 0.8 // 컵에 채울 물 비율

// 모터 핀 설정
int AA = 10;  // 물 공급 모터 핀 AA
int AB = 11;  // 물 공급 모터 핀 AB

// 초음파 센서 거리 측정 설정
#define MAX_DISTANCE 1000 // 최대 측정 거리 (cm)
NewPing sonar1(TRIG_PIN1, ECHO_PIN1, MAX_DISTANCE);
NewPing sonar2(TRIG_PIN2, ECHO_PIN2, MAX_DISTANCE);

// 컵 감지 관련 변수
long cupHeight = 0;  // 컵의 높이
long waterHeight = 0; // 물의 높이
bool cupDetected = false; // 컵 감지 여부
long cupAvg = 0; //  컵의 높이 평균 (오차범위 감소)


void setup() {
  pinMode(AA, OUTPUT);  // 모터 핀 AA를 출력으로 설정
  pinMode(AB, OUTPUT);  // 모터 핀 AB를 출력으로 설정
  Serial.begin(9600);   // 시리얼 통신 초기화
}

void loop() {
  cupHeight = CUP_H - measureDistance(sonar1); // 초음파센서1로 컵 높이 측정
waterHeight = CUP_H - measureDistance(sonar2); //초음파센서2로 물 높이 측정

  if (cupHeight > 4 && waterHeight < cupHeight) { // 컵이 감지되었는지 확인
    Serial.print("컵 감지됨, 높이: ");
    Serial.println(cupHeight);
   
    // 5초 동안 컵 높이가 일정하게 유지되는지 확인
    if (isCupStable(cupHeight)) {
      Serial.println("컵 높이 일정");
      Serial.println("================== 물 공급 시작 ==================");
      giveWater(cupAvg); // 물 주기 시작
      delay(5000);
    }
  } else {
    Serial.print("컵 감지되지 않음, 높이: ");
    Serial.println(cupHeight);
  }

  delay(500);
}

// 초음파 센서 거리 측정 함수
long measureDistance(NewPing sonar) {
  delay(20); // 센서 안정화
  return sonar.ping_cm(); // cm 단위로 거리 반환
}

// 컵 높이가 일정하게 유지되는지 확인
bool isCupStable(long targetHeight) {
  cupAvg = targetHeight;
  for (int i = 0; i < 10; i++) { // 2초 동안 (0.2초마다 10번 확인)
    long currentHeight = CUP_H - measureDistance(sonar1);
    Serial.print(i+1);
    Serial.print("컵 안정성 확인 중, 인식한 컵 높이: ");
    Serial.println(currentHeight);
    cupAvg += currentHeight;
    if (abs(currentHeight - targetHeight) > 3) { // 높이 차이가 3cm 이상이면 false
      return false;
    }
    delay(200);
  }
  cupAvg /= 11;
  Serial.print("평균 컵 높이: ");
  Serial.println(cupAvg);
  return true;
}

// 물 공급 함수
void giveWater(long cupHeight) {
  long targetWaterHeight = cupHeight * CUP_PER; // 컵 높이의 목표 수위
  startMotor(); // 물 공급 시작
  while (true) {
    waterHeight = CUP_H - measureDistance(sonar2); 

    Serial.print("컵 높이:");
    Serial.println(cupHeight);
    Serial.print("목표 물 높이:");
    Serial.println(targetWaterHeight);
    Serial.print("현재 물 높이:");
    Serial.println(waterHeight);
    
    if (waterHeight >= targetWaterHeight) {
      Serial.println("================== 물 공급 완료 ==================");
      stopMotor(); // 물 공급 중지
      break;
    } 
    delay(100);
  }
}

// 모터 시작 함수
void startMotor() {
  digitalWrite(AA, HIGH);
  digitalWrite(AB, LOW);
  Serial.println("모터 작동 : 물 공급 시작");
}

// 모터 정지 함수
void stopMotor() {
  digitalWrite(AA, LOW);
  digitalWrite(AB, LOW);
  Serial.println("모터 정지 : 물 공급 중지");
}
