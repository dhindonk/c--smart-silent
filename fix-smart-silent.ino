#include <WiFi.h>
#include <HTTPClient.h>

#define SOUND_SENSOR_PIN 35   // Pin analog ESP32 untuk KY-038
#define BUZZER_PIN 4          // Pin untuk buzzer
#define THRESHOLD 2000        // Threshold untuk level suara signifikan
#define POST_INTERVAL 5000    // Interval untuk pengiriman data (5 detik)

// Ganti dengan kredensial WiFi Anda
const char* ssid = "DhinTech";
const char* password = "";

// URL untuk API server Laravel
const String baseUrl = "http://192.168.169.160:8000/api"; // Ganti dengan URL server Laravel Anda

unsigned long lastPostTime = 0; // Variabel untuk mengatur waktu pengiriman data

void setup() {
  Serial.begin(115200);  // Memulai komunikasi serial
  pinMode(SOUND_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Menghubungkan ke WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  int soundLevel = analogRead(SOUND_SENSOR_PIN);  // Membaca level suara
  Serial.println(soundLevel);  // Menampilkan level suara ke Serial Monitor

  // Kendalikan buzzer berdasarkan level suara secara real-time
  if (soundLevel > THRESHOLD) {
    digitalWrite(BUZZER_PIN, HIGH);  // Nyalakan buzzer
  } else {
    digitalWrite(BUZZER_PIN, LOW);   // Matikan buzzer
  }

  // Kirim data ke server setiap 5 detik
  if (millis() - lastPostTime >= POST_INTERVAL) {
    lastPostTime = millis();
    sendNoiseDataToServer(soundLevel);
    
    // Periksa status buzzer dari server
    bool buzzerStatus = getBuzzerStatusFromServer();

    // Kendalikan buzzer berdasarkan status dari server
    if (buzzerStatus) {
      digitalWrite(BUZZER_PIN, HIGH);  // Nyalakan buzzer dari server
      delay(3000);
    }
  }

  delay(100); // Delay untuk menghindari spam pembacaan
}

// Fungsi untuk mengirimkan data kebisingan ke server Laravel
void sendNoiseDataToServer(int noiseLevel) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String url = baseUrl + "/noise";  // Endpoint untuk mengirimkan data noise
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"noise_level\": " + String(noiseLevel) + "}"; // JSON payload

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.println("Data noise sent successfully");
    } else {
      Serial.println("Error sending noise data");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

// Fungsi untuk mendapatkan status buzzer dari server
bool getBuzzerStatusFromServer() {
  bool status = false;

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String url = baseUrl + "/buzzer";  // Endpoint untuk mendapatkan status buzzer
    http.begin(url);

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println(payload);

      // Mengambil status buzzer dari response JSON
      if (payload.indexOf("\"status\":true") != -1) {
        status = true;
      }
    } else {
      Serial.println("Error getting buzzer status");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }

  return status;
}
