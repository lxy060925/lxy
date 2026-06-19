const int ledPin = 2;
const unsigned long shortOn = 200;
const unsigned long shortOff = 200;
const unsigned long longOn = 600;
const unsigned long longOff = 200;
const unsigned long letterGap = 500;
const unsigned long wordGap = 2000;

unsigned long lastTick = 0;
int stage = 0;    // 0=单词间隔 1=S 2=O 3=S
int flashCnt = 0;
bool ledOn = false;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.println("=== ESP32 SOS程序启动 ===");
}

void loop() {
  unsigned long now = millis();

  // 单词间隔特殊处理
  if (stage == 0) {
    if (now - lastTick >= wordGap) {
      lastTick = now;
      stage = 1;
      flashCnt = 0;
      ledOn = false;
    }
    return;
  }

  // 计算当前阶段的等待时间
  unsigned long waitTime;
  switch(stage) {
    case 1:
    case 3:
      waitTime = ledOn ? shortOn : shortOff;
      break;
    case 2:
      waitTime = ledOn ? longOn : longOff;
      break;
    default:
      return;
  }

  // 非阻塞延时
  if (now - lastTick < waitTime) return;
  lastTick = now;

  // 切换LED状态
  ledOn = !ledOn;
  digitalWrite(ledPin, ledOn);

  // LED关闭时计数并切换阶段
  if (!ledOn) {
    flashCnt++;

    if (stage == 1 && flashCnt >= 3) {
      flashCnt = 0;
      stage = 2;
      lastTick = now;
    }
    else if (stage == 2 && flashCnt >= 3) {
      flashCnt = 0;
      stage = 3;
      lastTick = now;
    }
    else if (stage == 3 && flashCnt >= 3) {
      flashCnt = 0;
      stage = 0;
      lastTick = now;
    }
  }
}