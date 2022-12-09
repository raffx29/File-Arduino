// Library
#include <AntaresESP32HTTP.h>    // Library ESP32 HTTP ANTARES
#include <DHT.h>                 // Library Sensor DHT
#include <WiFi.h>                // Library Wifi ESP32
#include <WiFiClient.h>          // Library Wifi ESP32
#include <WebServer.h>           // Library Web Server ESP32
#include <ESPmDNS.h>             // Library DNS ESP32
#include "EasyNextionLibrary.h"  // Library Nextion Display

// Deklarasi
#define ACCESSKEY "6a815df5659580b0:be0b63231457e082"  // Accesskey akun Antares user
#define WIFISSID "BBP MEKTAN"                          // SSID WiFi user
#define PASSWORD "1ndoJarwo"                           // Password WiFi user

// Pin
#define LDRPIN 34          // Pin Sensor LDR
#define SOILPIN 35         // pin Sensor SOIL
#define DHTPIN 32          // Pin Sensor DHT
#define DHTTYPE DHT22      // Type Sensor DHT
DHT dht(DHTPIN, DHTTYPE);  // Inisialisasi pin DHT & Tipe DHT

// Relay
#define relay1 5   // WATER PUMP
#define relay2 17  // LAMPU

// Application-Device
#define applicationName "SmartAgricultureSystem"  // Nama Aplikasi pada ANTARES
#define deviceName "ESP32"                        // Nama Device pada ANTARES

AntaresESP32HTTP antares(ACCESSKEY);  // Inisialisasi Accesskey akun Antares

MDNSResponder mdns;
WebServer server(80);
String webpage = "";

IPAddress local_IP(192, 158, 60, 168);
IPAddress gateway(192, 168, 60, 1);
IPAddress subnet(255, 255, 255, 0);

float humUdara, tempUdara, humTanah;
int ldr;

EasyNex myNex(Serial);

void setup() {
  myNex.begin(9600);
  Serial.begin(9600);
  dht.begin();
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);

  antares.wifiConnection(WIFISSID, PASSWORD);
  antares.setDebug(true);

  WiFi.begin(WIFISSID, PASSWORD);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFISSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (mdns.begin("esp")) {
    Serial.println("MDNS responder started");
  }
  server.begin();
  Serial.println("HTTP server started");

  androidButton();
}

void loop() {
  myNex.NextionListen();
  readSensor();
  webPage();
  nextSensor();
//  otomatis();

  server.handleClient();

  Serial.println("Kelembaban Udara : " + (String)humUdara + " %");
  Serial.println("Temperatur Udara : " + (String)tempUdara + " *C");
  Serial.println("Intensitas Cahaya : " + (String)ldr + " Cd");
  Serial.println("Kelembaban Tanah : " + (String)humTanah + " %");

  antares.add("Kelembaban Udara", humUdara);
  antares.add("Temperatur Udara", tempUdara);
  antares.add("Intensitas Cahaya", ldr);
  antares.add("Kelembaban Tanah", humTanah);

  antares.send(applicationName, deviceName);
  delay(1000);
}

void readSensor() {
  humUdara = dht.readHumidity();
  tempUdara = dht.readTemperature();
  humTanah = analogRead(SOILPIN);
  humTanah = map(humTanah, 0, 4095, 100, 0);
  ldr = analogRead(LDRPIN);
}

void webPage() {
  webpage += "<h1>ESP32 Web Server</h1><p>WATER PUMP <a href=\"relay1ON\"><button>ON</button></a>&nbsp;<a href=\"relay1OFF\"><button>OFF</button></a></p>";
  webpage += "<p>LAMPU <a href=\"relay2ON\"><button>ON</button></a>&nbsp;<a href=\"relay2OFF\"><button>OFF</button></a></p>";
}

void androidButton() {
  server.on("/", []() {
    server.send(200, "text/html", webpage);
  });

  server.on("/relay1ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(relay1, LOW);
    delay(1000);
  });

  server.on("/relay1OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(relay1, HIGH);
    delay(1000);
  });

  server.on("/relay2ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(relay2, LOW);
    delay(1000);
  });

  server.on("/relay2OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(relay2, HIGH);
    delay(1000);
  });
}

void nextSensor() {
  String tempUdaraString = String(tempUdara, 1);
  myNex.writeStr("t1.txt", tempUdaraString);
  int tempUdaraInt = tempUdara * 10;
  myNex.writeNum("x1.val", tempUdaraInt);

  String humUdaraString = String(humUdara, 1);
  myNex.writeStr("t2.txt", humUdaraString);
  int humUdaraInt = humUdara * 10;
  myNex.writeNum("x2.val", humUdaraInt);

  String ldrString = String(ldr, 1);
  myNex.writeStr("t0.txt", ldrString);
  int ldrInt = ldr * 10;
  myNex.writeNum("x0.val", ldrInt);

  String humTanahString = String(humTanah, 1);
  myNex.writeStr("t3.txt", humTanahString);
  int humTanahInt = humTanah * 10;
  myNex.writeNum("x3.val", humTanahInt);
}

void otomatis() {
  if (humTanah > 21 && ldr > 500) {
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
    Serial.println("PUMP ON LAMPU ON");
  } else if (humTanah < 21 && ldr > 500) {
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, HIGH);
    Serial.println("PUMP OFF LAMPU ON");
  } else if (humTanah > 21 && ldr < 500) {
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, LOW);
    Serial.println("PUMP ON LAMPU OFF");
  } else if (humTanah < 21 && ldr < 500) {
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    Serial.println("PUMP OFF LAMPU OFF");
  }
}