// ex02 使用millis实现无阻塞1Hz闪烁，周期2000ms
const int ledPin = 2;
unsigned long lastTime = 0;
const unsigned long interval = 2000;
bool ledStatus = LOW;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  unsigned long currentTime = millis();
  // 时间差达到1000ms时翻转LED
  if (currentTime - lastTime >= interval) {
    lastTime = currentTime;
    ledStatus = !ledStatus;
    digitalWrite(ledPin, ledStatus);
    Serial.print("LED状态：");
    Serial.println(ledStatus ? "亮" : "灭");
  }
}