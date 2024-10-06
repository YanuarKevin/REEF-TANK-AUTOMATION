#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SPI.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

#define DATA_PIN D6
#define CLOCK_PIN D7
#define LATCH_PIN D0
#define DHTPIN D5
#define DHTTYPE DHT11
#define ONE_WIRE_BUS D3
#define SERVO_PIN D4

#define NUM_REGISTERS 1
#define NUM_CHANNELS (NUM_REGISTERS * 4)

char auth[] = "j920xDAK2OfptZGmFJ_nbdAHLg6pkld1";
char ssid[] = "OKE-JON-1";
char pass[] = "banyuwangi";

bool relayStates[NUM_CHANNELS] = { false, false, false, false };  // Mulai dengan relay dimatikan (low)
bool notificationSentDHT = false;                                 // Penanda untuk notifikasi yang sudah dikirim untuk DHT11
bool notificationSentDS18B20 = false;

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long previousMillis = 0;
const long interval = 1000;  // interval dalam milidetik (misalnya 2000 = 2 detik)
float temperatureLimit = 0;  // Inisialisasi batas suhu awal

// PEMBUATAN SIMBOL DERAJAT
byte customDegreeChar[8] = { 
  B00110,
  B01001,
  B01001,
  B00110,
  B00000,
  B00000,
  B00000,
  B00000
};

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass, "iot.serangkota.go.id", 8080);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(2, 0);
  lcd.print("COPYRIGHT BY");
  lcd.setCursor(2, 1);
  lcd.print("YANUAR KEVIN");
  delay(5000);
  lcd.clear();

  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  dht.begin();
  sensors.begin();

  for (int i = 0; i < NUM_CHANNELS; i++) {
    relayStates[i] = true;
  }

  updateShiftRegister();

  servo.attach(SERVO_PIN);  // Menghubungkan servo ke pin yang ditentukan

  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V8);
}

void loop() {
  Blynk.run();
  unsigned long currentMillis = millis();  // Ambil waktu sekarang

  if (currentMillis - previousMillis >= interval) {
    // Simpan waktu sekarang sebagai waktu terakhir tindakan dilakukan
    previousMillis = currentMillis;

    readDHT();
    readDS18B20();
  }
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V8);
}

void updateShiftRegister() {
  digitalWrite(LATCH_PIN, LOW);
  for (int i = NUM_CHANNELS - 1; i >= 0; i--) {
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(DATA_PIN, relayStates[i]);
    digitalWrite(CLOCK_PIN, HIGH);
  }
  digitalWrite(LATCH_PIN, HIGH);

  // Serial print status relay
  Serial.println("Relay Status:");
  for (int i = 0; i < NUM_CHANNELS; i++) {
    Serial.print("Relay ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(relayStates[i] ? "OFF" : "ON");
  }
}

BLYNK_WRITE(V0) {
  int pinValue = param.asInt();  // Mendapatkan nilai dari tombol di aplikasi Blynk
  if (pinValue == 1 || pinValue == 0) {
    relayStates[0] = pinValue;  // Mengatur nilai relay sesuai dengan input dari Blynk
    updateShiftRegister();
  }
}

BLYNK_WRITE(V1) {
  int pinValue = param.asInt();
  if (pinValue == 1 || pinValue == 0) {
    relayStates[1] = pinValue;
    updateShiftRegister();
  }
}

BLYNK_WRITE(V2) {
  int pinValue = param.asInt();
  if (pinValue == 1 || pinValue == 0) {
    relayStates[2] = pinValue;
    updateShiftRegister();
  }
}

BLYNK_WRITE(V3) {
  int pinValue = param.asInt();
  if (pinValue == 1 || pinValue == 0) {
    relayStates[3] = pinValue;
    updateShiftRegister();
  }
}

BLYNK_WRITE(V8) {
  temperatureLimit = param.asFloat();  // Mendapatkan nilai batas suhu dari aplikasi Blynk
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Batas Suhu New: ");
  lcd.setCursor(7, 1);
  lcd.print(temperatureLimit);
  lcd.write(1);
  lcd.print("C");
  delay(2000);
  lcd.clear();
}

BLYNK_WRITE(V7) {
  int buttonState = param.asInt();
  if (buttonState == 1) {
    for (int pos = 180; pos >= 0; pos--) {  // Mengubah perulangan dari 0 derajat ke 180 derajat
      servo.write(pos);
      // delay(1); // Menambahkan sedikit penundaan agar pergerakan servo terlihat halus
    }
    servo.write(180);  // Mengembalikan servo ke posisi awal (180 derajat)
    Blynk.notify("IKAN SUDAH DIBERI PAKAN !!!");
    Blynk.email("yanuarkevinbwi31@gmail.com", "KONTROL AQUAROIUM 1", "IKAN SUDAH DIBERI PAKAN !!!");
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("IKAN SUDAH DI");
    lcd.setCursor(1, 1);
    lcd.print("BERI PAKAN !!!");
    delay(4000);
    lcd.clear();
  }
}

void readDHT() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (t >= 33 && !notificationSentDHT) {
    Blynk.notify("Suhu Ruangan Tinggi !!!");
    Blynk.email("yanuarkevinbwi31@gmail.com", "KONTROL AQUARIUM", "Suhu Ruangan Tinggi !!!");
    notificationSentDHT = true;  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SUHU DI RUANGAN");
    lcd.setCursor(0, 1);
    lcd.print("SANGAT TINGGI !!");
    delay(4000);
    lcd.clear();
  } else if (t <= 30.5) {
    notificationSentDHT = false;
  }

  lcd.setCursor(0, 0);
  lcd.print("SUHU RUANG: ");
  lcd.setCursor(11, 0);
  lcd.print(t, 1);
  lcd.print("C");
  Blynk.virtualWrite(V4, t);
  Blynk.virtualWrite(V5, h);
}

void readDS18B20() {
  sensors.requestTemperatures();             // Mulai proses pengukuran suhu
  float tempC = sensors.getTempCByIndex(0);  // Mendapatkan suhu dalam derajat Celsius

  if (tempC >= temperatureLimit && !notificationSentDS18B20) {
    Blynk.virtualWrite(V0, 1); // Menyalakan V0 jika suhu melebihi batas
    Serial.println("Suhu Air Melebihi Batas !!!");
    Blynk.notify("Suhu Air Melebihi Batas !!!");  
    Blynk.email("yanuarkevinbwi31@gmail.com", "KONTROL AQUARIUM", "Suhu Air Melebihi Batas !!!");
    notificationSentDS18B20 = true;  // Menandai bahwa notifikasi telah dikirimkan
    lcd.clear();                     
    lcd.setCursor(1, 0);
    lcd.print("SUHU PADA AIR");  
    lcd.setCursor(0, 1);
    lcd.print("MELEBIHI BATAS !!");  
    delay(4000);                     
    lcd.clear();                     
  } else if (tempC <= temperatureLimit - 1) {
    Blynk.virtualWrite(V0, 0);
    notificationSentDS18B20 = false;  // Reset penanda notifikasi jika pembacaan berhasil
  }

  lcd.setCursor(0, 1);
  lcd.print("SUHU AIR: ");
  lcd.setCursor(9, 1);
  lcd.print(tempC, 1);
  lcd.write(1);
  lcd.print("C");
  Blynk.virtualWrite(V6, tempC);
}
