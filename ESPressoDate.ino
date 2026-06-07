/*
This is the Arduino "uno-date" program that get's you a data

by setting up a wireless access point with the name of "Truth or Dare"

When a person connects to it it will show some questions to determin if 
they want to have a date with you.

The code is made for trying to make coding fun and find a purpose where 
hardware and software is used in learning technologies. 

How to install/build the code: 

Hardware: ESP32 Seeed Studio XIAO ESP32C6 WiFi 6+Bluetooth-compatible Ble 5 Support Zigbee Matter Wireless Development Board

https://www.aliexpress.com/item/1005006946443492.html

This code is mader for the esp32 hardware called Seeed Studio XIAO ESP32C6

https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/

To compile it just start arduino IDE ( i use version 2.3.9 ) then go to:

File -> Preferences -> Additional Boards Manager URL 

And add this: https://espressif.github.io/arduino-esp32/package_esp32_index.json


Tools -> Boards -> Boards managers ( or just press CTRL + SHIFT + B if it works ? )

Then search for "esp32 by Espressif Systems" and install it. 
I had version 3.3.10 installed when i wrote this code)

(C)opyleft Keld Norman, 2025

*/
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <LittleFS.h>
#define SERIAL_BAUD 115200
#define AP_SSID "Truth or Dare? 😏"
#define AP_CHANNEL 6
#define AP_MAX_CLIENTS 4
#define ACTIVE_CLIENT_LIMIT 2
#define LOG_FILE "/answers.csv"
#define ADMIN_KEY "1337"
static const char date_name[] = "Your Name";
static const char date_phone[] = "+xx xxxxxxxx";
static const char date_instagram[] = "@your.instagram";
static const char date_mail[] = "your@email.com";
static const bool external_antenna = true;
static const uint8_t RF_SWITCH_ENABLE_PIN = 3;
static const uint8_t RF_ANTENNA_SELECT_PIN = 14;
DNSServer dnsServer;
WebServer server(80);
static const char busyPage[] = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
<title>Busy</title>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
body{font-family:Arial;margin:18px;background:#fff0f6;color:#3a0a1e}
.box{max-width:520px;margin:auto;background:white;padding:22px;border-radius:18px;box-shadow:0 0 18px #ffc2d6;text-align:center}
h1{color:#000;font-size:30px;margin:8px 0 6px}
.small{color:#8a3a5a;font-size:15px}
</style>
</head>
<body>
<div class="box"><h1>Busy</h1><p class="small">Too many people are connected right now.</p><p>Try again in a moment ☕</p></div>
</body>
</html>
)HTML";
static const char appPage[] = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
<title>Truth or Dare</title>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
body{font-family:Arial;margin:18px;background:#fff0f6;color:#3a0a1e}
.box{max-width:520px;margin:auto;background:white;padding:22px;border-radius:18px;box-shadow:0 0 18px #ffc2d6;text-align:center}
h1{color:#d63384;font-size:30px;margin:8px 0 6px}
.small{color:#8a3a5a;font-size:15px;margin:0 0 22px}
.btn{display:block;width:70%;max-width:260px;font-size:17px;padding:10px 12px;margin:12px auto;border:0;border-radius:12px;background:#d63384;color:white;text-decoration:none;text-align:center}
.btn2{background:#000}
.wide{width:88%;max-width:330px}
.q{margin-top:20px;font-weight:bold}
button{display:block;width:82%;max-width:330px;font-size:16px;padding:10px 16px;margin:10px auto;border:0;border-radius:12px;background:#d63384;color:white}
.back{background:#777}
.bigArrow{display:block;font-size:64px;line-height:1;color:#c62828;background:transparent;border:0;margin:0 0 8px 0;padding:0;text-align:center}
.green{background:#2e7d32;color:#fff;font-weight:bold}
.gold{background:#d4af37;color:#000;font-weight:bold}
.plan{font-size:17px;line-height:1.45;text-align:center;margin:12px 0}
.hint{font-size:16px;color:#000;text-align:center;margin:10px 0}
.blackTitle{color:#000}
.contactLine{color:#000;margin:2px 0;line-height:1.25}
.contactLink{color:#000;text-decoration:underline}
.smsBox{width:92%;max-width:390px;height:118px;margin:6px auto 4px auto;padding:10px;border-radius:10px;border:1px solid #ccc;font-family:Arial;font-size:14px;color:#000}
.adminInput{width:80%;max-width:230px;font-size:24px;text-align:center;padding:10px;border-radius:10px;border:1px solid #ccc;margin:10px auto}
#below{text-align:center;margin-top:14px}
</style>
</head>
<body>
<div class="box" id="app"></div>
<div id="below"></div>
<script>
const app=document.getElementById("app");
const below=document.getElementById("below");
const dateName='%%DATE_NAME%%';
const datePhone='%%DATE_PHONE%%';
const dateInstagram='%%DATE_INSTAGRAM%%';
const dateMail='%%DATE_MAIL%%';
const datePhoneTel='%%DATE_PHONE_TEL%%';
const smsText='Hi '+dateName+' ('+datePhone+')\n\nI would like to go on a date with you 💕\n\nHere is my number:\n\nRegards.';
let step=0;
let answers={};
function logAnswer(path,answer){
fetch('/log?path='+encodeURIComponent(path)+'&answer='+encodeURIComponent(answer)+'&q1='+(answers.q1||'')+'&q2='+(answers.q2||''),{cache:'no-store'}).catch(()=>{});
}
function adminPrompt(){
below.innerHTML='';
app.innerHTML='<h1>Admin code</h1><p class="small">Enter code</p><input id="adminCode" class="adminInput" type="number" inputmode="numeric" pattern="[0-9]*" autofocus><button onclick="checkAdminCode()">Open</button><button class="back" onclick="home()">Back</button>';
setTimeout(()=>{document.getElementById('adminCode').focus();},100);
}
function checkAdminCode(){
let k=document.getElementById('adminCode').value;
if(k==='1337'){location.href='/admin?k='+encodeURIComponent(k);}
else{home();}
}
function copySms(){
let t=document.getElementById('smsText');
t.focus();
t.select();
t.setSelectionRange(0,99999);
document.execCommand('copy');
}
function drinkText(){
if(answers.q1==='yes'||answers.q1==='maybe'){return 'coffee ☕';}
return 'tea 🍵';
}
function snackText(){
if(answers.q2==='of course'||answers.q2==='depends on the cake'){return 'cake 🍰';}
return 'something sweet without cake';
}
function home(){
step=0;
answers={};
below.innerHTML='';
app.innerHTML='<h1>Truth or Dare? <span onclick="adminPrompt()" style="cursor:pointer">💘</span></h1><p class="small">Choose wisely:</p><button class="btn" onclick="truth()">Truth</button><button class="btn btn2" onclick="dare()">Dare</button>';
}
function truth(){
below.innerHTML='';
step=1;
truthStep();
}
function truthStep(){
below.innerHTML='';
let html='';
if(step===1){
html='<h1>Truth</h1><p class="small">Question 1 of 3</p><p class="q">Would coffee with a charming person<br>be a good idea? ☕</p><button onclick="answers.q1=\'yes\';step=2;truthStep()">Yes 💕</button><button onclick="answers.q1=\'maybe\';step=2;truthStep()">Maybe</button><button onclick="answers.q1=\'no\';step=2;truthStep()">No</button><p><button class="back" onclick="home()">Back</button></p>';
}
if(step===2){
html='<h1>Truth</h1><p class="small">Question 2 of 3</p><p class="q">Would cake make the situation better? 🍰</p><button onclick="answers.q2=\'of course\';step=3;truthStep()">Of course 💕</button><button onclick="answers.q2=\'depends on the cake\';step=3;truthStep()">It depends on the cake</button><button onclick="answers.q2=\'no cake\';step=3;truthStep()">No cake</button><p><button class="back" onclick="step=1;truthStep()">Back</button></p>';
}
if(step===3){
html='<h1>Truth</h1><p class="small">Last question</p><p class="q">Would you go on a date with me? 💘</p><button onclick="logAnswer(\'truth\',\'yes\');resultYes()">Yes 💕</button><button onclick="logAnswer(\'truth\',\'maybe\');resultMaybe()">Maybe, ask me nicely again ☕</button><button onclick="logAnswer(\'truth\',\'no\');resultNo()">No, but this was sweet</button><p><button class="back" onclick="step=2;truthStep()">Back</button></p>';
}
app.innerHTML=html;
}
function dare(){
below.innerHTML='';
app.innerHTML='<h1 class="blackTitle">Dare ☕</h1><p class="small">Your dare is simple.</p><p>Smile, look at me, and choose one:</p><button class="btn wide btn2" onclick="answers.q1=\'yes\';answers.q2=\'of course\';logAnswer(\'dare\',\'dare-yes\');resultYes()">Yes, coffee sounds good ☕</button><button class="btn wide btn2" onclick="answers.q1=\'yes\';answers.q2=\'of course\';logAnswer(\'dare\',\'dare-maybe\');resultMaybe()">Maybe, ask me properly 💌</button><button class="btn wide btn2" onclick="logAnswer(\'dare\',\'dare-no\');resultNo()">No, but good try</button><p><button class="back" onclick="home()">Back</button></p>';
}
function contact(){
below.innerHTML='';
app.innerHTML='<h1 class="blackTitle">Contact info</h1><br><p class="contactLine"><b>Name:</b> '+dateName+'<br><b>Instagram:</b> '+dateInstagram+'</p><p class="contactLine"><b>Mail:</b> <a class="contactLink" href="mailto:'+dateMail+'">'+dateMail+'</a></p><p class="contactLine"><b>Phone:</b> <a class="contactLink" href="tel:'+datePhoneTel+'">'+datePhone+'</a></p><br><textarea id="smsText" class="smsBox" readonly>'+smsText+'</textarea><p style="margin:4px 0"><button class="green" onclick="copySms()">Copy text</button></p><p style="color:#000;margin:4px 0">I hope you write 💌</p><p style="margin:4px 0"><button class="back" onclick="home()">Back</button></p>';
}
function resultYes(){
app.innerHTML='<h1>Accepted 💕</h1><p>Perfect. Date unlocked.</p><p class="plan">Suggestion: '+drinkText()+', '+snackText()+'<br>and suspiciously good conversation.</p>';
below.innerHTML='<p class="hint">Press the show contact info button!</p><div class="bigArrow">&#8595;</div><button class="back green" onclick="contact()">Show contact info</button><button class="back" onclick="home()">Start over</button>';
}
function resultMaybe(){
below.innerHTML='';
app.innerHTML='<h1>Maybe ☕</h1><p>Accepted. I will ask properly.</p><p>Would you like to go out for coffee with me? 💌</p><p><button class="back" onclick="home()">Start over</button></p>';
}
function resultNo(){
below.innerHTML='';
app.innerHTML='<h1>Good try</h1><p>Fair answer. No hard feelings.</p><p><button class="back" onclick="home()">Start over</button></p>';
}
home();
</script>
</body>
</html>
)HTML";
String makePhoneTelValue(const char *phone) {
  String value = String(phone);
  value.replace(" ", "");
  return value;
}
String makeAppPage() {
  String page = appPage;
  page.replace("%%DATE_NAME%%", date_name);
  page.replace("%%DATE_PHONE%%", date_phone);
  page.replace("%%DATE_INSTAGRAM%%", date_instagram);
  page.replace("%%DATE_MAIL%%", date_mail);
  page.replace("%%DATE_PHONE_TEL%%", makePhoneTelValue(date_phone));
  return page;
}
String cleanValue(String value) {
  value.replace(",", " ");
  value.replace("\n", " ");
  value.replace("\r", " ");
  return value;
}
String htmlEscape(String value) {
  value.replace("&", "&amp;");
  value.replace("<", "&lt;");
  value.replace(">", "&gt;");
  value.replace("\"", "&quot;");
  return value;
}
String answerLabel(String value) {
  if (value == "yes") return "Yes";
  if (value == "maybe") return "Maybe";
  if (value == "no") return "No";
  if (value == "of course") return "Of course";
  if (value == "depends on the cake") return "It depends on the cake";
  if (value == "no cake") return "No cake";
  if (value == "dare-yes") return "Yes, coffee sounds good";
  if (value == "dare-maybe") return "Maybe, ask me properly";
  if (value == "dare-no") return "No, but good try";
  return value;
}
String answerClass(String value) {
  if (value == "yes" || value == "dare-yes") return "greenEntry";
  if (value == "maybe" || value == "dare-maybe") return "yellowEntry";
  if (value == "no" || value == "dare-no") return "redEntry";
  return "neutralEntry";
}
String pathLabel(String value) {
  if (value == "truth") return "Truth";
  if (value == "dare") return "Dare";
  return value;
}
bool isAdmin() {
  return server.arg("k") == ADMIN_KEY;
}
bool hasAnswers() {
  File file = LittleFS.open(LOG_FILE, FILE_READ);
  if (!file) return false;
  bool found = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      found = true;
      break;
    }
  }
  file.close();
  return found;
}
void appendLog(String path, String answer, String q1, String q2) {
  File file = LittleFS.open(LOG_FILE, FILE_APPEND);
  if (!file) {
    Serial.println("[!] Error: Cannot open log file");
    return;
  }
  file.print(millis());
  file.print(",");
  file.print(cleanValue(path));
  file.print(",");
  file.print(cleanValue(answer));
  file.print(",");
  file.print(cleanValue(q1));
  file.print(",");
  file.println(cleanValue(q2));
  file.close();
  Serial.print("[+] Saved answer: ");
  Serial.print(path);
  Serial.print(" / ");
  Serial.println(answer);
}
void handleApp() {
  Serial.print("[+] HTTP request: ");
  Serial.println(server.uri());
  if (WiFi.softAPgetStationNum() > ACTIVE_CLIENT_LIMIT) {
    Serial.println("[!] Too many active clients. Showing busy page.");
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/html; charset=utf-8", busyPage);
    return;
  }
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Content-Language", "en");
  server.send(200, "text/html; charset=utf-8", makeAppPage());
}
void handleLog() {
  appendLog(server.arg("path"), server.arg("answer"), server.arg("q1"), server.arg("q2"));
  server.send(200, "text/plain; charset=utf-8", "OK");
}
void handleAdminLogin() {
  String html = "<!DOCTYPE html><html lang=\"da\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>Admin login</title><style>body{font-family:Arial;margin:18px;background:#fff0f6;color:#3a0a1e}.box{max-width:420px;margin:auto;background:white;padding:22px;border-radius:18px;box-shadow:0 0 18px #ffc2d6;text-align:center}input{font-size:24px;text-align:center;padding:10px;border-radius:10px;border:1px solid #ccc;width:80%;max-width:230px}button{font-size:16px;padding:10px 16px;margin:10px;border:0;border-radius:12px;background:#d63384;color:white}</style></head><body><div class=\"box\"><h1>Admin code</h1><form action=\"/admin\" method=\"get\"><input name=\"k\" type=\"number\" inputmode=\"numeric\" pattern=\"[0-9]*\" autofocus><br><button type=\"submit\">Open</button></form><p><a href=\"/\">Back</a></p></div></body></html>";
  server.send(200, "text/html; charset=utf-8", html);
}
void handleAdmin() {
  if (server.arg("k").length() == 0) {
    handleAdminLogin();
    return;
  }
  if (!isAdmin()) {
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "redirect to app");
    return;
  }
  bool any = false;
  String html = "<!DOCTYPE html><html lang=\"da\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>Date status</title><style>body{font-family:Arial;margin:18px;background:#fff0f6;color:#3a0a1e}.box{max-width:900px;margin:auto;background:white;padding:22px;border-radius:18px;box-shadow:0 0 18px #ffc2d6}h1{text-align:center;color:#d63384;margin:8px 0 6px}.small{text-align:center;color:#8a3a5a}.entry{border-radius:12px;padding:12px;margin:12px 0}.greenEntry{background:#d8f3dc}.yellowEntry{background:#fff3b0}.redEntry{background:#ffd6d6}.neutralEntry{background:#f7f7f7}.q{font-weight:bold;color:#000}.a{margin:4px 0 10px}.buttons{text-align:center}.btn{display:inline-block;font-size:16px;padding:10px 16px;margin:8px 6px 0 6px;border:0;border-radius:12px;background:#c62828;color:white;text-decoration:none}.btn2{background:#777}.disabled{display:inline-block;font-size:16px;padding:10px 16px;margin:8px 6px 0 6px;border-radius:12px;background:#bbb;color:#666;text-decoration:none}</style></head><body><div class=\"box\"><h1>Date status</h1><p class=\"small\">Saved answers are shown here:</p>";
  File file = LittleFS.open(LOG_FILE, FILE_READ);
  if (!file) {
    html += "<p>No answers saved yet.</p>";
  } else {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) continue;
      any = true;
      int p1 = line.indexOf(',');
      int p2 = line.indexOf(',', p1 + 1);
      int p3 = line.indexOf(',', p2 + 1);
      int p4 = line.indexOf(',', p3 + 1);
      String ms = line.substring(0, p1);
      String path = line.substring(p1 + 1, p2);
      String answer = line.substring(p2 + 1, p3);
      String q1 = line.substring(p3 + 1, p4);
      String q2 = line.substring(p4 + 1);
      html += "<div class=\"entry " + answerClass(answer) + "\">";
      html += "<p><b>Time:</b> " + String(ms.toInt() / 1000) + " seconds after start</p>";
      html += "<p><b>Choice:</b> " + htmlEscape(pathLabel(path)) + "</p>";
      if (path == "truth") {
        html += "<p class=\"q\">Question 1: Would coffee with a charming person<br>be a good idea?</p>";
        html += "<p class=\"a\">Answer: " + htmlEscape(answerLabel(q1)) + "</p>";
        html += "<p class=\"q\">Question 2: Would cake make the situation better?</p>";
        html += "<p class=\"a\">Answer: " + htmlEscape(answerLabel(q2)) + "</p>";
        html += "<p class=\"q\">Last question: Would you go on a date with me?</p>";
        html += "<p class=\"a\">Answer: " + htmlEscape(answerLabel(answer)) + "</p>";
      } else if (path == "dare") {
        html += "<p class=\"q\">Dare: Smile, look at me, and choose one.</p>";
        html += "<p class=\"a\">Answer: " + htmlEscape(answerLabel(answer)) + "</p>";
      } else {
        html += "<p class=\"a\">Answer: " + htmlEscape(answerLabel(answer)) + "</p>";
      }
      html += "</div>";
    }
    file.close();
    if (!any) html += "<p>No answers saved yet.</p>";
  }
  html += "<p class=\"buttons\">";
  if (any) {
    html += "<a class=\"btn\" href=\"/clear?k=" ADMIN_KEY "\">Delete answers</a>";
  } else {
    html += "<span class=\"disabled\">Delete answers</span>";
  }
  html += "<a class=\"btn btn2\" href=\"/\">Back</a></p></div></body></html>";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html; charset=utf-8", html);
}
void handleAnswers() {
  handleAdmin();
}
void handleClear() {
  if (!isAdmin()) {
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "redirect to app");
    return;
  }
  if (hasAnswers()) {
    LittleFS.remove(LOG_FILE);
    Serial.println("[+] Answer log cleared");
  }
  server.send(200, "text/html; charset=utf-8", "<!DOCTYPE html><html lang=\"da\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>Deleted</title><style>body{font-family:Arial;margin:18px;background:#fff0f6;color:#3a0a1e}.box{max-width:520px;margin:auto;background:white;padding:22px;border-radius:18px;box-shadow:0 0 18px #ffc2d6;text-align:center}a{display:inline-block;font-size:16px;padding:10px 16px;margin:8px;border-radius:12px;background:#d63384;color:white;text-decoration:none}</style></head><body><div class=\"box\"><h1>Answers deleted</h1><p><a href=\"/admin\">Back til admin</a></p><p><a href=\"/\">Home</a></p></div></body></html>");
}
void handleNotFound() {
  Serial.print("[+] HTTP redirect: ");
  Serial.println(server.uri());
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "redirect to app");
}
void configureRfAntenna() {
  if (external_antenna) {
    pinMode(RF_SWITCH_ENABLE_PIN, OUTPUT);
    digitalWrite(RF_SWITCH_ENABLE_PIN, LOW);
    delay(100);
    pinMode(RF_ANTENNA_SELECT_PIN, OUTPUT);
    digitalWrite(RF_ANTENNA_SELECT_PIN, HIGH);
    Serial.println("[+] RF antenna: external U.FL antenna enabled");
  } else {
    pinMode(RF_SWITCH_ENABLE_PIN, OUTPUT);
    digitalWrite(RF_SWITCH_ENABLE_PIN, LOW);
    delay(100);
    pinMode(RF_ANTENNA_SELECT_PIN, OUTPUT);
    digitalWrite(RF_ANTENNA_SELECT_PIN, LOW);
    Serial.println("[+] RF antenna: built-in ceramic antenna enabled");
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  unsigned long serial_start = millis();
  while (!Serial && millis() - serial_start < 3000) {
   delay(10);
  }
  delay(500);
  Serial.println();
  Serial.println("[+] ESP32 Truth or Dare starting");
  configureRfAntenna();
  Serial.print("[+] Serial Monitor baud rate: ");
  Serial.println(SERIAL_BAUD);
  Serial.print("[+] Access Point SSID: ");
  Serial.println(AP_SSID);
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.softAP(AP_SSID, "", AP_CHANNEL, 0, AP_MAX_CLIENTS);
  Serial.print("[+] Access Point channel: ");
  Serial.println(AP_CHANNEL);
  Serial.print("[+] Access Point max clients: ");
  Serial.println(AP_MAX_CLIENTS);
  Serial.print("[+] Active client page limit: ");
  Serial.println(ACTIVE_CLIENT_LIMIT);
  Serial.print("[+] Access Point IP address: ");
  Serial.println(WiFi.softAPIP());
  WiFi.AP.enableDhcpCaptivePortal();
  if (dnsServer.start()) {
    Serial.println("[+] Started DNS server in captive portal mode");
  } else {
    Serial.println("[!] Error: Cannot start DNS server");
  }
  if (LittleFS.begin(true)) {
    Serial.println("[+] LittleFS mounted");
  } else {
    Serial.println("[!] Error: LittleFS mount failed");
  }
  server.on("/", handleApp);
  server.on("/portal", handleApp);
  server.on("/generate_204", handleApp);
  server.on("/gen_204", handleApp);
  server.on("/hotspot-detect.html", handleApp);
  server.on("/ncsi.txt", handleApp);
  server.on("/log", handleLog);
  server.on("/admin", handleAdmin);
  server.on("/answers", handleAnswers);
  server.on("/clear", handleClear);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("[+] HTTP server started on port 80");
  Serial.print("[+] Connect to Wi-Fi: ");
  Serial.println(AP_SSID);
  Serial.println("[+] Open http://192.168.4.1/");
  Serial.println("[+] Admin: http://192.168.4.1/admin");
  Serial.println("[+] Setup complete");
}
void loop() {
  server.handleClient();
  delay(1);
}
