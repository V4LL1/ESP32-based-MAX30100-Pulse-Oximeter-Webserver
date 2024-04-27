#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS 1000

float BPM, SpO2;
const char* ssid = "xxxx-xxxx-xxxx";  // Enter SSID here
const char* password = "xxx-xxx-xxx";  //Enter Password here

PulseOximeter pox;
uint32_t tsLastReport = 0;
WebServer server(80);

void onBeatDetected() {
  Serial.println("Beat Detected!");
}

void setup() {
  Serial.begin(115200);
  pinMode(19, OUTPUT);
  delay(100);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  
  Serial.println("HTTP server started");
  
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
    pox.setOnBeatDetectedCallback(onBeatDetected);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
}

void loop() {
  server.handleClient();
  pox.update();
  BPM = pox.getHeartRate();
  SpO2 = pox.getSpO2();
  
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("BPM: ");
    Serial.println(BPM);
    Serial.print("SpO2: ");
    Serial.print(SpO2);
    Serial.println("%");
    Serial.println("*********************************");
    Serial.println();
    tsLastReport = millis();
  }
}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(BPM, SpO2));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float BPM, float SpO2) {
  String ptr = "<!DOCTYPE html>";
  ptr += "<html lang='pt-br'>";
  ptr += "<head>";
  ptr += "<meta charset='UTF-8'>";
  ptr += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr += "<title>Oxímetro de Pulso - Servidor Web ESP32</title>";
  ptr += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.4/css/all.min.css'>";
  ptr += "<style>";
  ptr += "body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f8f9fa; display: flex; justify-content: center; align-items: center; height: 100vh; }";
  ptr += ".container { background-color: #fff; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); padding: 40px; text-align: center; }";
  ptr += "h1 { color: #343a40; margin-bottom: 20px; }";
  ptr += ".reading { margin-bottom: 30px; }";
  ptr += ".reading h2 { font-size: 24px; color: #495057; margin-bottom: 10px; }";
  ptr += ".reading p { font-size: 36px; font-weight: bold; color: #007bff; margin: 0; }";
  ptr += ".icon { font-size: 48px; margin-bottom: 10px; color: #007bff; }";
  ptr += "</style>";
  ptr += "</head>";
  ptr += "<body>";
  ptr += "<div class='container'>";
  ptr += "<h1>Leituras do Oxímetro de Pulso</h1>";
  ptr += "<div class='reading'>";
  ptr += "<i class='fas fa-heartbeat icon'></i>";
  ptr += "<h2>Frequência Cardíaca</h2>";
  ptr += "<p>" + String(BPM) + " BPM</p>";
  ptr += "</div>";
  ptr += "<div class='reading'>";
  ptr += "<i class='fas fa-burn icon'></i>";
  ptr += "<h2>Saturação de Oxigênio (SpO2)</h2>";
  ptr += "<p>" + String(SpO2) + "%</p>";
  ptr += "</div>";
  ptr += "</div>";
  ptr += "</body>";
  ptr += "</html>";
  return ptr;
}

