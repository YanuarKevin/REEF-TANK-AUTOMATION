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
#define SALINITAS_PIN A0

#define NUM_REGISTERS 1
#define NUM_CHANNELS (NUM_REGISTERS * 4)

char auth[] = "j920xDAK2OfptZGmFJ_nbdAHLg6pkld1";
char ssid[] = "OKE_JON";
char pass[] = "banyuwangi";

bool relayStates[NUM_CHANNELS] = { false, false, false, false };  // Mulai dengan relay dimatikan (low)
bool notificationSentDHT = false;                                 // Penanda untuk notifikasi yang sudah dikirim untuk DHT11
bool notificationSentDS18B20 = false;
bool autoMode_fan = false;
bool previousFanState = false;
bool notificationSentSalinitasup = false;
bool notificationSentSalinitasdown = false;

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long previousMillis = 0;
const long interval = 1000; 
float temperature_max = 30.0;
float temperature_min = 27.0;
int salinitas_max = 0;
int salinitas_min = 0;

byte Derajat[8] = { 
  0b00110,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
byte Termometer[8] = {
  0b00100,
  0b01010,
  0b01010,
  0b01010,
  0b01110,
  0b11111,
  0b11111,
  0b01110 
};

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass, "iot.serangkota.go.id", 8080);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, Derajat);
  lcd.createChar(1, Termometer);
  lcd.setCursor(2, 0);
  lcd.print("COPYRIGHT BY");
  lcd.setCursor(2, 1);
  lcd.print("YK REEF STORE");
  delay(5000);
  lcd.clear();

  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(SALINITAS_PIN, INPUT);

  dht.begin();
  sensors.begin();

  for (int i = 0; i < NUM_CHANNELS; i++) {
    relayStates[i] = true;
  }

  updateShiftRegister();

  servo.attach(SERVO_PIN);  // Menghubungkan servo ke pin yang ditentukan

  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V5);
  Blynk.syncVirtual(V6);
  Blynk.syncVirtual(V7);
  Blynk.syncVirtual(V8);
  Blynk.syncVirtual(V9);
}

void loop() {
  Blynk.run();
  unsigned long currentMillis = millis();  // Ambil waktu sekarang

  if (currentMillis - previousMillis >= interval) {
    // Simpan waktu sekarang sebagai waktu terakhir tindakan dilakukan
    previousMillis = currentMillis;
    readDHT();
    readDS18B20();
    readSalinity();
  }
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V5);
  Blynk.syncVirtual(V6);
  Blynk.syncVirtual(V7);
  Blynk.syncVirtual(V8);
  Blynk.syncVirtual(V9);
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

BLYNK_WRITE(V1) {
  int buttonState = param.asInt();
  if (buttonState == 1 && !autoMode_fan) {
    Blynk.virtualWrite(V2, 1);  // Matikan tombol manual di aplikasi Blynk 
    autoMode_fan = true;  // Aktifkan kontrol otomatis
    Serial.println("Mode Otomatis Aktif");  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("KIPAS OTOMATIS :");
    lcd.setCursor(6, 1);
    lcd.print("AKTIF");
    delay(2000);
    lcd.clear();
  } else if (buttonState == 0 && autoMode_fan) {
    autoMode_fan = false;
    Serial.println("Mode Otomatis Nonaktif");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("KIPAS OTOMATIS :");
    lcd.setCursor(3, 1);
    lcd.print("NON-AKTIF");
    delay(2000);
    lcd.clear();
  }
}

BLYNK_WRITE(V2) {
  int pinValue = param.asInt();  // Mendapatkan nilai dari tombol di aplikasi Blynk
  if (pinValue == 1 || pinValue == 0) {
    relayStates[0] = pinValue;  // Mengatur nilai relay sesuai dengan input dari Blynk
    updateShiftRegister();
  }
}

BLYNK_WRITE(V3) {
  int pinValue = param.asInt();
  if (pinValue == 1 || pinValue == 0) {
    relayStates[1] = pinValue;
    updateShiftRegister();
  }
}

BLYNK_WRITE(V4) {
  int pinValue = param.asInt();
  if (pinValue == 1 || pinValue == 0) {
    relayStates[2] = pinValue;
    updateShiftRegister();
  }
}

BLYNK_WRITE(V5) {
  int pinValue = param.asInt();
  if (pinValue == 1 || pinValue == 0) {
    relayStates[3] = pinValue;
    updateShiftRegister();
  }
}

BLYNK_WRITE(V0) {
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

BLYNK_WRITE(V6) {
  temperature_max = param.asFloat();  // Mendapatkan nilai dari Blynk (float / desimal)
  Serial.print("SETTING SUHU MAX: ");
  Serial.print(temperature_max);
  Serial.println("°C");
  updateTemperatureSettings(temperature_min, temperature_max);
}

BLYNK_WRITE(V7) {
  temperature_min = param.asFloat();
  Serial.print("SETTING SUHU MIN: ");
  Serial.print(temperature_min);
  Serial.println("°C");
  updateTemperatureSettings(temperature_min, temperature_max);
}

BLYNK_WRITE(V8) {
  salinitas_max = param.asInt();  // Mendapatkan nilai dari Blynk (int / bulat)
  Serial.print("SETTING SALINITAS MAX: ");
  Serial.println(salinitas_max);
}

BLYNK_WRITE(V9) {
  salinitas_min = param.asInt();
  Serial.print("SETTING SALINITAS MIN: ");
  Serial.println(salinitas_min);
}

void updateTemperatureSettings(float minTemp, float maxTemp) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(1));
  lcd.print(" SETTING UPDATE");
  lcd.setCursor(1, 1);
  lcd.print(minTemp);
  lcd.print(" - ");
  lcd.print(maxTemp);
  lcd.write(byte(0));
  lcd.print("C");
  delay(3000);
  lcd.clear();
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

  lcd.setCursor(0, 1);
  lcd.write(byte(1));
  lcd.print(" RUANG: ");
  lcd.print(t, 1);
  lcd.write(byte(0));
  lcd.print("C");
  Blynk.virtualWrite(V11, t);
  Blynk.virtualWrite(V12, h);
}

void readDS18B20() {
  sensors.requestTemperatures();             // Mulai proses pengukuran suhu
  float tempC = sensors.getTempCByIndex(0);  // Mendapatkan suhu dalam derajat Celsius

  if (tempC >= temperature_max && !notificationSentDS18B20) {
    // Blynk.virtualWrite(V2, 0); // Menyalakan V0 jika suhu melebihi batas (tidak fungsi dalam program ini)
    Serial.println("Suhu Air Melebihi Batas !!!");
    Blynk.notify("Suhu Air Melebihi Batas !!!");  
    Blynk.email("yanuarkevinbwi31@gmail.com", "KONTROL AQUARIUM", "Suhu Air Melebihi Batas !!!");
    notificationSentDS18B20 = true;  // Menandai bahwa notifikasi telah dikirimkan
    lcd.clear();                     
    lcd.setCursor(0, 0);
    lcd.print("SUHU PADA AIR");  
    lcd.setCursor(0, 1);
    lcd.print("MELEBIHI BATAS !!");  
    delay(4000);                     
    lcd.clear();                     
  } else if (tempC <= temperature_min) {
    notificationSentDS18B20 = false;  // Reset penanda notifikasi jika pembacaan berhasil
  }
  if (autoMode_fan) {
    if (tempC >= temperature_max && !previousFanState) {
      relayStates[0] = 0;  // Nyalakan relay (relay 0) jika suhu lebih dari temperature_max
      updateShiftRegister();
      Blynk.virtualWrite(V2, 0);
      Serial.println("Kipas Dinyalakan (Auto)");
      previousFanState = true;  // Simpan status kipas sebagai menyala
    } else if (tempC <= temperature_min && previousFanState) {
      relayStates[0] = 1;
      updateShiftRegister();
      Blynk.virtualWrite(V2, 1);
      Serial.println("Kipas Dimatikan (Auto)");
      previousFanState = false;
    }
  }
  lcd.setCursor(0, 0);
  lcd.write(byte(1));
  lcd.print(" AIR: ");
  lcd.print(tempC);
  lcd.write(byte(0));
  lcd.print("C");
  Blynk.virtualWrite(V10, tempC);
}

void readSalinity() {
  int salinitasair = analogRead(SALINITAS_PIN);

  if (salinitasair > salinitas_max && !notificationSentSalinitasup) {
    Serial.println("Salinitas Air Tinggi !!!");
    Blynk.notify("Salinitas Air Tinggi !!!");  
    Blynk.email("yanuarkevinbwi31@gmail.com", "KONTROL AQUARIUM", "Salinitas Air Tinggi !!!");
    notificationSentSalinitasup = true;
    notificationSentSalinitasdown = false;
  }
  if (salinitasair < salinitas_min && !notificationSentSalinitasdown) {
    Serial.println("Salinitas Air Rendah !!!");
    Blynk.notify("Salinitas Air Rendah !!!");  
    Blynk.email("yanuarkevinbwi31@gmail.com", "KONTROL AQUARIUM", "Salinitas Air Rendah !!!");
    notificationSentSalinitasdown = true;
    notificationSentSalinitasup = false;  // Reset tinggi saat rendah
  }
  if (salinitasair >= salinitas_min && salinitasair <= salinitas_max) {
    if (notificationSentSalinitasup || notificationSentSalinitasdown) {
      notificationSentSalinitasup = false;
      notificationSentSalinitasdown = false;
      Serial.println("Salinitas Air Normal.");
    }
  }
  Blynk.virtualWrite(V13, salinitasair);
}