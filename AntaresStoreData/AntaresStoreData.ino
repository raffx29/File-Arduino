// Library
#include <AntaresESP32HTTP.h>
#include <DHT.h>

// Deklarasi
#define ACCESSKEY "6a815df5659580b0:be0b63231457e082"  // Ganti dengan access key akun Antares anda
#define WIFISSID "BBP MEKTAN"                          // Ganti dengan SSID WiFi anda
#define PASSWORD "1ndoJarwo"                           // Ganti dengan password WiFi anda

// Pin
#define LDRPIN 34
#define SOILPIN 35
#define DHTPIN 18
#define DHTTYPE DHT22
#define relay1 5
#define relay2 17

// Application-Device
#define applicationName "SmartAgricultureSystem"  // Ganti dengan application name Antares yang telah dibuat
#define deviceName "ESP32"                        // Ganti dengan device Antares yang telah dibuat

AntaresESP32HTTP antares(ACCESSKEY);  // Buat objek antares
DHT dht(DHTPIN, DHTTYPE);             // Buat objek DHT

void setup() {
  Serial.begin(115200);
  dht.begin();
  antares.setDebug(true);                     
  antares.wifiConnection(WIFISSID, PASSWORD);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
}

void loop() {
  // Isi variabel dengan nilai acak, dengan tipe data yang berbeda
  // Sensor DHT22
  float humUdara = dht.readHumidity();              // Baca kelembapan Udara
  float tempUdara = dht.readTemperature();          // Baca temperatur Udara
  Serial.println("Kelembaban Udara : " + (String)humUdara + " %");
  Serial.println("Temperatur Udara: " + (String)tempUdara + " *C");
  antares.add("Kelembaban Udara", humUdara);
  antares.add("Temperatur Udara", tempUdara);
  if (isnan(humUdara) || isnan(tempUdara)) {     // Cek jika sensor error
      Serial.println("Gagal Membaca DHT sensor!");
      return;
	}

  // Sensor LDR
  int ldr = analogRead(LDRPIN);  // Baca Intensitas Cahaya
  Serial.println("Intensitas Cahaya : " + (String)ldr + " Cd");
  antares.add("Intensitas Cahaya", ldr);
  if (isnan(ldr)) {     // Cek jika sensor error
      Serial.println("Gagal Membaca LDR sensor!");
      return;
	}  

  // Sensor Soil
  int nilaiADS = analogRead(SOILPIN);  // Baca Kelembaban Tanah
  int humTanah = (convertToPercent(nilaiADS));
  Serial.println("Kelembaban Tanah : " + (String)humTanah + " %");
  antares.add("Kelembaban Tanah", humTanah);
  if (isnan(nilaiADS)) {     // Cek jika sensor error
      Serial.println("Gagal Membaca SOIL sensor!");
      return;
	}  

  // Kirim dari penampungan data ke Antares
  antares.send(applicationName, deviceName);
  delay(10000);

if (humTanah > 21 && ldr > 500) {
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
    Serial.println("KIPAS ON LAMPU ON");
  } else if (humTanah < 21 && ldr > 500) {
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, HIGH);
    Serial.println("KIPAS OFF LAMPU ON");
  } else if (humTanah > 21 && ldr < 500) {
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, LOW);
    Serial.println("KIPAS ON LAMPU OFF");
  } else if (humTanah < 21 && ldr < 500) {
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    Serial.println("KIPAS OFF LAMPU OFF");
  }
  
}

  int convertToPercent(int value) {
    int percentValue = 0;
    percentValue = map (value, 0, 4095, 100, 0);
    return percentValue;
  }