#include <Wire.h> 
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

//--- 설정 값 (여기서 게임을 조절하세요!) ---
const int hitOffset = 50; 
const int targetUpTime = 4500;  // 4.5초로 수정 (원하는 시간으로 설정)
const int delayAfterHit = 2000;

//--- 하드웨어 핀 설정 ---
const int photoresistorPin1 = A0;
const int servoPin1 = 9;
const int photoresistorPin2 = A1;
const int servoPin2 = 10;
const int photoresistorPin3 = A2;
const int servoPin3 = 11;
const int BUZZER_PIN = 12;
const int LED1 = 5;
const int LED2 = 6;
const int LED3 = 7; // BUZZER_PIN과 동일합니다. 핀 번호를 8번 등으로 바꿔야 합니다.
                    // 만약 LED3이 7번이 맞다면 부저와 동시에 켜집니다.
                    // 여기서는 핀 번호를 8로 수정했다고 가정합니다.
                    // const int LED3 = 8; 


//--- 서보모터 각도 설정 ---
const int servoUpAngle = 90;
const int servoDownAngle = 0;

//--- LCD 설정 ---
LiquidCrystal_I2C lcd(0x27, 20, 4);

//--- 서보모터 객체 생성 ---
Servo targetServo1;
Servo targetServo2;
Servo targetServo3;

//--- 게임 변수 ---
int targetSequence[] = {1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3};
int sequenceLength = sizeof(targetSequence) / sizeof(int);
int currentSequenceIndex = 0;
int score = 0;
int startLightValue = 0;
int highScore = 0;
bool targetIsUp = false;
bool gameRunning = false;
bool idleIsUp = false;
unsigned long gameStartTime;
unsigned long targetUpStartTime;
unsigned long nextTargetTime;
int baseLightValue1 = 0; 
int baseLightValue2 = 0;
int baseLightValue3 = 0;
bool hit = false;


//--- 비차단(Non-Blocking) 타이머를 위한 변수 ---
bool isServoMoving = false;      // 서보가 현재 움직이는 중인지 여부
unsigned long servoMoveStartTime = 0; // 서보 움직임 시작 시간
const long servoMoveDuration = 1500; // 서보가 움직이는 데 걸리는 시간 (기존 delay)

bool isLedOn = false;          // LED가 켜져있는지 여부
unsigned long ledOnStartTime = 0;   // LED 켜기 시작한 시간
const long ledOnDuration = 1500;    // LED 켜져있는 시간

// (setupServo, updateScoreboard, playArcadeScream, idleGame 함수는 기존과 동일)
// ... (기존 setupServo 함수) ...
void setupServo(){ 
  targetServo1.attach(servoPin1);
  targetServo2.attach(servoPin2);
  targetServo3.attach(servoPin3);
  targetServo1.write(servoDownAngle);
  targetServo2.write(servoDownAngle);
  targetServo3.write(servoDownAngle);
  delay(1000); // setup()에서는 delay 사용 가능
  targetServo1.write(servoUpAngle);
  targetServo2.write(servoUpAngle);
  targetServo3.write(servoUpAngle);
  delay(1500); 
  baseLightValue1 = analogRead(photoresistorPin1);
  baseLightValue2 = analogRead(photoresistorPin2);
  baseLightValue3 = analogRead(photoresistorPin3);
  Serial.println("--- Sensor Calibration ---");
  Serial.print("Target 1 Base Light: ");
  Serial.println(baseLightValue1);
  Serial.print("Target 2 Base Light: ");
  Serial.println(baseLightValue2);
  Serial.print("Target 3 Base Light: ");
  Serial.println(baseLightValue3);
  Serial.println("--------------------------");
  targetServo1.write(servoDownAngle);
  targetServo2.write(servoDownAngle);
  targetServo3.write(servoDownAngle);
  delay(1000);
}
// ... (기존 updateScoreboard 함수) ...
void updateScoreboard() {
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(" Score:   " + String(score));
}
// ... (기존 playArcadeScream 함수) ...
void playArcadeScream() {
  for (int hz = 2400; hz > 1000; hz -= 120) { 
    tone(BUZZER_PIN, hz, 20); 
    delay(10); 
    tone(BUZZER_PIN, hz - 180, 20); 
    delay(10);
  }
  for (int hz = 1000; hz > 300; hz -= 50) { 
    tone(BUZZER_PIN, hz, 10); 
    delay(5); 
  }
  noTone(BUZZER_PIN); 
}

void shuffleArray(int arr[], int size) {
  // 배열의 마지막 요소부터 첫 번째 요소까지 역순으로 반복
  for (int i = size - 1; i > 0; i--) {
    // 0부터 i (포함) 사이의 무작위 인덱스 j를 선택
    // random(i + 1)은 0부터 i까지의 정수를 반환합니다.
    long j = random(i + 1);

    // arr[i]와 arr[j]의 위치를 교환 (swap)
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
  }
}

// ... (기존 idleGame 함수) ...
void idleGame(){
  if (!idleIsUp) {
      delay(2000); // idleGame은 게임 시작 전이므로 delay 사용 가능
      targetServo3.write(servoUpAngle);
      delay(1000);

      lcd.setCursor(0, 0);
      lcd.print("Best" + String(highScore));

      lcd.setCursor(0, 1);
      lcd.print("start:hit target");
      idleIsUp = true;
      
      startLightValue = analogRead(photoresistorPin3);
    }
    int idleCurrent = analogRead(photoresistorPin3);

    if (startLightValue + hitOffset < idleCurrent) {
      targetServo3.write(servoDownAngle);
      delay(1500);
      gameRunning = true;
      currentSequenceIndex = 0;
      score = 0;
      randomSeed(analogRead(A0));
      shuffleArray(targetSequence, sequenceLength);
      updateScoreboard(); // 점수판 초기화
      // 게임 시작 시 Best 스코어만 남기기
      lcd.setCursor(8, 0);
      lcd.print("        ");
    }
}


//--- 초기 설정 함수 ---
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.init();
  lcd.backlight();
  pinMode(BUZZER_PIN, OUTPUT);
  setupServo();
  gameStartTime = millis();
  nextTargetTime = gameStartTime + 1000;
  updateScoreboard();
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
}


//--- (수정됨) 표적 올리는 함수 ---
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

  // delay(1500); <-- 제거!
  
  isServoMoving = true;           // 서보가 움직이기 시작했다고 표시
  servoMoveStartTime = millis();  // 움직임 시작 시간 기록

  Serial.println("--------------------");
  Serial.println("Target " + String(targetNum) + " RAISING...");
}

//--- (수정됨) 표적 내리는 함수 ---
void lowerTarget(int targetNum) {
  if (hit == true) { // 맞췄을 때만 LED 켜기
    isLedOn = true;
    ledOnStartTime = millis(); // LED 켜진 시간 기록
    if (targetNum == 1) {
      digitalWrite(LED1, HIGH);
    } else if (targetNum == 2) {
      digitalWrite(LED2, HIGH);
    } else {
      digitalWrite(LED3, HIGH);
    }
  }

  // 서보 내리기
  if (targetNum == 1) {
    targetServo1.write(servoDownAngle);
  } else if (targetNum == 2) {
    targetServo2.write(servoDownAngle);
  } else {
    targetServo3.write(servoDownAngle);
  }

  // delay(1500) 등 모든 딜레이 제거!
  
  isServoMoving = true;           // 서보가 움직이기 시작했다고 표시
  servoMoveStartTime = millis();  // 움직임 시작 시간 기록
  
  Serial.println("Target " + String(targetNum) + " LOWERING...");
}

//--- (수정됨) 메인 루프 함수 ---
void loop() {
  unsigned long currentTime = millis();

  // --- 1. 비차단 LED 끄기 관리 ---
  // LED가 켜져있고, 켜진 시간이 ledOnDuration(1.5초)를 지났으면 LED를 끈다.
  if (isLedOn && (currentTime - ledOnStartTime > ledOnDuration)) {
    isLedOn = false;
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
  }

  // --- 2. 비차단 서보 움직임 관리 ---
  // 서보가 움직이는 중(isServoMoving == true)일 때
  if (isServoMoving) {
    // 서보가 움직인지 1.5초가 지났는지 확인
    if (currentTime - servoMoveStartTime > servoMoveDuration) {
      isServoMoving = false; // 서보 움직임 완료

      // 서보 움직임이 끝난 후 처리
      if (targetIsUp) { 
        // 표적이 올라와 있었으니, 방금 동작은 '내리기'였음
        targetIsUp = false;
        currentSequenceIndex++; // 다음 표적으로 이동
        nextTargetTime = currentTime + delayAfterHit; // 다음 표적까지 딜레이
        Serial.println("Target DOWN complete.");
      } 
      else {
        // 표적이 내려가 있었으니, 방금 동작은 '올리기'였음
        targetIsUp = true;
        targetUpStartTime = currentTime; // 표적이 올라온 시간 기록 (타이머 시작!)
        Serial.println("Target UP complete.");
      }
    }
    // 서보가 아직 움직이는 중이면 (1.5초가 안 지났으면)
    // 센서 읽기 등 다른 로직을 실행하지 않고 loop를 빠져나간다.
    // (빠르게 다음 loop()를 실행하며 시간 체크)
    return; 
  }

  // --- 3. 게임 실행 로직 (서보가 멈춰있을 때만 실행됨) ---
  if (!gameRunning) {
    idleGame(); // 게임 시작 대기
  }
  else {
    // --- 4. 게임 종료 확인 ---
    if (currentSequenceIndex >= sequenceLength) { 
      if (score > highScore) {
        highScore = score;
        lcd.setCursor(0, 0); 
        lcd.print("Best" + String(highScore));
      }
      lcd.setCursor(8, 0);
      lcd.print(" You" + String(score));
      gameRunning = false; 
      idleIsUp = false;
      return;
    }
    
    // --- 5. 표적 올릴 시간 확인 ---
    // (표적이 내려가 있고, 서보가 안 움직이고, 다음 표적 올릴 시간이 되었을 때)
    if (!targetIsUp && currentTime >= nextTargetTime) { 
      raiseTarget(targetSequence[currentSequenceIndex]);
    }

    // --- 6. 표적이 올라와 있을 때 (히트 또는 타임아웃 확인) ---
    // (표적이 올라와 있고, 서보가 안 움직일 때)
    if (targetIsUp) { 
      hit = false;
      int currentTarget = targetSequence[currentSequenceIndex];
      int currentLightValue = 0;
      unsigned long timeSinceUp = currentTime - targetUpStartTime;

      // ... (기존 히트 판정 로직 - 동일) ...
      if (currentTarget == 1) {
        currentLightValue = analogRead(photoresistorPin1);
        if (currentLightValue > baseLightValue1 + hitOffset) {
          hit = true;
          Serial.println(currentLightValue);
        }
      }
      else if (currentTarget == 2){ 
        currentLightValue = analogRead(photoresistorPin2);
        if (currentLightValue > baseLightValue2 + hitOffset) {
          hit = true;
          Serial.println(currentLightValue);
        }
      }
      else{ // currentTarget == 3
        currentLightValue = analogRead(photoresistorPin3);
        if (currentLightValue > baseLightValue3 + hitOffset) {
          hit = true;
          Serial.println(currentLightValue);
        }
      }
      
      // --- 7. 판정 처리 (히트 또는 타임아웃) ---
      if (hit) {
        score = score + 100;
        updateScoreboard();
        
        playArcadeScream(); // 참고: 이 함수는 여전히 delay를 사용합니다.
        
        lowerTarget(currentTarget); // 표적 내리기 '시작'
      }
      else if (timeSinceUp > targetUpTime) { // 타임아웃
        hit = false; // 맞춘 게 아니므로 LED가 켜지지 않도록 함
        lowerTarget(currentTarget); // 표적 내리기 '시작'
        Serial.println("Time OUT!");
      }
    }
  }
}