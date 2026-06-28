#include <esp32-hal-touch.h> 

#define TOUCH_PIN 4
#define LED_PIN   2

const int pwmFreq = 5000;
const int pwmRes = 8;
const int pwmChannel = 0;

// 三档间隔：5 / 10 / 20，快慢差距明显
int speedGear[3] = {5, 10, 20}; 
int currentGear = 0;

const int touchThreshold = 30;
bool lastTouchFlag = false;
const unsigned long debounceTime = 30;
unsigned long lastTouchTime = 0;

// 无阻塞呼吸灯变量
int duty = 0;
int step = 1;
unsigned long lastBrightUpdate = 0;

// 串口输出控制变量
unsigned long lastSerialPrint = 0;
const unsigned long serialPrintInterval = 1000;

void setup() {
  Serial.begin(115200);
  ledcSetup(pwmChannel, pwmFreq, pwmRes);
  ledcAttachPin(LED_PIN, pwmChannel);
  
  Serial.println("========================================");
  Serial.println("   多档位触摸调速呼吸灯 v1.0");
  Serial.println("========================================");
  Serial.println("触摸引脚: GPIO4");
  Serial.println("LED引脚: GPIO2");
  Serial.println("当前档位: 1档 (最快)");
  Serial.println("----------------------------------------");
  
  int touchBase = touchRead(TOUCH_PIN);
  Serial.printf("触摸基准值: %d\n", touchBase);
  Serial.printf("触摸阈值: %d\n", touchThreshold);
  Serial.println("提示: 触摸GPIO4切换档位");
  Serial.println("========================================");
  Serial.println();
}

void loop() {
  unsigned long now = millis();
  int touchVal = touchRead(TOUCH_PIN);
  bool currentTouchFlag = (touchVal < touchThreshold);

  // 触摸检测：每轮loop都会执行，不再阻塞
  if (currentTouchFlag && !lastTouchFlag && (now - lastTouchTime > debounceTime)) {
    currentGear = (currentGear + 1) % 3;
    
    Serial.println("----------------------------------------");
    Serial.println("🔵 触摸事件触发!");
    Serial.printf("触摸值: %d (阈值: %d)\n", touchVal, touchThreshold);
    Serial.printf("切换至档位: %d档 (延时: %dms)\n", currentGear + 1, speedGear[currentGear]);
    
    String gearDesc;
    switch(currentGear) {
      case 0: gearDesc = "快速呼吸 (间隔5ms)"; break;
      case 1: gearDesc = "中速呼吸 (间隔10ms)"; break;
      case 2: gearDesc = "慢速呼吸 (间隔20ms)"; break;
    }
    Serial.printf("呼吸模式: %s\n", gearDesc.c_str());
    Serial.println("----------------------------------------");
    
    lastTouchTime = now;
  }
  lastTouchFlag = currentTouchFlag;

  // 无阻塞呼吸渐变，不占用CPU
  int interval = speedGear[currentGear];
  if (now - lastBrightUpdate >= interval) {
    duty += step;
    // 到达上下边界反转渐变方向
    if (duty >= 255) step = -1;
    if (duty <= 0) step = 1;
    ledcWrite(pwmChannel, duty);
    lastBrightUpdate = now;
  }

  // 每秒打印一次状态
  if (now - lastSerialPrint >= serialPrintInterval) {
    static unsigned long cycleStart = millis();
    unsigned long cycleDuration = now - cycleStart;
    if(duty == 0) cycleStart = millis();

    Serial.printf("[状态] 档位: %d | 延时: %3dms | 周期: %4dms | 触摸值: %4d", 
                  currentGear + 1, 
                  interval, 
                  cycleDuration,
                  touchVal);
                  
    if (currentTouchFlag) {
      Serial.print(" | ✋ 触摸中");
    } else {
      Serial.print(" | 🟢 空闲");
    }
    Serial.println();
    lastSerialPrint = now;
  }
}