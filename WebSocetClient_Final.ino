#include <ESP8266WiFi.h>
#include <ArduinoWebsockets.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define oneWireBus  0

#define STASSID "PaliniAlves"
#define STAPSK  "12345678"

const char* ssid     = STASSID;
const char* password = STAPSK;

using namespace websockets;
WebsocketsClient client;

bool ws_connected = false;

void wifi_connect() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && WiFi.status() == 7) {
    delay(500);
    Serial.print("Status: ");
    Serial.println(WiFi.status());
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (WiFi.status() == WL_CONNECTED) {
    client.connect("ws://192.168.4.1:80/ws");
  }
}

void onEventsCallback(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("Web socket connected!");
    ws_connected = true;
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Web socket disconnected!");
    ws_connected = false;
    client.connect("ws://192.168.4.1:80/ws");
  }
}

float temperatura = 0.0;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);

  client.onEvent(onEventsCallback);

  // We start by connecting to a WiFi network
  wifi_connect();

}

//loopCounter será usado para toda vez que o loop rodar 15 vezes, a temperatura seja enviada
// O que da mais ou menos 10 segundos

int loopCounter = 0;

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    client.poll();
    if (ws_connected && loopCounter >= 15) {
      String data;

      sensors.requestTemperatures();
      temperatura = sensors.getTempCByIndex(0);

      temperatura = (temperatura * 100) / 100;

      if (temperatura < 1) temperatura = 0;
      if (temperatura > 99) temperatura = 99;

      Serial.println(" ");
      Serial.print("Temperatura: ");
      Serial.println(temperatura);

      client.send(temperatura);
      loopCounter = 0;

    }
  } else {
    ws_connected = false;
    wifi_connect();
  }

  loopCounter++;
  delay(500);
}
