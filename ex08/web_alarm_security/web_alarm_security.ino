// ex08 安防报警主机：网页布防/撤防 + 触摸触发报警闪烁（精美UI版）- AP模式
#include <WiFi.h>
#include <WebServer.h>

// WiFi AP配置 - ESP32自己创建热点
const char* ssid = "ESP32_Security_Alarm";  // 热点名称
const char* password = "12345678";           // 热点密码（至少8位）

// 硬件定义
#define LED_PIN 2
#define TOUCH_PIN 4
const int touchThreshold = 50;

// 全局状态变量
bool isArm = false;        // false=撤防，true=布防
bool alarmTrigger = false; // 报警锁定标记

WebServer server(80);

// 生成精美网页
String makeHtml() {
  String html = R"raw(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>安防报警主机</title>
<style>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  min-height: 100vh;
  display: flex;
  justify-content: center;
  align-items: center;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  background: linear-gradient(135deg, #0a0a1a 0%, #1a1a3e 50%, #0a0a1a 100%);
  padding: 20px;
  transition: background 0.5s ease;
}

body.alarm-active {
  background: linear-gradient(135deg, #1a0000 0%, #3e1a1a 50%, #1a0000 100%);
  animation: alarmFlash 0.5s ease-in-out infinite alternate;
}

@keyframes alarmFlash {
  0% { background: linear-gradient(135deg, #1a0000 0%, #3e1a1a 50%, #1a0000 100%); }
  100% { background: linear-gradient(135deg, #2a0000 0%, #5e2a2a 50%, #2a0000 100%); }
}

.container {
  background: rgba(255, 255, 255, 0.05);
  backdrop-filter: blur(20px);
  border-radius: 40px;
  padding: 50px 60px 60px;
  max-width: 520px;
  width: 100%;
  border: 1px solid rgba(255, 255, 255, 0.08);
  box-shadow: 0 30px 80px rgba(0, 0, 0, 0.6);
  transition: all 0.3s ease;
  position: relative;
  overflow: hidden;
}

.container::before {
  content: '';
  position: absolute;
  top: -50%;
  left: -50%;
  width: 200%;
  height: 200%;
  background: radial-gradient(circle at center, rgba(255,255,255,0.03) 0%, transparent 70%);
  pointer-events: none;
}

h1 {
  color: #ffffff;
  font-size: 26px;
  font-weight: 300;
  letter-spacing: 3px;
  text-align: center;
  margin-bottom: 6px;
}

h1 .shield {
  display: inline-block;
  animation: pulseShield 2s ease-in-out infinite;
}

@keyframes pulseShield {
  0%, 100% { transform: scale(1); }
  50% { transform: scale(1.1); }
}

.subtitle {
  text-align: center;
  color: rgba(255, 255, 255, 0.25);
  font-size: 12px;
  letter-spacing: 2px;
  margin-bottom: 35px;
  font-weight: 300;
}

/* 状态指示灯 */
.status-indicator {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 15px;
  margin-bottom: 30px;
  padding: 15px;
  background: rgba(255,255,255,0.03);
  border-radius: 20px;
  border: 1px solid rgba(255,255,255,0.05);
}

.status-light {
  width: 16px;
  height: 16px;
  border-radius: 50%;
  transition: all 0.3s ease;
  flex-shrink: 0;
}

.status-light.disarmed {
  background: #6bcb77;
  box-shadow: 0 0 20px rgba(107, 203, 119, 0.3);
}

.status-light.armed {
  background: #ffd93d;
  box-shadow: 0 0 20px rgba(255, 217, 61, 0.4);
  animation: pulseLight 1s ease-in-out infinite;
}

.status-light.alarm {
  background: #ff6b6b;
  box-shadow: 0 0 30px rgba(255, 107, 107, 0.6);
  animation: alarmPulse 0.3s ease-in-out infinite alternate;
}

@keyframes pulseLight {
  0%, 100% { opacity: 1; transform: scale(1); }
  50% { opacity: 0.5; transform: scale(0.9); }
}

@keyframes alarmPulse {
  0% { opacity: 1; transform: scale(1); box-shadow: 0 0 30px rgba(255, 107, 107, 0.6); }
  100% { opacity: 0.7; transform: scale(1.2); box-shadow: 0 0 50px rgba(255, 107, 107, 0.9); }
}

.status-text {
  color: rgba(255,255,255,0.7);
  font-size: 16px;
  font-weight: 300;
  letter-spacing: 0.5px;
}

.status-text .highlight {
  font-weight: 600;
  transition: color 0.3s ease;
}

.status-text .highlight.disarmed {
  color: #6bcb77;
}

.status-text .highlight.armed {
  color: #ffd93d;
}

.status-text .highlight.alarm {
  color: #ff6b6b;
  animation: textAlarm 0.3s ease-in-out infinite alternate;
}

@keyframes textAlarm {
  0% { opacity: 1; }
  100% { opacity: 0.5; }
}

/* 报警图标 */
.alarm-icon {
  text-align: center;
  font-size: 64px;
  margin: 10px 0 20px;
  transition: all 0.3s ease;
  opacity: 0.3;
  filter: grayscale(1);
}

.alarm-icon.active {
  opacity: 1;
  filter: grayscale(0);
  animation: alarmIconPulse 0.5s ease-in-out infinite alternate;
}

@keyframes alarmIconPulse {
  0% { transform: scale(1) rotate(0deg); }
  100% { transform: scale(1.1) rotate(5deg); }
}

/* 按钮组 */
.button-group {
  display: flex;
  gap: 15px;
  margin-top: 30px;
}

.btn {
  flex: 1;
  padding: 16px 20px;
  font-size: 16px;
  font-weight: 600;
  border: none;
  border-radius: 16px;
  cursor: pointer;
  transition: all 0.3s ease;
  letter-spacing: 1px;
  position: relative;
  overflow: hidden;
}

.btn::after {
  content: '';
  position: absolute;
  top: 50%;
  left: 50%;
  width: 0;
  height: 0;
  border-radius: 50%;
  background: rgba(255,255,255,0.2);
  transform: translate(-50%, -50%);
  transition: width 0.6s, height 0.6s;
}

.btn:active::after {
  width: 300px;
  height: 300px;
}

.btn-arm {
  background: linear-gradient(135deg, #00b894, #00a381);
  color: white;
  box-shadow: 0 4px 20px rgba(0, 184, 148, 0.3);
}

.btn-arm:hover {
  transform: translateY(-2px);
  box-shadow: 0 8px 30px rgba(0, 184, 148, 0.4);
}

.btn-arm:active {
  transform: translateY(0px);
}

.btn-arm:disabled {
  opacity: 0.4;
  cursor: not-allowed;
  transform: none;
}

.btn-disarm {
  background: linear-gradient(135deg, #e17055, #d63031);
  color: white;
  box-shadow: 0 4px 20px rgba(214, 48, 49, 0.3);
}

.btn-disarm:hover {
  transform: translateY(-2px);
  box-shadow: 0 8px 30px rgba(214, 48, 49, 0.4);
}

.btn-disarm:active {
  transform: translateY(0px);
}

.btn-disarm:disabled {
  opacity: 0.4;
  cursor: not-allowed;
  transform: none;
}

/* 触摸状态显示 */
.touch-status {
  margin-top: 25px;
  padding: 12px;
  background: rgba(255,255,255,0.03);
  border-radius: 12px;
  text-align: center;
  color: rgba(255,255,255,0.2);
  font-size: 12px;
  letter-spacing: 0.5px;
  border: 1px solid rgba(255,255,255,0.03);
}

.touch-status .value {
  color: rgba(255,255,255,0.4);
  font-weight: 600;
}

.touch-status .sensor-triggered {
  color: #ff6b6b;
  animation: textAlarm 0.3s ease-in-out infinite alternate;
}

/* 响应式 */
@media (max-width: 480px) {
  .container {
    padding: 30px 25px 35px;
  }
  h1 {
    font-size: 20px;
  }
  .button-group {
    flex-direction: column;
  }
  .btn {
    padding: 14px;
  }
  .status-indicator {
    flex-direction: column;
    gap: 8px;
  }
}
</style>
</head>
<body>
<div class="container" id="container">
  <h1><span class="shield">🛡️</span> 安防报警主机</h1>
  <div class="subtitle">ESP32 AP模式 智能安防系统</div>
  
  <div class="status-indicator">
    <div class="status-light" id="statusLight"></div>
    <div class="status-text">
      系统状态：<span class="highlight" id="statusText">加载中...</span>
    </div>
  </div>
  
  <div class="alarm-icon" id="alarmIcon">🔔</div>
  
  <div class="button-group">
    <button class="btn btn-arm" id="armBtn" onclick="sendCommand('arm')">
      🔒 布防
    </button>
    <button class="btn btn-disarm" id="disarmBtn" onclick="sendCommand('disarm')">
      🔓 撤防
    </button>
  </div>
  
  <div class="touch-status">
    触摸传感器：<span class="value" id="touchValue">--</span>
    <span id="touchIndicator"></span>
  </div>
</div>

<script>
let currentState = {
  armed: false,
  alarm: false,
  touchValue: 0
};

// 获取状态并更新UI
function fetchStatus() {
  fetch("/status")
    .then(res => res.json())
    .then(data => {
      currentState = data;
      updateUI(data);
    })
    .catch(err => {
      console.log('获取状态失败:', err);
    });
}

// 发送命令
function sendCommand(cmd) {
  fetch("/" + cmd)
    .then(res => res.text())
    .then(() => {
      // 等待状态更新
      setTimeout(fetchStatus, 200);
    })
    .catch(err => {
      console.log('发送命令失败:', err);
    });
}

// 更新UI
function updateUI(data) {
  const light = document.getElementById('statusLight');
  const text = document.getElementById('statusText');
  const icon = document.getElementById('alarmIcon');
  const container = document.getElementById('container');
  const armBtn = document.getElementById('armBtn');
  const disarmBtn = document.getElementById('disarmBtn');
  const touchValue = document.getElementById('touchValue');
  const touchIndicator = document.getElementById('touchIndicator');
  
  // 更新触摸值
  touchValue.textContent = data.touchValue || '--';
  
  // 判断触摸状态
  if (data.touchValue < 50) {
    touchIndicator.textContent = ' 👆 触摸中';
    touchIndicator.className = 'sensor-triggered';
  } else {
    touchIndicator.textContent = '';
    touchIndicator.className = '';
  }
  
  // 更新状态
  if (data.alarm) {
    // 报警状态
    light.className = 'status-light alarm';
    text.textContent = '⚠️ 报警触发中！';
    text.className = 'highlight alarm';
    icon.className = 'alarm-icon active';
    container.className = 'container alarm-active';
    armBtn.disabled = true;
    disarmBtn.disabled = false;
  } else if (data.armed) {
    // 布防状态
    light.className = 'status-light armed';
    text.textContent = '🔒 已布防';
    text.className = 'highlight armed';
    icon.className = 'alarm-icon';
    container.className = 'container';
    armBtn.disabled = true;
    disarmBtn.disabled = false;
  } else {
    // 撤防状态
    light.className = 'status-light disarmed';
    text.textContent = '🔓 已撤防';
    text.className = 'highlight disarmed';
    icon.className = 'alarm-icon';
    container.className = 'container';
    armBtn.disabled = false;
    disarmBtn.disabled = true;
  }
}

// 每200ms刷新状态
fetchStatus();
setInterval(fetchStatus, 200);
</script>
</body>
</html>
)raw";
  return html;
}

// 首页路由
void handleRoot() {
  server.send(200, "text/html;charset=utf-8", makeHtml());
}

// 布防路由
void handleArm() {
  isArm = true;
  alarmTrigger = false;  // 布防时重置报警
  digitalWrite(LED_PIN, HIGH); // 熄灭LED
  Serial.println("🔒 系统已布防");
  server.send(200, "text/plain", "OK");
}

// 撤防路由
void handleDisarm() {
  isArm = false;
  alarmTrigger = false;
  digitalWrite(LED_PIN, HIGH);
  Serial.println("🔓 系统已撤防");
  server.send(200, "text/plain", "OK");
}

// 状态查询接口 - 返回JSON
void handleStatus() {
  int touchVal = touchRead(TOUCH_PIN);
  String json = "{";
  json += "\"armed\":" + String(isArm ? "true" : "false") + ",";
  json += "\"alarm\":" + String(alarmTrigger ? "true" : "false") + ",";
  json += "\"touchValue\":" + String(touchVal);
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("========================================");
  Serial.println("🛡️ ESP32 安防报警系统启动 (AP模式)");
  Serial.println("========================================");

  // LED引脚初始化（反相默认熄灭）
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // 设置ESP32为AP模式（创建WiFi热点）
  Serial.println("📡 正在创建WiFi热点...");
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.println("✅ WiFi热点创建成功！");
  Serial.print("📶 热点名称：");
  Serial.println(ssid);
  Serial.print("🔑 热点密码：");
  Serial.println(password);
  Serial.print("🌐 网页访问地址：http://");
  Serial.println(IP);
  Serial.println("========================================");

  // 绑定网页路由
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.on("/status", handleStatus);
  server.begin();
  Serial.println("🌍 Web服务器已启动");
  Serial.println("📱 请用手机/电脑连接热点后，在浏览器访问上述地址");
}

void loop() {
  server.handleClient();
  
  // 读取触摸值
  int touchVal = touchRead(TOUCH_PIN);
  bool touchNow = (touchVal < touchThreshold);

  // 布防 + 触摸触发报警（仅当未报警时触发）
  if (isArm && touchNow && !alarmTrigger) {
    alarmTrigger = true;
    Serial.println("🚨 报警触发！");
  }

  // 报警闪烁控制
  if (alarmTrigger) {
    digitalWrite(LED_PIN, LOW);  // 点亮
    delay(100);
    digitalWrite(LED_PIN, HIGH); // 熄灭
    delay(100);
  }
}