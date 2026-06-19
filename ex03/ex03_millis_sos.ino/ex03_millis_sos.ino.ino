// ex03作业：使用millis纯无阻塞方式实现SOS摩斯信号灯
// 时序标准：3次短闪S → 3次长闪O → 3次短闪S，整套结束停顿2000ms
const int ledPin = 2;
// 时序参数，与实验文档要求完全匹配
const unsigned long shortOn = 200;
const unsigned long shortOff = 200;
const unsigned long longOn = 600;
const unsigned long longOff = 200;
const unsigned long letterGap = 500;
const unsigned long wordGap = 2000;

// 计时变量
unsigned long lastTick = 0;
int stage = 0;    // 0=单词间隔 1=字母S 2=字母O 3=末尾S
int flashCnt = 0; // 当前字母闪烁次数计数
bool ledOn = false; // LED亮灭状态标记

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.println("=== ESP32 SOS程序启动 ===");
}

void loop() {
  unsigned long now = millis();

  // ========== 单词间隔特殊处理 ==========
  if (stage == 0) {
    if (now - lastTick >= wordGap) {
      lastTick = now;
      stage = 1;
      flashCnt = 0;
      ledOn = false;
      Serial.println("\n开始新一轮SOS信号：播放字母S（三次短闪）");
    }
    return;  // 单词间隔期间不执行下面的闪烁逻辑
  }

  // ========== 计算当前阶段的等待时间 ==========
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
      return;  // 防止意外情况
  }

  // ========== 非阻塞延时判断 ==========
  if (now - lastTick < waitTime) return;
  lastTick = now;

  // ========== 切换LED亮灭状态 ==========
  ledOn = !ledOn;
  digitalWrite(ledPin, ledOn);

  // ========== LED熄灭时计数，一组闪烁完成切换下一字母 ==========
  if (!ledOn) {
    flashCnt++;

    // S三组短闪完成 → 切换到O
    if (stage == 1 && flashCnt >= 3) {
      flashCnt = 0;
      stage = 2;
      lastTick = now;
      Serial.println("字母S完成，播放O（三组长闪）");
    }
    // O三组长闪完成 → 切换到末尾S
    else if (stage == 2 && flashCnt >= 3) {
      flashCnt = 0;
      stage = 3;
      lastTick = now;
      Serial.println("字母O完成，播放末尾S（三次短闪）");
    }
    // 末尾S三组短闪完成 → 进入2000ms单词间隔
    else if (stage == 3 && flashCnt >= 3) {
      flashCnt = 0;
      stage = 0;
      lastTick = now;
      Serial.println("========================================");
      Serial.println("完整SOS播放结束，停顿2000ms后重新循环");
      Serial.println("========================================");
    }
  }
}