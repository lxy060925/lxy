// ex04 触摸自锁开关，带软件防抖、边缘触发
#define TOUCH_PIN 4    // T0 GPIO4触摸引脚
#define LED_PIN   2    // ESP32板载LED

const int touchThreshold = 20;
bool ledState = false;
bool lastTouchFlag = false;
const unsigned long debounceTime = 150;
unsigned long lastTouchTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  int touchVal = touchRead(TOUCH_PIN);
  bool currentTouchFlag = (touchVal < touchThreshold);
  unsigned long now = millis();

  // 边缘检测：松开→按下瞬间 + 防抖
  if (currentTouchFlag && !lastTouchFlag && (now - lastTouchTime > debounceTime)) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    Serial.print("LED切换状态：");
    Serial.println(ledState ? "点亮" : "熄灭");
    lastTouchTime = now;
  }
  lastTouchFlag = currentTouchFlag;
  delay(20);
}