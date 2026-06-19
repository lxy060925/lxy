// ex02 使用millis实现无阻塞LED闪烁，对比delay阻塞式代码
const int ledPin = 2;
// 记录上次翻转LED的系统时间
unsigned long lastTime = 0;
// 闪烁周期2000ms，亮2秒灭2秒
const unsigned long interval = 2000;
bool ledStatus = LOW;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // 获取当前系统运行毫秒数
  unsigned long currentTime = millis();
  // 判断间隔是否达到设定周期
  if (currentTime - lastTime >= interval) {
    lastTime = currentTime;
    ledStatus = !ledStatus;
    digitalWrite(ledPin, ledStatus);
    Serial.print("LED状态：");
    Serial.println(ledStatus ? "亮" : "灭");
  }
  // 优势：loop内其他代码不会被延时阻塞，可同时执行多任务
}