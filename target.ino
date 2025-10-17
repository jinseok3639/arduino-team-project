#include <Servo.h>
#include <LiquidCrystal_I2C.h>

//--- 설정 값 (여기서 게임을 조절하세요!) ---
const int hitOffset = 30;      // 기준 밝기보다 이 값만큼 커지면 '히트'로 인정
const int gameDuration = 40000; // 총 게임 시간 (40초)
const int targetUpTime = 5000;  // 표적이 올라와 있는 시간 (5초)
const int delayAfterHit = 1000; // 표적 맞춘 후 다음 표적까지 딜레이 (1초)

//--- 하드웨어 핀 설정 ---
const int photoresistorPin1 = A0;
const int servoPin1 = 9;
const int photoresistorPin2 = A1;
const int servoPin2 = 10;
const int photoresistorPin3 = A2;
const int servoPin3 = 11;

const int LED1 = 5;
const int LED2 = 6;
const int LED3 = 7;

//--- 서보모터 각도 설정 ---
const int servoUpAngle = 90;
const int servoDownAngle = 0;

//--- LCD 설정 ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

//--- 서보모터 객체 생성 ---
Servo targetServo1;
Servo targetServo2;
Servo targetServo3;

//--- 게임 변수 ---
//int targetSequence[] = {1, 1, 1, 1, 1}; // 표적이 올라올 순서
//int targetSequence[] = {2, 2, 2, 2, 2}; // 표적이 올라올 순서
int targetSequence[] = {1, 2, 1, 2, 1};
int sequenceLength = sizeof(targetSequence) / sizeof(int);
int currentSequenceIndex = 0;
int score = 0;
int baseLightValue = 0; // 표적이 올라왔을 때의 기준 밝기를 저장할 변수
bool targetIsUp = false;
bool gameRunning = false;
unsigned long gameStartTime;
unsigned long targetUpStartTime;
unsigned long nextTargetTime;

int baseLightValue1 = 0; 
int baseLightValue2 = 0;

bool hit = false;

//--- 초기 설정 함수 ---
void setup() {
  Serial.begin(9600);

  targetServo1.attach(servoPin1);
  targetServo2.attach(servoPin2);
  targetServo3.attach(servoPin3);
  targetServo1.write(servoDownAngle);
  targetServo2.write(servoDownAngle);
  targetServo3.write(servoDownAngle);

  delay(1000);

    // 표적을 모두 올림
  targetServo1.write(servoUpAngle);
  targetServo2.write(servoUpAngle);
  targetServo3.write(servoUpAngle);

  delay(1500); // 서보가 움직일 시간을 충분히 줌

  // 각 표적의 초기 밝기 값을 읽어서 변수에 저장
  baseLightValue1 = analogRead(photoresistorPin1);
  baseLightValue2 = analogRead(photoresistorPin2);
  baseLightValue3 = analogRead(photoresistorPin3);

  // 측정된 값을 시리얼 모니터에 출력 (디버깅용)
  Serial.println("--- Sensor Calibration ---");
  Serial.print("Target 1 Base Light: ");
  Serial.println(baseLightValue1);
  Serial.print("Target 2 Base Light: ");
  Serial.println(baseLightValue2);
  Serial.print("Target 3 Base Light: ");
  Serial.println(baseLightValue3);
  Serial.println("--------------------------");
    
  // 측정이 끝나면 표적을 다시 내림
  targetServo1.write(servoDownAngle);
  targetServo2.write(servoDownAngle);
  targetServo3.write(servoDownAngle);
  delay(1000);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Laser Target");
  lcd.setCursor(0, 1);
  lcd.print("Game Start!");

  gameStartTime = millis();
  nextTargetTime = gameStartTime + 1000;
  gameRunning = true;
  updateScoreboard();

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
}

//--- 메인 루프 함수 ---
void loop() {
  int currentVal1 = analogRead(photoresistorPin1);
  int currentVal2 = analogRead(photoresistorPin2);
  int currentVal3 = analogRead(photoresistorPin3);
  Serial.print("target 1: ");
  Serial.print(currentVal1);
  Serial.print(" | target 2: ");
  Serial.print(currentVal2);
  Serial.print("target 3: ");
  Serial.println(currentVal3);

  if (!gameRunning || currentSequenceIndex >= sequenceLength) {
    if (gameRunning) {
      lcd.setCursor(0, 0);
      lcd.print("Game Over!      ");
      lcd.setCursor(0, 1);
      lcd.print("Final Score: " + String(score));
      gameRunning = false;
    }
    return;
  }

  if (millis() - gameStartTime > gameDuration) {
    gameRunning = false;
    targetServo1.write(servoDownAngle);
    targetServo2.write(servoDownAngle);
    targetServo3.write(servoDownAngle);
    return;
  }
  
  unsigned long currentTime = millis();

  if (!targetIsUp && currentTime >= nextTargetTime) {
    raiseTarget(targetSequence[currentSequenceIndex]);
  }

  if (targetIsUp) {
    hit = false;
    int currentTarget = targetSequence[currentSequenceIndex];
    int currentLightValue = 0;

    // 현재 올라온 표적의 조도센서 값 읽기
    if (currentTarget == 1) {
      currentLightValue = analogRead(photoresistorPin1);
      // 디버깅: 현재 값과 목표 값을 시리얼 모니터에 출력
      Serial.print("Target 1 Current: ");
      Serial.print(currentLightValue);
      Serial.print(" / Hit if > ");
      Serial.println(baseLightValue1 + hitOffset);
      
      // ★★★ 새로운 히트 판정 로직 ★★★
      if (currentLightValue > baseLightValue1 + hitOffset) {
        hit = true;
        Serial.println(currentLightValue);
      }
    } else if (currentTarget == 2){ 
      currentLightValue = analogRead(photoresistorPin2);
      // 디버깅: 현재 값과 목표 값을 시리얼 모니터에 출력
      Serial.print("Target 2 Current: ");
      Serial.print(currentLightValue);
      Serial.print(" / Hit if > ");
      Serial.println(baseLightValue2 + hitOffset);

      // ★★★ 새로운 히트 판정 로직 ★★★
      if (currentLightValue > baseLightValue2 + hitOffset) {
        hit = true;
        Serial.println(currentLightValue);
      }
    }
    else{ // currentTarget == 3
      currentLightValue = analogRead(photoresistorPin3);
      // 디버깅: 현재 값과 목표 값을 시리얼 모니터에 출력
      Serial.print("Target 3 Current: ");
      Serial.print(currentLightValue);
      Serial.print(" / Hit if > ");
      Serial.println(baseLightValue3 + hitOffset);

      // ★★★ 새로운 히트 판정 로직 ★★★
      if (currentLightValue > baseLightValue3 + hitOffset) {
        hit = true;
        Serial.println(currentLightValue);
      }
    }
    // currentTime - targetUpStartTime > targetUpTime
    if (hit) {
      if(hit) {
        score++;
        updateScoreboard();
      }
      lowerTarget(currentTarget);
    }
  }
}

//--- 표적 올리는 함수 ---
void raiseTarget(int targetNum) {
  if (targetNum == 1) {
    targetServo1.write(servoUpAngle);
  }
  else if (targetNum == 2) {
    targetServo2.write(servoUpAngle);
  }
  else {
    targetServo3.write(servoUpAngle);
  }

  // 서보모터가 움직일 시간을 잠시 기다려 정확한 값을 측정
  delay(1500);
  
  targetIsUp = true;
  targetUpStartTime = millis();
  Serial.println("--------------------");
  Serial.println("Target " + String(targetNum) + " UP");
  Serial.print("  -> Baseline Light Set To: ");
  Serial.println(baseLightValue);
}

//--- 표적 내리는 함수 ---
void lowerTarget(int targetNum) {
  if (targetNum == 1) {
    if (hit == true){
        digitalWrite(LED1, HIGH);
        targetServo1.write(servoDownAngle);
        delay(1500);
        digitalWrite(LED1, LOW);
    }
    else{
      targetServo1.write(servoDownAngle);
    }
  }
  else if (targetNum == 2) { // LED 번호 바꿔야됨 !!
    if (hit == true){
        digitalWrite(LED2, HIGH);
        targetServo2.write(servoDownAngle);
        delay(1500);
        digitalWrite(LED2, LOW);
    }
    else{
      targetServo2.write(servoDownAngle);
    }
  }
  else {
    if (hit == true){
        digitalWrite(LED3, HIGH);
        targetServo3.write(servoDownAngle);
        delay(1500);
        digitalWrite(LED3, LOW);
    }
    else{
      targetServo3.write(servoDownAngle);
    }
  }
  targetIsUp = false;
  currentSequenceIndex++;
  nextTargetTime = millis() + delayAfterHit;
  Serial.println("Target " + String(targetNum) + " DOWN");
}

//--- 점수판 업데이트 함수 ---
void updateScoreboard() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Score: " + String(score));
}