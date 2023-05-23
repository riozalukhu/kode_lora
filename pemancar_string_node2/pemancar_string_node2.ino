#include <LoRa.h> 
#include <ArduinoJson.h>

const int mq4Pin = A0; // pin analog untuk membaca tegangan keluaran sensor MQ-4
float mq4Voltage = 0; // variabel untuk menyimpan nilai tegangan keluaran
float mq4Resistance = 0; // variabel untuk menyimpan nilai resistansi sensor
int mq4PPM; // variabel untuk menyimpan nilai konsentrasi gas CH4 dalam udara

int id = 2;

const int analogInPin = A1; // Pin analog input untuk MQ-7
const float VCC = 5.0;      // Tegangan referensi
const float RL = 1.0;       // Resistor beban
const float Ro = 10.0;      // Nilai resistor sensor pada udara bersih
const float Rs_Ro_CO = 4.4; // Rasio Rs/Ro untuk CO (dari datasheet)

int dustPin = A2; // Pin analog input untuk sensor debu
int ledPin = 2; // Pin untuk LED indikator
int delayTime = 280; // Waktu tunggu sebelum membaca nilai sensor
int delayTime2 = 40; // Waktu tunggu setelah membaca nilai sensor
float voltage2 = 0; // Variabel untuk menyimpan nilai tegangan keluaran
int dustDensity; // Variabel untuk menyimpan nilai densitas debu
 
void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT); // Set pin LED sebagai output
  while (!Serial);
 
  Serial.println("LoRa Sender");
 
  if (!LoRa.begin(920E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(7);  // diganti dengan rentang 7 - 12 
  //LoRa.setSignalBandwidth(500E3);
  //LoRa.setCodingRate4(8); //5, 6, 7, 8
}
 
void loop() {
  //kode sensor mq4
  int sensorValue = analogRead(mq4Pin);
  mq4Voltage = (sensorValue / 1024.0) * 5.0;

  // hitung resistansi sensor MQ-4 menggunakan rumus yang disediakan oleh produsen sensor
  mq4Resistance = ((5.0 - mq4Voltage) / mq4Voltage) * 10.0;

  // hitung konsentrasi gas CH4 dalam udara menggunakan rumus yang disediakan oleh produsen sensor
  mq4PPM = pow(10, ((log10(mq4Resistance / 1.5) - 0.48) / 0.7));
  
  
  Serial.print("mq4ppm: ");
  Serial.println(mq4PPM);

  //kode sensor mq7
  int MQ7 = analogRead(analogInPin); // Baca nilai sensor
  int voltage = (MQ7  / 1024.0) * 5.0; // Konversi nilai sensor menjadi tegangan
  int Rs_Ro = ((5.0 - voltage) / voltage ) * 10.0;
  int CO_ppm = pow(10, ((log10(Rs_Ro / 5.0) + 0.4771 ) / (-0.8502))); // Hitung konsentrasi CO (dalam ppm)
  Serial.print(", mq7PPM: ");
  Serial.println(CO_ppm);
  delay(1000); // Tunggu selama 1 detik sebelum membaca ulang

  digitalWrite(ledPin, HIGH); // Nyalakan LED
  delayMicroseconds(delayTime); // Tunggu selama 280us
  
  // Baca nilai tegangan dan hitung nilai densitas debu
  int sensordebu = analogRead(dustPin);
  voltage2 = (sensordebu / 1024.0) * 5.0;
  dustDensity = 0.17 * voltage - 0.1;
  
  digitalWrite(ledPin, LOW); // Matikan LED
  delayMicroseconds(delayTime2); // Tunggu selama 40us
  
  // Kirim data ke Serial Monitor
  
  Serial.print("Dust Density: ");
  Serial.print(dustDensity);
  Serial.println("ug/m3");
  
  
  
  // Kirim paket LoRa dengan data JSON
  LoRa.beginPacket();
  LoRa.print(id);
  LoRa.print(",");
  LoRa.print(mq4PPM);
  LoRa.print(",");
  LoRa.print(CO_ppm);
  LoRa.print(",");
  LoRa.print(dustDensity);
  LoRa.endPacket();

  delay(2000);
}
