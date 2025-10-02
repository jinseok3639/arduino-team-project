// 각 부품에 연결된 핀 번호를 상수로 정의합니다.
const int BUTTON_PIN = 2;    // 풀다운 버튼
const int BUZZER_PIN = 7;    // 수동 부저
const int RED_PIN = 9;       // RGB LED의 빨간색(R) 핀
const int GREEN_PIN = 10;    // RGB LED의 초록색(G) 핀
const int BLUE_PIN = 11;     // RGB LED의 파란색(B) 핀
const int LASER_PIN = 12;    // 레이저 모듈

// --- 장탄수 설정 ---
const int MAX_AMMO = 10;     // 최대 장탄수
int ammoCount = MAX_AMMO;    // 현재 장탄수

// --- 버튼 상태 감지를 위한 변수 ---
int lastButtonState = LOW;   // 이전 버튼 상태 저장

// [기존] 총소리 효과음 함수
void mixedFireSound() {
  tone(BUZZER_PIN, 2500, 20);
  delay(20);
  for (int hz = 1200; hz > 200; hz -= 25) {
    tone(BUZZER_PIN, hz, 10);
    delay(2);
  }
}

// [추가됨] 탄약 없을 때 '딸깍' 소리
void emptySound() {
  tone(BUZZER_PIN, 150, 50);
}

// [추가됨] 재장전 소리
void reloadSound() {
  for (int hz = 100; hz < 700; hz += 10) {
    tone(BUZZER_PIN, hz, 10);
    delay(2);
  }
  delay(100);
  for (int hz = 700; hz > 400; hz -= 5) {
    tone(BUZZER_PIN, hz, 5);
    delay(1);
  }
  delay(3000); // 3초 대기

  // 3. 재장전 완료 알림음 ('철컥!' 하는 느낌의 소리)
  tone(BUZZER_PIN, 880, 75);  // 짧고 높은 '철' 소리
  delay(80);
  tone(BUZZER_PIN, 660, 100); // 조금 더 낮은 '컥' 소리
}

// [추가됨] 장탄수에 따라 LED 색상을 업데이트하는 함수
void updateAmmoLED() {
  int redValue, greenValue;

  // 장탄수가 절반 이상일 때 (녹색 -> 노란색)
  if (ammoCount > MAX_AMMO / 2) {
    // 녹색은 최대로 유지하고, 빨간색을 점점 늘려 노란색으로 만듭니다.
    greenValue = 255;
    redValue = map(ammoCount, MAX_AMMO, MAX_AMMO / 2, 0, 255);
  }
  // 장탄수가 절반 이하일 때 (노란색 -> 빨간색)
  else {
    // 빨간색은 최대로 유지하고, 녹색을 점점 줄여 빨간색으로 만듭니다.
    redValue = 255;
    greenValue = map(ammoCount, MAX_AMMO / 2, 0, 255, 0);
  }

  // 계산된 RGB 값으로 LED 색상 설정
  analogWrite(RED_PIN, redValue);
  analogWrite(GREEN_PIN, greenValue);
  analogWrite(BLUE_PIN, 0); // 파란색은 사용하지 않음
}

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(LASER_PIN, OUTPUT);

  // 시작 시 LED 색상을 최신 상태로 업데이트 (초록색)
  updateAmmoLED();
}

void loop() {
  int currentButtonState = digitalRead(BUTTON_PIN);

  // 버튼이 눌리는 '순간'을 감지 (LOW -> HIGH)
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    
    // 1. 장탄수가 남아있을 경우: 발사
    if (ammoCount > 0) {
      ammoCount--; // 장탄수 1 감소
      Serial.print("Fire! Ammo left: ");
      Serial.println(ammoCount);

      // 레이저와 사운드 효과
      digitalWrite(LASER_PIN, HIGH);
      mixedFireSound();
      digitalWrite(LASER_PIN, LOW); // 발사 효과 후 레이저 즉시 끄기

      // LED 색상 업데이트
      updateAmmoLED();
    }
    // 2. 장탄수가 없는 경우: '딸깍' 소리 후 재장전
    else {
      Serial.println("Empty! Press again to reload.");
      emptySound(); // 탄약 없는 소리
      
      // 재장전 대기: 버튼에서 손을 뗄 때까지 기다림
      while(digitalRead(BUTTON_PIN) == HIGH);
      // 다시 버튼을 누를 때까지 기다림
      while(digitalRead(BUTTON_PIN) == LOW);
      
      Serial.println("Reloading...");
      reloadSound(); // 재장전 소리
      ammoCount = MAX_AMMO; // 장탄수 재충전
      updateAmmoLED(); // LED 색상 초기화 (녹색)
      Serial.println("Reload complete!");
    }
    
    delay(50); // 디바운싱: 버튼의 떨림 현상 방지
  }

  // 현재 버튼 상태를 다음 루프에서 사용하기 위해 저장
  lastButtonState = currentButtonState;
}