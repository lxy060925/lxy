// lab02进阶实验：SOS摩斯求救信号灯（三短三长三短）
const int ledPin = 2;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // ========== S：短闪3次 ==========
  for(int i=0; i<3; i++) {
    digitalWrite(ledPin, HIGH); 
    delay(200); 
    digitalWrite(ledPin, LOW); 
    delay(200); 
  }
  delay(500); // 字母间隔S-O

  // ========== O：长闪3次 ==========
  for(int i=0; i<3; i++) {
    digitalWrite(ledPin, HIGH); 
    delay(600); 
    digitalWrite(ledPin, LOW); 
    delay(200); 
  }
  delay(500); // 字母间隔O-S

  // ========== S：短闪3次 ==========
  for(int i=0; i<3; i++) {
    digitalWrite(ledPin, HIGH); 
    delay(200); 
    digitalWrite(ledPin, LOW); 
    delay(200); 
  }
  delay(2000); // 单次完整SOS结束，长停顿2秒再循环
}