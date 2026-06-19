#include <math.h>
const int ledPin = 2;
const int pwmChannel = 0;
const int freq = 5000;
const int resolution = 8;

void setup() {
  Serial.begin(115200);
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(ledPin, pwmChannel);
}

void loop() {
  // 正弦曲线实现自然柔和呼吸
  for(float angle = 0; angle <= 2 * PI; angle += 0.01){
    // sin范围-1~1，映射到0~255亮度
    int brightness = map(sin(angle), -1, 1, 0, 255);
    ledcWrite(pwmChannel, brightness);
    delay(8);
  }
  Serial.println("正弦柔和呼吸一轮完成");
}