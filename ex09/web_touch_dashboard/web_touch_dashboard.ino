// ex09 实时触摸数值Web仪表盘，AJAX自动刷新 - AP模式
#include <WiFi.h>
#include <WebServer.h>

// WiFi AP配置 - ESP32自己创建热点
const char* ssid = "ESP32_Touch_Dashboard";  // 热点名称
const char* password = "12345678";            // 热点密码（至少8位）

#define TOUCH_PIN 4  // GPIO4 支持触摸功能 (T0)
WebServer server(80);

// 仪表盘网页：JS定时拉取传感器数据，带可视化仪表盘效果
String makeHtml(){
  String html = R"raw(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>触摸传感器实时仪表盘</title>
<style>
*{margin:0;padding:0;box-sizing:border-box;}
body{
  text-align:center;
  font-family:'Segoe UI',Arial,sans-serif;
  min-height:100vh;
  background:linear-gradient(135deg, #0c0c1e 0%, #1a1a3e 50%, #0c0c1e 100%);
  display:flex;
  justify-content:center;
  align-items:center;
  padding:20px;
}
.container{
  background:rgba(255,255,255,0.05);
  backdrop-filter:blur(10px);
  border-radius:30px;
  padding:40px 50px 50px;
  box-shadow:0 20px 60px rgba(0,0,0,0.8), inset 0 1px 0 rgba(255,255,255,0.1);
  border:1px solid rgba(255,255,255,0.05);
  max-width:500px;
  width:100%;
}
h1{
  color:#00d4ff;
  font-size:24px;
  font-weight:300;
  letter-spacing:2px;
  margin-bottom:10px;
  text-shadow:0 0 20px rgba(0,212,255,0.3);
}
.subtitle{
  color:rgba(255,255,255,0.4);
  font-size:14px;
  letter-spacing:1px;
  margin-bottom:30px;
}
/* 仪表盘圆形 */
.meter{
  width:280px;
  height:280px;
  margin:0 auto 30px;
  position:relative;
}
.meter-svg{
  transform:rotate(-90deg);
  width:100%;
  height:100%;
}
.meter-bg{
  fill:none;
  stroke:rgba(255,255,255,0.08);
  stroke-width:12;
}
.meter-progress{
  fill:none;
  stroke:#00d4ff;
  stroke-width:12;
  stroke-linecap:round;
  transition:stroke-dashoffset 0.15s ease-out;
  filter:drop-shadow(0 0 8px rgba(0,212,255,0.4));
}
.meter-center{
  position:absolute;
  top:50%;
  left:50%;
  transform:translate(-50%,-50%);
  text-align:center;
}
#dataBox{
  font-size:56px;
  font-weight:700;
  color:#ffffff;
  line-height:1;
  text-shadow:0 0 30px rgba(0,212,255,0.2);
  transition:color 0.3s;
}
#unit{
  color:rgba(255,255,255,0.4);
  font-size:14px;
  margin-top:5px;
  letter-spacing:2px;
}
/* 状态指示 */
.status-container{
  display:flex;
  justify-content:center;
  align-items:center;
  gap:12px;
  padding:12px 20px;
  background:rgba(255,255,255,0.03);
  border-radius:50px;
  border:1px solid rgba(255,255,255,0.06);
}
.status-dot{
  width:10px;
  height:10px;
  border-radius:50%;
  display:inline-block;
  transition:background 0.3s, box-shadow 0.3s;
}
#statusText{
  color:rgba(255,255,255,0.7);
  font-size:15px;
  font-weight:300;
  letter-spacing:0.5px;
}
/* 数值范围条 */
.range-bar{
  margin-top:20px;
  display:flex;
  justify-content:space-between;
  align-items:center;
  gap:10px;
}
.range-track{
  flex:1;
  height:3px;
  background:rgba(255,255,255,0.08);
  border-radius:2px;
  position:relative;
  overflow:hidden;
}
.range-fill{
  height:100%;
  background:linear-gradient(90deg, #ff6b6b, #ffd93d, #6bcb77);
  border-radius:2px;
  transition:width 0.15s ease-out;
  width:0%;
}
.range-label{
  color:rgba(255,255,255,0.3);
  font-size:11px;
  min-width:30px;
}
/* 刷新指示 */
.refresh-badge{
  margin-top:20px;
  color:rgba(255,255,255,0.15);
  font-size:12px;
  letter-spacing:1px;
}
.refresh-badge span{
  display:inline-block;
  animation:pulse 1.5s ease-in-out infinite;
}
@keyframes pulse{
  0%,100%{opacity:0.3;}
  50%{opacity:1;}
}
</style>
</head>
<body>
<div class="container">
  <h1>⚡ 触摸监控面板</h1>
  <div class="subtitle">ESP32 AP模式 实时数据采集</div>
  
  <div class="meter">
    <svg class="meter-svg" viewBox="0 0 200 200">
      <circle class="meter-bg" cx="100" cy="100" r="85"/>
      <circle class="meter-progress" id="progressCircle" cx="100" cy="100" r="85"
        stroke-dasharray="534.07" stroke-dashoffset="534.07"/>
    </svg>
    <div class="meter-center">
      <div id="dataBox">--</div>
      <div id="unit">触摸值</div>
    </div>
  </div>
  
  <div class="status-container">
    <span class="status-dot" id="statusDot"></span>
    <span id="statusText">初始化中...</span>
  </div>
  
  <div class="range-bar">
    <span class="range-label">0</span>
    <div class="range-track">
      <div class="range-fill" id="rangeFill"></div>
    </div>
    <span class="range-label">100</span>
  </div>
  
  <div class="refresh-badge">
    <span>●</span> 实时刷新 10次/秒
  </div>
</div>

<script>
const TOUCH_MIN = 0;
const TOUCH_MAX = 100;  // 触摸值范围0~100（实际ESP32触摸值0~4095，但我们归一化显示）
const CIRCUMFERENCE = 534.07; // 2*PI*85

function refreshData(){
  fetch("/getTouch")
    .then(res=>{
      if(!res.ok) throw new Error('网络错误');
      return res.text();
    })
    .then(val=>{
      const raw = parseInt(val);
      if(isNaN(raw)) throw new Error('无效数据');
      
      // 显示原始值
      document.getElementById("dataBox").innerText = raw;
      
      // 归一化到0-100（ESP32触摸值通常0~100左右，手指触摸时降到10-30）
      const normalized = Math.min(100, Math.max(0, raw));
      const percent = (normalized / 100) * 100;
      
      // 更新环形进度
      const offset = CIRCUMFERENCE - (percent / 100) * CIRCUMFERENCE;
      document.getElementById("progressCircle").style.strokeDashoffset = offset;
      
      // 更新进度条
      document.getElementById("rangeFill").style.width = percent + '%';
      
      // 更新状态
      const dot = document.getElementById("statusDot");
      const text = document.getElementById("statusText");
      
      if(raw < 20) {
        dot.style.background = '#ff6b6b';
        dot.style.boxShadow = '0 0 15px rgba(255,107,107,0.6)';
        text.textContent = '👆 手指触摸中';
        text.style.color = '#ff6b6b';
        document.getElementById("dataBox").style.color = '#ff6b6b';
      } else if(raw < 50) {
        dot.style.background = '#ffd93d';
        dot.style.boxShadow = '0 0 15px rgba(255,217,61,0.5)';
        text.textContent = '🖐️ 靠近感应区';
        text.style.color = '#ffd93d';
        document.getElementById("dataBox").style.color = '#ffd93d';
      } else {
        dot.style.background = '#6bcb77';
        dot.style.boxShadow = '0 0 15px rgba(107,203,119,0.5)';
        text.textContent = '✋ 未触摸';
        text.style.color = '#6bcb77';
        document.getElementById("dataBox").style.color = '#6bcb77';
      }
    })
    .catch(err=>{
      document.getElementById("dataBox").innerText = '--';
      document.getElementById("statusText").textContent = '⚠️ 连接中...';
      document.getElementById("statusDot").style.background = '#ff6b6b';
    });
}

// 首次立即加载，然后每100ms刷新
refreshData();
setInterval(refreshData, 100);
</script>
</body>
</html>
)raw";
  return html;
}

// 首页返回仪表盘页面
void handleRoot(){
  server.send(200, "text/html;charset=utf-8", makeHtml());
}

// 数据接口：仅返回当前触摸数值，供网页AJAX拉取
void handleGetTouch(){
  int val = touchRead(TOUCH_PIN);
  server.send(200, "text/plain", String(val));
}

void setup() {
  Serial.begin(115200);
  delay(800); // 等待串口硬件稳定，避免开机丢失打印
  Serial.println("========================================");
  Serial.println("⚡ ESP32触摸仪表盘启动 (AP模式)");
  Serial.println("========================================");

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
  server.on("/getTouch", handleGetTouch);
  server.begin();
  Serial.println("🌍 Web服务器已启动");
  Serial.println("📱 请用手机/电脑连接热点后，在浏览器访问上述地址");
}

void loop() {
  server.handleClient();
}