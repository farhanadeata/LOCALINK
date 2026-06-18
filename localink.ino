#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <DNSServer.h>

#define AP_SSID "LOCALINK"
#define AP_PASS "Frnyx2708"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;

String messagesJson = "[]";
uint16_t onlineUsers = 0;

// =========================
// LOAD CHAT
// =========================

void loadMessages() {

  if (!LittleFS.exists("/messages.json")) {

    File file =
      LittleFS.open(
      "/messages.json",
      "w");

    file.print("[]");

    file.close();
  }

  File file =
    LittleFS.open(
    "/messages.json",
    "r");

  if (!file) {

    messagesJson = "[]";
    return;
  }

  messagesJson =
    file.readString();

  file.close();
}

// =========================
// SAVE CHAT
// =========================

void saveMessages() {

  File file =
    LittleFS.open(
    "/messages.json",
    "w");

  if (!file)
    return;

  file.print(messagesJson);

  file.close();
}

// =========================
// SEND HISTORY
// =========================

void sendHistory(
    AsyncWebSocketClient *client) {

  String json =

    "{\"type\":\"history\","
    "\"messages\":"
    + messagesJson +
    "}";

  client->text(json);
}

// =========================
// ONLINE USER
// =========================

void broadcastOnline() {

  String json =

    "{\"type\":\"online\","
    "\"count\":"
    + String(onlineUsers)
    + "}";

  ws.textAll(json);
}

// =========================
// NEW MESSAGE
// =========================

void addMessage(
    String username,
    String role,
    String text) {

  String time =
    String(millis() / 1000);

  String item =

    "{"
    "\"username\":\"" + username + "\","
    "\"role\":\"" + role + "\","
    "\"message\":\"" + text + "\","
    "\"time\":\"" + time + "\""
    "}";

  if (messagesJson == "[]") {

    messagesJson =
      "[" + item + "]";
  }
  else {

    messagesJson.remove(
      messagesJson.length() - 1);

    messagesJson +=
      "," + item + "]";
  }

  saveMessages();

  String payload =

    "{"
    "\"type\":\"message\","
    "\"username\":\"" + username + "\","
    "\"role\":\"" + role + "\","
    "\"message\":\"" + text + "\","
    "\"time\":\"" + time + "\""
    "}";

  ws.textAll(payload);
}

// =========================
// CLEAR CHAT
// =========================

void clearChat() {

  messagesJson = "[]";

  saveMessages();

  ws.textAll(
    "{\"type\":\"clear\"}");
}

// =========================
// SOCKET EVENT
// =========================

void handleSocketMessage(
    void *arg,
    uint8_t *data,
    size_t len,
    AsyncWebSocketClient *client) {

  AwsFrameInfo *info =
    (AwsFrameInfo*)arg;

  if (!info->final)
    return;

  String payload = "";

  for (
    size_t i = 0;
    i < len;
    i++) {

    payload +=
      (char)data[i];
  }

  Serial.println(
    "RX: " + payload);

  if (payload.startsWith(
      "SEND|")) {

    payload.remove(0,5);

    int p1 =
      payload.indexOf("|");

    int p2 =
      payload.indexOf(
      "|",
      p1 + 1);

    String username =
      payload.substring(
      0,
      p1);

    String role =
      payload.substring(
      p1 + 1,
      p2);

    String message =
      payload.substring(
      p2 + 1);

    addMessage(
      username,
      role,
      message);
  }

  else if (
      payload ==
      "CLEAR") {

    clearChat();
  }
}

void onWsEvent(

    AsyncWebSocket *server,

    AsyncWebSocketClient *client,

    AwsEventType type,

    void *arg,

    uint8_t *data,

    size_t len) {

  switch(type) {

    case WS_EVT_CONNECT:

      onlineUsers++;

      Serial.printf(
      "Client %u connected\n",
      client->id());

      sendHistory(client);

      broadcastOnline();

      break;

    case WS_EVT_DISCONNECT:

      if (onlineUsers > 0)
        onlineUsers--;

      Serial.printf(
      "Client %u disconnected\n",
      client->id());

      broadcastOnline();

      break;

    case WS_EVT_DATA:

      handleSocketMessage(
        arg,
        data,
        len,
        client);

      break;

    default:
      break;
  }
}

// =========================
// SETUP
// =========================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("LOCALINK V2");

  WiFi.mode(WIFI_AP);

  WiFi.softAP(
    AP_SSID,
    AP_PASS);

dnsServer.start(
    53,
    "*",
    WiFi.softAPIP()
  );

  Serial.print(
      "IP : ");

  Serial.println(
      WiFi.softAPIP());

  if (!LittleFS.begin()) {

    Serial.println(
      "LittleFS ERROR");

    return;
  }

  loadMessages();

  ws.onEvent(
      onWsEvent);

  server.addHandler(
      &ws);

  server.serveStatic(
      "/",
      LittleFS,
      "/")
      .setDefaultFile(
      "index.html");

    server.on("/generate_204", HTTP_GET,
  [](AsyncWebServerRequest *request){
      request->redirect("/");
  });

  server.on("/hotspot-detect.html", HTTP_GET,
  [](AsyncWebServerRequest *request){
      request->redirect("/");
  });

  server.on("/ncsi.txt", HTTP_GET,
  [](AsyncWebServerRequest *request){
      request->redirect("/");
  });

  server.on("/connecttest.txt", HTTP_GET,
  [](AsyncWebServerRequest *request){
      request->redirect("/");
  });

  server.onNotFound(
  [](AsyncWebServerRequest *request){
      request->redirect("/");
  });

  server.begin();
  

  Serial.println(
      "SERVER READY");
}

// =========================
// LOOP
// =========================

void loop() {

  dnsServer.processNextRequest();

  ws.cleanupClients();
}