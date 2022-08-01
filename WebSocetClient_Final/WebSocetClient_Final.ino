#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>

#include <OneWire.h>
#include <DallasTemperature.h>

/*
 * MUDAR AQUI OS VALORES DA CONFIGURACAO DE CONEXAO!!
 */
#define SERVER_SSID "CT_22"
#define SERVER_PASSWORD "AusyxSolucoes"
/*
 * ---------------------------------------------------
 */

#define oneWireBus 0

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  WiFiMulti.addAP(SERVER_SSID, SERVER_PASSWORD);

  Serial.print("Tentando primeira conexão, ficará aqui eternamente até conseguir se conectar a: ");
  Serial.println(SERVER_SSID);
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println("Conectado!");

  webSocket.begin("192.168.4.1", 80, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 3000, 2);
}


//loopCounter será usado para toda vez que o loop rodar 15 vezes, a temperatura seja enviada
// O que da mais ou menos 10 segundos
int loopCounter = 0;
int temperatura = 0;
char tempStr[3];

void loop() {
  webSocket.loop();

  if (webSocket.isConnected()) {
    if (loopCounter >= 50) {
      sensors.requestTemperatures();
      temperatura = sensors.getTempCByIndex(0);

      if (temperatura < 1) temperatura = 0;
      if (temperatura > 150) temperatura = 150;

      itoa(temperatura, tempStr, 10);

      //        Serial.println(" ");
      //        Serial.print("Temperatura: ");
      //        Serial.println(tempStr);

      webSocket.sendTXT(tempStr);
      loopCounter = 0;
    }
  }

  loopCounter++;
  delay(100);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      break;
  }
}
