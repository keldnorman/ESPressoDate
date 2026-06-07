/*
Dette er Arduino-programmet "uno-date", som skaffer dig en date.

Det gør det ved at oprette et trådløst access point med navnet "Truth or Dare".
Når en person forbinder til det, viser det nogle spørgsmål for at afgøre, om personen vil på date med dig.

Koden er lavet for at gøre programmering sjovt og give det et formål, hvor hardware og software bruges til at lære teknologier.

Sådan installerer/bygger du koden:

Hardware: ESP32 Seeed Studio XIAO ESP32C6 WiFi 6 + Bluetooth-kompatibel BLE 5, med support for Zigbee og Matter, trådløst udviklingsboard.

https://www.aliexpress.com/item/1005006946443492.html

Denne kode er lavet til ESP32-hardwaren kaldet Seeed Studio XIAO ESP32C6.

https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/

For at kompilere den skal du bare starte Arduino IDE. Jeg bruger version 2.3.9. Gå derefter til:

File -> Preferences -> Additional Boards Manager URL

Og tilføj dette:

https://espressif.github.io/arduino-esp32/package_esp32_index.json

Tools -> Boards -> Boards Manager

Eller tryk CTRL + SHIFT + B, hvis det virker.

Søg derefter efter "esp32 by Espressif Systems" og installer den.

Jeg havde version 3.3.10 installeret, da jeg skrev denne kode.

(C)opyleft Keld Norman, 2025

*/
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <LittleFS.h>
#define SERIAL_BAUD 115200
#define AP_SSID "Sandhed eller konsekvens? 😏"
#define AP_CHANNEL 6
#define AP_MAX_CLIENTS 4
#define ACTIVE_CLIENT_LIMIT 2
#define LOG_FILE "/answers.csv"
#define ADMIN_KEY "1337"
static const char date_name[] = "Dit Navn";
static const char date_phone[] = "+45 xxxxxxxx";
static const char date_instagram[] = "@din.instagram";
static const char date_mail[] = "din@mail.com";
static const bool external_antenna = true;
static const uint8_t RF_SWITCH_ENABLE_PIN = 3;
static const uint8_t RF_ANTENNA_SELECT_PIN = 14;
DNSServer dnsServer;
WebServer server(80);
static const char busyPage[] = R"HTML(
<!DOCTYPE html>
<html lang="da">
<head>
<title>Optaget</title>
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
<div class="box"><h1>Optaget</h1><p class="small">Der er lidt for mange på lige nu.</p><p>Prøv igen om lidt ☕</p></div>
</body>
</html>
)HTML";
static const char appPage[] = R"HTML(
<!DOCTYPE html>
<html lang="da">
<head>
<title>Sandhed eller konsekvens</title>
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
const dateName='__DATE_NAME__';
const datePhone='__DATE_PHONE__';
const dateInstagram='__DATE_INSTAGRAM__';
const dateMail='__DATE_MAIL__';
const datePhoneTel=datePhone.split(' ').join('');
const smsText='Hej '+dateName+' ('+datePhone+')\n\nJeg vil gerne på en date med dig 💕\n\nHer er mit nummer:\n\nKh.';
let step=0;
let answers={};
function logAnswer(path,answer){
fetch('/log?path='+encodeURIComponent(path)+'&answer='+encodeURIComponent(answer)+'&q1='+(answers.q1||'')+'&q2='+(answers.q2||''),{cache:'no-store'}).catch(()=>{});
}
function adminPrompt(){
below.innerHTML='';
app.innerHTML='<h1>Admin kode</h1><p class="small">Indtast kode</p><input id="adminCode" class="adminInput" type="number" inputmode="numeric" pattern="[0-9]*" autofocus><button onclick="checkAdminCode()">Åbn</button><button class="back" onclick="home()">Tilbage</button>';
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
if(answers.q1==='ja'||answers.q1==='måske'){return 'kaffe ☕';}
return 'te 🍵';
}
function snackText(){
if(answers.q2==='selvfølgelig'||answers.q2==='afhænger af kagen'){return 'kage 🍰';}
return 'noget sødt uden kage';
}
function home(){
step=0;
answers={};
below.innerHTML='';
app.innerHTML='<h1>Sandhed eller konsekvens? <span onclick="adminPrompt()" style="cursor:pointer">💘</span></h1><p class="small">Vælg med omhu:</p><button class="btn" onclick="truth()">Sandhed</button><button class="btn btn2" onclick="dare()">Konsekvens</button>';
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
html='<h1>Sandhed</h1><p class="small">Spørgsmål 1 af 3</p><p class="q">Ville kaffe med en charmerende person<br>være en god idé? ☕</p><button onclick="answers.q1=\'ja\';step=2;truthStep()">Ja 💕</button><button onclick="answers.q1=\'måske\';step=2;truthStep()">Måske</button><button onclick="answers.q1=\'nej\';step=2;truthStep()">Nej</button><p><button class="back" onclick="home()">Tilbage</button></p>';
}
if(step===2){
html='<h1>Sandhed</h1><p class="small">Spørgsmål 2 af 3</p><p class="q">Ville kage gøre situationen bedre? 🍰</p><button onclick="answers.q2=\'selvfølgelig\';step=3;truthStep()">Selvfølgelig 💕</button><button onclick="answers.q2=\'afhænger af kagen\';step=3;truthStep()">Det afhænger af kagen</button><button onclick="answers.q2=\'ingen kage\';step=3;truthStep()">Ingen kage</button><p><button class="back" onclick="step=1;truthStep()">Tilbage</button></p>';
}
if(step===3){
html='<h1>Sandhed</h1><p class="small">Sidste spørgsmål</p><p class="q">Vil du på en date med mig? 💘</p><button onclick="logAnswer(\'truth\',\'ja\');resultYes()">Ja 💕</button><button onclick="logAnswer(\'truth\',\'måske\');resultMaybe()">Måske, spørg mig pænt igen ☕</button><button onclick="logAnswer(\'truth\',\'nej\');resultNo()">Nej, men det her var sødt</button><p><button class="back" onclick="step=2;truthStep()">Tilbage</button></p>';
}
app.innerHTML=html;
}
function dare(){
below.innerHTML='';
app.innerHTML='<h1 class="blackTitle">Konsekvens ☕</h1><p class="small">Din konsekvens er enkel.</p><p>Smil, kig på mig, og vælg én:</p><button class="btn wide btn2" onclick="answers.q1=\'ja\';answers.q2=\'selvfølgelig\';logAnswer(\'dare\',\'dare-ja\');resultYes()">Ja, kaffe lyder godt ☕</button><button class="btn wide btn2" onclick="answers.q1=\'ja\';answers.q2=\'selvfølgelig\';logAnswer(\'dare\',\'dare-måske\');resultMaybe()">Måske, spørg mig ordentligt 💌</button><button class="btn wide btn2" onclick="logAnswer(\'dare\',\'dare-nej\');resultNo()">Nej, men godt forsøg</button><p><button class="back" onclick="home()">Tilbage</button></p>';
}
function contact(){
below.innerHTML='';
app.innerHTML='<h1 class="blackTitle">Kontaktinfo</h1><br><p class="contactLine"><b>Navn:</b> '+dateName+'<br><b>Instagram:</b> '+dateInstagram+'</p><p class="contactLine"><b>Mail:</b> <a class="contactLink" href="mailto:'+dateMail+'">'+dateMail+'</a></p><p class="contactLine"><b>Telefon:</b> <a class="contactLink" href="tel:'+datePhoneTel+'">'+datePhone+'</a></p><br><textarea id="smsText" class="smsBox" readonly>'+smsText+'</textarea><p style="margin:4px 0"><button class="green" onclick="copySms()">Kopiér tekst</button></p><p style="color:#000;margin:4px 0">Jeg håber, du skriver 💌</p><p style="margin:4px 0"><button class="back" onclick="home()">Tilbage</button></p>';
}
function resultYes(){
app.innerHTML='<h1>Accepteret 💕</h1><p>Perfekt. Date låst op.</p><p class="plan">Forslag: '+drinkText()+', '+snackText()+'<br>og mistænkeligt god samtale.</p>';
below.innerHTML='<p class="hint">Tryk på vis kontaktinfo knappen!</p><div class="bigArrow">&#8595;</div><button class="back green" onclick="contact()">Vis kontaktinfo</button><button class="back" onclick="home()">Start igen</button>';
}
function resultMaybe(){
below.innerHTML='';
app.innerHTML='<h1>Måske ☕</h1><p>Accepteret. Jeg spørger ordentligt.</p><p>Vil du med ud og drikke kaffe med mig? 💌</p><p><button class="back" onclick="home()">Start igen</button></p>';
}
function resultNo(){
below.innerHTML='';
app.innerHTML='<h1>Godt forsøg</h1><p>Fair svar. Ingen sure miner.</p><p><button class="back" onclick="home()">Start igen</button></p>';
}
home();
</script>
</body>
</html>
)HTML";
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
  if (value == "ja") return "Ja";
  if (value == "måske") return "Måske";
  if (value == "nej") return "Nej";
  if (value == "selvfølgelig") return "Selvfølgelig";
  if (value == "afhænger af kagen") return "Det afhænger af kagen";
  if (value == "ingen kage") return "Ingen kage";
  if (value == "dare-ja") return "Ja, kaffe lyder godt";
  if (value == "dare-måske") return "Måske, spørg mig ordentligt";
  if (value == "dare-nej") return "Nej, men godt forsøg";
  return value;
}
String answerClass(String value) {
  if (value == "ja" || value == "dare-ja") return "greenEntry";
  if (value == "måske" || value == "dare-måske") return "yellowEntry";
  if (value == "nej" || value == "dare-nej") return "redEntry";
  return "neutralEntry";
}
String pathLabel(String value) {
  if (value == "truth") return "Sandhed";
  if (value == "dare") return "Konsekvens";
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
  String page = appPage;
  page.replace("__DATE_NAME__", date_name);
  page.replace("__DATE_PHONE__", date_phone);
  page.replace("__DATE_INSTAGRAM__", date_instagram);
  page.replace("__DATE_MAIL__", date_mail);
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Content-Language", "da");
  server.send(200, "text/html; charset=utf-8", page);
}
void handleLog() {
  appendLog(server.arg("path"), server.arg("answer"), server.arg("q1"), server.arg("q2"));
  server.send(200, "text/plain; charset=utf-8", "OK");
}
void handleAdminLogin() {
  String html = "<!DOCTYPE html><html lang=\"da\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>Admin login</title><style>body{font-family:Arial;margin:18px;background:#fff0f6;color:#3a0a1e}.box{max-width:420px;margin:auto;background:white;padding:22px;border-radius:18px;box-shadow:0 0 18px #ffc2d6;text-align:center}input{font-size:24px;text-align:center;padding:10px;border-radius:10px;border:1px solid #ccc;width:80%;max-width:230px}button{font-size:16px;padding:10px 16px;margin:10px;border:0;border-radius:12px;background:#d63384;color:white}</style></head><body><div class=\"box\"><h1>Admin kode</h1><form action=\"/admin\" method=\"get\"><input name=\"k\" type=\"number\" inputmode=\"numeric\" pattern=\"[0-9]*\" autofocus><br><button type=\"submit\">Åbn</button></form><p><a href=\"/\">Tilbage</a></p></div></body></html>";
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
  String html = "<!DOCTYPE html><html lang=\"da\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>Date status</title><style>body{font-family:Arial;margin:18px;background:#fff0f6;color:#3a0a1e}.box{max-width:900px;margin:auto;background:white;padding:22px;border-radius:18px;box-shadow:0 0 18px #ffc2d6}h1{text-align:center;color:#d63384;margin:8px 0 6px}.small{text-align:center;color:#8a3a5a}.entry{border-radius:12px;padding:12px;margin:12px 0}.greenEntry{background:#d8f3dc}.yellowEntry{background:#fff3b0}.redEntry{background:#ffd6d6}.neutralEntry{background:#f7f7f7}.q{font-weight:bold;color:#000}.a{margin:4px 0 10px}.buttons{text-align:center}.btn{display:inline-block;font-size:16px;padding:10px 16px;margin:8px 6px 0 6px;border:0;border-radius:12px;background:#c62828;color:white;text-decoration:none}.btn2{background:#777}.disabled{display:inline-block;font-size:16px;padding:10px 16px;margin:8px 6px 0 6px;border-radius:12px;background:#bbb;color:#666;text-decoration:none}</style></head><body><div class=\"box\"><h1>Date status</h1><p class=\"small\">Gemte svar vises her med det samme.</p>";
  File file = LittleFS.open(LOG_FILE, FILE_READ);
  if (!file) {
    html += "<p>Ingen svar gemt endnu.</p>";
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
      html += "<p><b>Tid:</b> " + String(ms.toInt() / 1000) + " sekunder efter start</p>";
      html += "<p><b>Valg:</b> " + htmlEscape(pathLabel(path)) + "</p>";
      if (path == "truth") {
        html += "<p class=\"q\">Spørgsmål 1: Ville kaffe med en charmerende person<br>være en god idé?</p>";
        html += "<p class=\"a\">Svar: " + htmlEscape(answerLabel(q1)) + "</p>";
        html += "<p class=\"q\">Spørgsmål 2: Ville kage gøre situationen bedre?</p>";
        html += "<p class=\"a\">Svar: " + htmlEscape(answerLabel(q2)) + "</p>";
        html += "<p class=\"q\">Sidste spørgsmål: Vil du på en date med mig?</p>";
        html += "<p class=\"a\">Svar: " + htmlEscape(answerLabel(answer)) + "</p>";
      } else if (path == "dare") {
        html += "<p class=\"q\">Konsekvens: Smil, kig på mig, og vælg én.</p>";
        html += "<p class=\"a\">Svar: " + htmlEscape(answerLabel(answer)) + "</p>";
      } else {
        html += "<p class=\"a\">Svar: " + htmlEscape(answerLabel(answer)) + "</p>";
      }
      html += "</div>";
    }
    file.close();
    if (!any) html += "<p>Ingen svar gemt endnu.</p>";
  }
  html += "<p class=\"buttons\">";
  if (any) {
    html += "<a class=\"btn\" href=\"/clear?k=" ADMIN_KEY "\">Slet svar</a>";
  } else {
    html += "<span class=\"disabled\">Slet svar</span>";
  }
  html += "<a class=\"btn btn2\" href=\"/\">Tilbage</a></p></div></body></html>";
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
  server.send(200, "text/html; charset=utf-8", "<!DOCTYPE html><html lang=\"da\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>Slettet</title><style>body{font-family:Arial;margin:18px;background:#fff0f6;color:#3a0a1e}.box{max-width:520px;margin:auto;background:white;padding:22px;border-radius:18px;box-shadow:0 0 18px #ffc2d6;text-align:center}a{display:inline-block;font-size:16px;padding:10px 16px;margin:8px;border-radius:12px;background:#d63384;color:white;text-decoration:none}</style></head><body><div class=\"box\"><h1>Svar slettet</h1><p><a href=\"/admin\">Tilbage til admin</a></p><p><a href=\"/\">Forside</a></p></div></body></html>");
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
