#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define oneWireBus 0

const char* ssid     = "CT_101";
const char* password = "AusyxSolucoes";

WiFiEventHandler handleStationModeConnected;
WiFiEventHandler handleStationModeDisconnected;
WiFiEventHandler handleStationModeGotIP;

WebSocketsClient wsClient;
void handleWSEvent(WStype_t type, uint8_t * payload, size_t length);

void wifi_connect(void);
void ws_connect(void);
void ws_disconnect(void);

bool wifiConnected = false;
bool wsConnected = false;

float temperatura = 0.0;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  handleStationModeConnected = WiFi.onStationModeConnected(&onWiFiConnected);
  handleStationModeDisconnected = WiFi.onStationModeDisconnected(&onWiFiDisconnected);
  handleStationModeGotIP = WiFi.onStationModeGotIP(&onWiFiGotIP);

  wsClient.onEvent(webSocketEvent);
  wsClient.enableHeartbeat(10000, 3000, 3);
  
  wifi_connect();
}

//loopCounter será usado para toda vez que o loop rodar 15 vezes, a temperatura seja enviada
// O que da mais ou menos 10 segundos
int loopCounter = 0;

void loop() {
  if (wifiConnected) {
    if (wsConnected) {
      wsClient.loop();

      if (loopCounter >= 15) {
        sensors.requestTemperatures();
        temperatura = sensors.getTempCByIndex(0);

        temperatura = (temperatura * 100) / 100;

        if (temperatura < 1) temperatura = 0;
        if (temperatura > 99) temperatura = 99;

        Serial.println(" ");
        Serial.print("Temperatura: ");
        Serial.println(temperatura);

        wsClient.sendTXT(temperatura);
        loopCounter = 0;
      }
    }
  }

  loopCounter++;
  delay(150);
}

void wifi_connect() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
}

void ws_connect() {
  Serial.println("Attempting to connect to WebSocket");
  wsClient.begin("192.168.4.1", 80, "ws");
}

void ws_disconnect() {
  wsClient.disconnect();
}

void onWiFiConnected(const WiFiEventStationModeConnected &evt) {
  Serial.println("WiFi connected!");
}

void onWiFiDisconnected(const WiFiEventStationModeDisconnected &evt) {
  Serial.println("WiFi disconnected!");
  wifiConnected = false;
  wsConnected = false;

  wifi_connect();
  ws_disconnect();
}

void onWiFiGotIP(const WiFiEventStationModeGotIP &evt) {
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //connect web socket
  ws_connect();
  wifiConnected = true;
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket Disconnected!");
      wsConnected = false;
      ws_connect();
      break;
    case WStype_CONNECTED:
      Serial.printf("WebSocket Connected to url: %s\n", payload);
      wsConnected = true;
      break;
  }
}
