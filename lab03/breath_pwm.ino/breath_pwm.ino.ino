// lab03 PWM呼吸灯实验（兼容所有ESP32内核版本标准写法）
const int ledPin = 2;
const int pwmChannel = 0;  // ESP32共有0~15共16个PWM通道，任选一个
const int freq = 5000;     // PWM频率5000Hz
const int resolution = 8;  // 8位分辨率，占空比范围0~255

void setup() {
  Serial.begin(115200);
  // 1. 先配置PWM通道：通道号、频率、分辨率
  ledcSetup(pwmChannel, freq, resolution);
  // 2. 将PWM通道绑定到LED引脚
  ledcAttachPin(ledPin, pwmChannel);
}

void loop() {
  // 亮度逐渐变亮
  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){
    ledcWrite(pwmChannel, dutyCycle); // 写入对应通道占空比
    delay(10);
  }
  // 亮度逐渐变暗
  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    ledcWrite(pwmChannel, dutyCycle);
    delay(10);
  }
  Serial.println("Breathing cycle completed");
}