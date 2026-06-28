// ex06 双通道反相渐变警车双闪灯
// 引脚更换为指定池内：LED_A=GPIO15，LED_B=GPIO4
#define LED_A 15
#define LED_B 4

// PWM基础配置
const int freq = 5000;
const int res = 8; // 分辨率 0~255
// 两路独立PWM通道，ESP32通道范围0~15
const int chanA = 0;
const int chanB = 1;

void setup() {
  Serial.begin(115200);
  // 配置通道参数：通道号、频率、分辨率
  ledcSetup(chanA, freq, res);
  ledcSetup(chanB, freq, res);
  // 将引脚绑定到对应PWM通道
  ledcAttachPin(LED_A, chanA);
  ledcAttachPin(LED_B, chanB);
}

void loop() {
  // A灯从0→255渐亮，B灯同步255→0渐暗
  for(int dutyA = 0; dutyA <= 255; dutyA++){
    int dutyB = 255 - dutyA;
    ledcWrite(chanA, dutyA);
    ledcWrite(chanB, dutyB);
    delay(8);
  }
  // A灯从255→0渐暗，B灯同步0→255渐亮
  for(int dutyA = 255; dutyA >= 0; dutyA--){
    int dutyB = 255 - dutyA;
    ledcWrite(chanA, dutyA);
    ledcWrite(chanB, dutyB);
    delay(8);
  }
}