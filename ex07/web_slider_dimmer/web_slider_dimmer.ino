// ex07 Web网页滑动条无极PWM调光 - 精美UI版 (AP模式)
#include <WiFi.h>
#include <WebServer.h>

// WiFi AP配置 - ESP32自己创建热点
const char* ssid = "ESP32_PWM_Light";  // 热点名称
const char* password = "12345678";      // 热点密码（至少8位）

// PWM硬件配置
#define LED_PIN 2
const int pwmFreq = 5000;
const int pwmRes = 8;
const int pwmChan = 0;

WebServer server(80);

// 生成带滑动条+JS实时请求的网页（精美UI版）
String makeHtmlPage(){
  String html = R"raw(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Web无极调光器</title>
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
}

.container {
  background: rgba(255, 255, 255, 0.05);
  backdrop-filter: blur(20px);
  border-radius: 40px;
  padding: 50px 60px 60px;
  max-width: 550px;
  width: 100%;
  border: 1px solid rgba(255, 255, 255, 0.08);
  box-shadow: 0 30px 80px rgba(0, 0, 0, 0.6), inset 0 1px 0 rgba(255, 255, 255, 0.06);
  transition: border-color 0.3s ease;
}

h1 {
  color: #ffffff;
  font-size: 28px;
  font-weight: 300;
  letter-spacing: 3px;
  text-align: center;
  margin-bottom: 8px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.subtitle {
  text-align: center;
  color: rgba(255, 255, 255, 0.3);
  font-size: 13px;
  letter-spacing: 2px;
  margin-bottom: 40px;
  font-weight: 300;
}

/* LED 模拟显示 */
.led-display {
  width: 120px;
  height: 120px;
  margin: 0 auto 40px;
  border-radius: 50%;
  background: radial-gradient(circle at 30% 30%, rgba(255,255,255,0.2), rgba(0,0,0,0.2));
  position: relative;
  transition: all 0.15s ease;
  box-shadow: 0 0 30px rgba(255, 255, 255, 0.05);
}

.led-display::before {
  content: '';
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  width: 80%;
  height: 80%;
  border-radius: 50%;
  background: radial-gradient(circle at 40% 35%, #ffffff, #444444);
  transition: all 0.15s ease;
  box-shadow: inset 0 -8px 20px rgba(0,0,0,0.4);
}

.led-display.active::before {
  background: radial-gradient(circle at 40% 35%, #ffe066, #ff8800);
  box-shadow: 0 0 60px rgba(255, 136, 0, 0.4), 0 0 120px rgba(255, 136, 0, 0.2), inset 0 -8px 20px rgba(0,0,0,0.3);
}

.led-display .label {
  position: absolute;
  bottom: -30px;
  left: 50%;
  transform: translateX(-50%);
  color: rgba(255,255,255,0.2);
  font-size: 11px;
  letter-spacing: 1px;
  white-space: nowrap;
}

/* 数值显示 */
.value-display {
  display: flex;
  justify-content: center;
  align-items: baseline;
  gap: 6px;
  margin-bottom: 35px;
}

#brightVal {
  font-size: 72px;
  font-weight: 700;
  color: #ffffff;
  line-height: 1;
  transition: color 0.3s ease;
  text-shadow: 0 0 40px rgba(255,255,255,0.05);
  font-variant-numeric: tabular-nums;
}

.value-display .percent {
  color: rgba(255,255,255,0.2);
  font-size: 24px;
  font-weight: 300;
}

/* 自定义滑动条 */
.slider-container {
  position: relative;
  padding: 10px 0;
}

input[type="range"] {
  -webkit-appearance: none;
  appearance: none;
  width: 100%;
  height: 6px;
  border-radius: 3px;
  outline: none;
  background: linear-gradient(to right, rgba(102, 126, 234, 0.2), rgba(118, 75, 162, 0.2));
  transition: background 0.3s ease;
  cursor: pointer;
}

input[type="range"]::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 24px;
  height: 24px;
  border-radius: 50%;
  background: radial-gradient(circle at 40% 35%, #667eea, #4a3f7a);
  cursor: pointer;
  transition: all 0.2s ease;
  box-shadow: 0 0 20px rgba(102, 126, 234, 0.3), 0 4px 15px rgba(0,0,0,0.3);
  border: 2px solid rgba(255,255,255,0.15);
}

input[type="range"]::-webkit-slider-thumb:hover {
  transform: scale(1.1);
  box-shadow: 0 0 30px rgba(102, 126, 234, 0.5), 0 4px 20px rgba(0,0,0,0.4);
}

input[type="range"]::-webkit-slider-thumb:active {
  transform: scale(0.95);
}

input[type="range"]::-moz-range-thumb {
  width: 24px;
  height: 24px;
  border-radius: 50%;
  background: radial-gradient(circle at 40% 35%, #667eea, #4a3f7a);
  cursor: pointer;
  border: 2px solid rgba(255,255,255,0.15);
  box-shadow: 0 0 20px rgba(102, 126, 234, 0.3);
}

/* 亮度刻度标记 */
.ticks {
  display: flex;
  justify-content: space-between;
  padding: 8px 2px 0;
  color: rgba(255,255,255,0.15);
  font-size: 10px;
  letter-spacing: 0.5px;
}

/* 状态信息 */
.info-bar {
  display: flex;
  justify-content: space-between;
  margin-top: 30px;
  padding-top: 25px;
  border-top: 1px solid rgba(255,255,255,0.05);
  color: rgba(255,255,255,0.2);
  font-size: 12px;
  letter-spacing: 0.5px;
}

.info-bar .status {
  display: flex;
  align-items: center;
  gap: 8px;
}

.info-bar .dot {
  width: 6px;
  height: 6px;
  border-radius: 50%;
  background: #6bcb77;
  display: inline-block;
  animation: pulse-dot 2s ease-in-out infinite;
}

@keyframes pulse-dot {
  0%, 100% { opacity: 0.3; }
  50% { opacity: 1; }
}

.info-bar .connection-info {
  color: rgba(255,255,255,0.15);
}

/* 响应式 */
@media (max-width: 480px) {
  .container {
    padding: 30px 25px 35px;
  }
  h1 {
    font-size: 22px;
  }
  #brightVal {
    font-size: 56px;
  }
  .led-display {
    width: 90px;
    height: 90px;
  }
}
</style>
</head>
<body>
<div class="container" id="container">
  <h1>💡 无极调光器</h1>
  <div class="subtitle">ESP32 AP模式 实时控制</div>
  
  <div class="led-display" id="ledDisplay">
    <span class="label">LED 状态</span>
  </div>
  
  <div class="value-display">
    <span id="brightVal">128</span>
    <span class="percent">%</span>
  </div>
  
  <div class="slider-container">
    <input type="range" id="slider" min="0" max="255" value="128">
    <div class="ticks">
      <span>0%</span>
      <span>50%</span>
      <span>100%</span>
    </div>
  </div>
  
  <div class="info-bar">
    <div class="status">
      <span class="dot"></span>
      <span>实时连接</span>
    </div>
    <div class="connection-info">
      <span id="ipDisplay">ESP32-AP</span>
    </div>
  </div>
</div>

<script>
const slider = document.getElementById("slider");
const valText = document.getElementById("brightVal");
const ledDisplay = document.getElementById("ledDisplay");
const container = document.getElementById("container");

// 更新UI状态
function updateUI(value) {
  const num = parseInt(value);
  const percent = Math.round((num / 255) * 100);
  
  // 更新数值显示
  valText.innerText = percent;
  
  // 更新LED模拟
  if (num > 10) {
    ledDisplay.classList.add('active');
    // 根据亮度调整发光颜色温度
    const intensity = num / 255;
    const r = Math.round(255 * intensity);
    const g = Math.round(200 * intensity * intensity);
    const b = Math.round(100 * intensity * intensity * intensity);
    ledDisplay.style.boxShadow = `0 0 ${30 + 70 * intensity}px rgba(255, ${Math.round(150 * intensity)}, 0, ${0.1 + 0.3 * intensity})`;
  } else {
    ledDisplay.classList.remove('active');
    ledDisplay.style.boxShadow = '0 0 30px rgba(255,255,255,0.05)';
  }
  
  // 更新数值颜色
  const hue = 40 - (num / 255) * 30;
  valText.style.color = `hsl(${hue}, 100%, ${50 + 20 * (num/255)}%)`;
  valText.style.textShadow = `0 0 ${20 + 40 * (num/255)}px hsla(${hue}, 100%, 60%, ${0.1 + 0.2 * (num/255)})`;
  
  // 更新边框发光
  container.style.borderColor = `rgba(255, ${Math.round(150 * num/255)}, 0, ${0.05 + 0.1 * (num/255)})`;
}

// 发送亮度到ESP32
function sendBrightness(value) {
  fetch("/set?bright=" + value)
    .then(response => {
      if (!response.ok) throw new Error('网络错误');
      return response.text();
    })
    .catch(err => {
      console.log('发送失败:', err);
    });
}

// 滑动条事件
slider.addEventListener("input", function() {
  const val = this.value;
  updateUI(val);
  sendBrightness(val);
});

// 初始化UI
updateUI(slider.value);

// 获取并显示IP地址（通过页面加载时获取）
fetch("/ip")
  .then(res => res.text())
  .then(ip => {
    document.getElementById("ipDisplay").textContent = ip;
  })
  .catch(() => {});
</script>
</body>
</html>
)raw";
  return html;
}

// 首页路由：返回完整网页
void handleRoot(){
  server.send(200, "text/html;charset=utf-8", makeHtmlPage());
}

// 亮度设置路由：解析URL参数，设置PWM占空比
void handleSetBright(){
  if(server.hasArg("bright")){
    int bright = server.arg("bright").toInt();
    // 限制数值0~255
    if(bright < 0) bright = 0;
    if(bright > 255) bright = 255;
    ledcWrite(pwmChan, bright);
    Serial.print("收到亮度指令：");
    Serial.println(bright);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing bright parameter");
  }
}

// 新增：获取IP地址接口（用于页面显示）
void handleGetIP(){
  String ip = WiFi.softAPIP().toString();  // 获取AP模式的IP
  server.send(200, "text/plain", ip);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("===== 设备上电初始化完成 =====");

  // PWM初始化
  ledcSetup(pwmChan, pwmFreq, pwmRes);
  ledcAttachPin(LED_PIN, pwmChan);
  ledcWrite(pwmChan, 128); // 初始亮度50%

  // 设置ESP32为AP模式（创建WiFi热点）
  Serial.println("正在创建WiFi热点...");
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.println("WiFi热点创建成功！");
  Serial.print("热点名称：");
  Serial.println(ssid);
  Serial.print("热点密码：");
  Serial.println(password);
  Serial.print("网页访问地址：http://");
  Serial.println(IP);

  // 绑定路由
  server.on("/", handleRoot);
  server.on("/set", handleSetBright);
  server.on("/ip", handleGetIP);
  server.begin();
  Serial.println("Web服务已启动");
  Serial.println("请用手机/电脑连接热点后，在浏览器访问上述地址");
}

void loop() {
  server.handleClient();
}