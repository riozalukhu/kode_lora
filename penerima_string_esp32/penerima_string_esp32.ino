#include <LoRa.h>
#include <ArduinoJson.h>

#define BAUD_RATE 9600

void setup() {
  Serial.begin(BAUD_RATE);
  while (!Serial);

  Serial2.begin(BAUD_RATE);
  while (!Serial2);

  if (!LoRa.begin(920E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(7); // diganti dengan rentang 7, 8, 9, 10, 11, 12
  //LoRa.setSignalBandwidth(125E3); 125, 250, 500
  //LoRa.setCodingRate4(5); 5, 6, 7, 8
  // Set up WiFi and MQTT
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Membaca data yang diterima
    String message = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }

    // Memisahkan data menjadi token berdasarkan koma
    String tokens[4]; // Asumsikan ada 4 token dalam data
    int tokenIndex = 0;
    int lastIndex = 0;
    for (int i = 0; i < message.length(); i++) {
      if (message.charAt(i) == ',') {
        tokens[tokenIndex++] = message.substring(lastIndex, i);
        lastIndex = i + 1;
      }
    }
    tokens[tokenIndex] = message.substring(lastIndex);

    // Memeriksa jumlah token yang sesuai
    if (tokenIndex == 3) {
      int id = tokens[0].toInt();
      int mq4PPM = tokens[1].toInt();
      int mq7PPM = tokens[2].toInt();
      int dustDensity = tokens[3].toInt();

      // Membuat objek JSON
      StaticJsonDocument<100> doc;
      doc["id"] = id;
      doc["mq4"] = mq4PPM;
      doc["mq7"] = mq7PPM;
      doc["debu"] = dustDensity;
      doc["rssi"] = LoRa.packetRssi();
      doc["snr"] = LoRa.packetSnr();

      // Mengonversi objek JSON menjadi string JSON
      String json;
      serializeJson(doc, json);

      // Mengirimkan string JSON melalui Serial ke ESP32
      Serial.println(json);
      Serial2.println(json);
    }
  }
}
