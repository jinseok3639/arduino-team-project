// 각 부품에 연결된 핀 번호를 상수로 정의합니다.
const int BUTTON_PIN = 2;    // [유지] 풀업 방식으로 사용할 버튼 핀
const int BUZZER_PIN = 7;    // 수동 부저
const int RED_PIN = 9;       // RGB LED의 빨간색(R) 핀
const int GREEN_PIN = 10;    // RGB LED의 초록색(G) 핀
const int BLUE_PIN = 11;     // RGB LED의 파란색(B) 핀
const int LASER_PIN = 12;    // 레이저 모듈

// --- 장탄수 설정 ---
const int MAX_AMMO = 10;     // 최대 장탄수
int ammoCount = MAX_AMMO;    // 현재 장탄수

// --- 버튼 상태 감지를 위한 변수 ---
// [변경됨] 풀업 방식의 기본 상태는 HIGH이므로 초기값을 HIGH로 설정합니다.
int lastButtonState = HIGH;

// 총소리 효과음 함수
void mixedFireSound() {
  tone(BUZZER_PIN, 2500, 20);
  delay(20);
  for (int hz = 1200; hz > 200; hz -= 25) {
    tone(BUZZER_PIN, hz, 10);
    delay(2);
  }
}

// 탄약 없을 때 '딸깍' 소리
void emptySound() {
  tone(BUZZER_PIN, 150, 50);
}

// 재장전 소리
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

  tone(BUZZER_PIN, 880, 75);
  delay(80);
  tone(BUZZER_PIN, 660, 100);
}

// 장탄수에 따라 LED 색상을 업데이트하는 함수
void updateAmmoLED() {
  int redValue, greenValue;
  if (ammoCount > MAX_AMMO / 2) {
    greenValue = 255;
    redValue = map(ammoCount, MAX_AMMO, MAX_AMMO / 2, 0, 255);
  } else {
    redValue = 255;
    greenValue = map(ammoCount, MAX_AMMO / 2, 0, 255, 0);
  }
  analogWrite(RED_PIN, redValue);
  analogWrite(GREEN_PIN, greenValue);
  analogWrite(BLUE_PIN, 0);
}

void setup() {
  Serial.begin(9600);
  // [변경됨] 버튼 핀을 내부 풀업 저항을 사용하는 입력으로 설정합니다.
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(LASER_PIN, OUTPUT);
  updateAmmoLED();
}

void loop() {
  int currentButtonState = digitalRead(BUTTON_PIN);

  // [변경됨] 버튼이 눌리는 순간을 (HIGH -> LOW)로 감지합니다.
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    
    // 1. 장탄수가 남아있을 경우: 발사
    if (ammoCount > 0) {
      ammoCount--;
      Serial.print("Fire! Ammo left: ");
      Serial.println(ammoCount);
      
      digitalWrite(LASER_PIN, HIGH);
      mixedFireSound();
      digitalWrite(LASER_PIN, LOW);
      
      updateAmmoLED();
    }
    // 2. 장탄수가 없는 경우: '딸깍' 소리 후 재장전
    else {
      Serial.println("Empty! Press again to reload.");
      emptySound();
      
      // [변경됨] 버튼에서 손을 뗄 때(HIGH)까지 기다림
      while(digitalRead(BUTTON_PIN) == LOW); 
      // [변경됨] 다시 버튼을 누를 때(LOW)까지 기다림
      while(digitalRead(BUTTON_PIN) == HIGH); 
      
      Serial.println("Reloading...");
      reloadSound();
      ammoCount = MAX_AMMO;
      updateAmmoLED();
      Serial.println("Reload complete!");
    }
    
    delay(50); // 디바운싱
  }

  lastButtonState = currentButtonState;
}