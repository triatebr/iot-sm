/*
  Smart City IoT - Projeto 
  Hardware Utilizado: Nodemcu v1.0, DHT11, MQ-135
  Autores: Yhan  Silva | Lucas Barros - data: 27/10/2018 
  Referências: https://youtu.be/fS0GeaOkNRw
*/

// -- Bibliotecas auxiliares --

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <MQ135.h>
#include "DHT.h"

// -- Hardware --

#define MQ135PIN D1
#define DHTPIN D2
#define DHTTYPE DHT11

// -- Instância objeto --

DHT dht(DHTPIN, DHTTYPE);
MQ135 co2Sensor(MQ135PIN);
WiFiClientSecure client;

// -- Variáveis e constantes --

const char *ssid = "Insira seu SSID";
const char *password = "Insira a senha";
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
String googleSheetsID = "AKfycbxEAGB6P02cki1z8W-W6Fz7X_EynNWS5mCV19SUem0R4yXuO5A";
long previusMillis;
const long interval = 30000;
int temperature, humidity, co2Level;
float lastH, lastT, lastC;

// -- Escopo funções auxiliares --
void connectToWiFi();
void readSensor();
void sendDataToGoogleSheets(int temp, int hum, int cO2);

// -- Setup -- 

void setup() {
  dht.begin();
  Serial.begin(115200);
  connectToWiFi();
}

// -- Loop leitura de sensor e envio de dados a cada 30s --

void loop() {
  if (millis() - previusMillis >= interval) {
    readSensor();
    previusMillis = millis();
  }
}

// -- Funções auxiliares --

// -- Conecta a rede WiFi fornecida e exibe endereço IP --

void connectToWiFi() {
  Serial.println("Conectando a rede: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  WiFi.mode(WIFI_STA);
  Serial.println("");
  Serial.println("Conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  delay(1000);
}

/* Realiza leitura dos sensores, verifica se a leitura é válida, caso não seja utiliza o último valor 
 * lido. Estes valores são convertidos para int e são parâmetros da função sendDataToGoogleSheets
 */

void readSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float c = co2Sensor.getPPM();
  if (isnan(h) || isnan(t) || isnan(c)) {
    Serial.print("Temperatura: ");
    Serial.print(lastH);
    Serial.print(" Umidade: ");
    Serial.print(lastT);
    Serial.print( " Nível C02: ");
    Serial.println(lastC);
    temperature = (int) lastT;
    humidity = (int) lastH;
    co2Level = (int) lastC;  
    sendDataToGoogleSheets(lastT, lastH, lastC);
  } 
  
  else {
    Serial.print("Temperatura: ");
    Serial.print(lastH);
    Serial.print(" Umidade: ");
    Serial.print(lastT);
    Serial.print( "Nível C02: ");
    Serial.println(lastC);
    temperature = (int) t;
    humidity = (int) h;
    co2Level = (int) c;
    sendDataToGoogleSheets(t, h, c);
    lastH = h;
    lastT = t;
    lastC = c;
  }
}

/* Recebe como parâmetros os valores inteiros de temperatura e umidade lidos pelo DHT
 * Conecta ao host scripts google, verifica o certificado fingerprint. 
 * Converte os valores de temperatura e umidade para string, envia estes dados via GET
 * Monta a requisição e resposta e plota os dados na serial
 */

void sendDataToGoogleSheets(int temp, int hum, int co2) {
  Serial.print("Conectando à: ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("Falha na conexão ao Google Sheets");
    return;
  }
  if (client.verify(fingerprint, host)) Serial.println("Certificado OK");
  else Serial.println("Certificado NOK, verificar");

  String stringTemp = String(temp, DEC);
  String stringHum = String(hum, DEC);
  String stringCo2 = String(co2, DEC);
  String url = "/macros/s/" + googleSheetsID + "/exec?TEMPERATURA=" + stringTemp + "&UMIDADE=" + stringHum + "&CO2=" + stringCo2;
  Serial.print("Requisitando URL ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("Request enviada");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers Received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("Sucesso");
  } else {
    Serial.println("Falha no envio!");
  }
  Serial.println("Resposta:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("Fechando conexão!");
}