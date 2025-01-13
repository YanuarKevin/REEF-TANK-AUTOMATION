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
char ssid[] = "ojo omong ae";
char pass[] = "banyuwangi";

bool relayStates[NUM_CHANNELS] = { false, false, false, false };
bool notificationSentDHT = false;                      
bool notificationSentDS18B20 = false;
bool autoMode_fan = false;
bool previousFanState = false;

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long previousMillis = 0; // interval waktu sekarang
const long interval = 100;  // interval prosses

float temp_ruang = 0.0;
int hum_ruang = 0;
float tempC = 0.0;
float temperature_max = 30.0;
float temperature_min = 27.0;

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
byte Air[8] = {
  0b00100,
  0b00100,
  0b01110,
  0b11111,
  0b11111,
  0b01110,
  0b00000,
  0b00000
};

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass, "iot.serangkota.go.id", 8080); 

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, Derajat);
  lcd.createChar(1, Termometer);
  lcd.createChar(2, Air);
  lcd.setCursor(2, 0);
  lcd.print("COPYRIGHT BY");
  lcd.setCursor(2, 1);
  lcd.print("YK REEF STORE");
  delay(5000);
  lcd.clear();

  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  servo.attach(SERVO_PIN);

  dht.begin();
  sensors.begin();

  for (int i = 0; i < NUM_CHANNELS; i++) {
    relayStates[i] = true;
  }
  updateShiftRegister();

  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V5);
  Blynk.syncVirtual(V6);
  Blynk.syncVirtual(V7);
}

void loop() {
  Blynk.run();
  unsigned long currentMillis = millis();  // Ambil waktu sekarang

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    readDHT();
    readDS18B20();
    updateLCDDisplay();
  }
  // updateLCDDisplay();
  // readDHT();
  // readDS18B20();
}

BLYNK_CONNECTED() {
  for (int i = 0; i <= 7; i++) {
    Blynk.syncVirtual(i);
  }
}

void updateShiftRegister() {
  digitalWrite(LATCH_PIN, LOW);
  for (int i = NUM_CHANNELS - 1; i >= 0; i--) {
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(DATA_PIN, relayStates[i]);
    digitalWrite(CLOCK_PIN, HIGH);
  }
  digitalWrite(LATCH_PIN, HIGH);

  Serial.println("Relay Status:");
  for (int i = 0; i < NUM_CHANNELS; i++) {
    Serial.print("Relay ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(relayStates[i] ? "OFF" : "ON");
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
    Blynk.email("yanuarkevinbwi31@gmail.com", "SMART REEF TANK", "IKAN SUDAH DIBERI PAKAN !!!");
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("IKAN SUDAH DI");
    lcd.setCursor(1, 1);
    lcd.print("BERI PAKAN !!!");
    delay(3000);
    lcd.clear();
  }
}

BLYNK_WRITE(V1) {
  int buttonState = param.asInt();
  if (buttonState == 1 && !autoMode_fan) {
    autoMode_fan = true;  // Aktifkan kontrol otomatis
    Blynk.virtualWrite(V2, 1);  // Matikan tombol manual di aplikasi Blynk
    Serial.println("Mode Otomatis Aktif");
  } else if (buttonState == 0 && autoMode_fan) {
    autoMode_fan = false;
    Serial.println("Mode Otomatis Nonaktif");
  }
}

BLYNK_WRITE(V2) {
  int pinValue = param.asInt();  // Dapatkan status tombol manual (1=ON, 0=OFF)
  if (pinValue == 1 || pinValue == 0) {
    if (autoMode_fan == false) {
      relayStates[0] = pinValue;
      updateShiftRegister();
    }
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

// 7. SETTING SUHU MAX
BLYNK_WRITE(V6) {
  temperature_max = param.asFloat();  // Mendapatkan nilai dari Blynk (float / desimal)
  Serial.print("SETTING SUHU MAX: ");
  Serial.print(temperature_max);
  Serial.println("°C");
}
// 8. SETTING SUHU MIN
BLYNK_WRITE(V7) {
  temperature_min = param.asFloat();
  Serial.print("SETTING SUHU MIN: ");
  Serial.print(temperature_min);
  Serial.println("°C");
}

// PEMBACAAN DAN KOFIGURASI SENSOR
void readDHT() {
  hum_ruang = dht.readHumidity(); 
  temp_ruang = dht.readTemperature();
  
  if (temp_ruang >= 33 && !notificationSentDHT) {
    Blynk.notify("Suhu Ruangan Tinggi !!!");
    Blynk.email("yanuarkevinbwi31@gmail.com", "SMART REEF TANK", "Suhu Ruangan Tinggi !!!");
    notificationSentDHT = true;  
  } else if (temp_ruang <= 30.5 && notificationSentDHT) {
    notificationSentDHT = false;
  }
  Serial.print("suhu ruang: ");
  Serial.println(temp_ruang);
  Blynk.virtualWrite(V11, temp_ruang);
  Blynk.virtualWrite(V12, hum_ruang);
}

void readDS18B20() {
  sensors.requestTemperatures();  
  tempC = sensors.getTempCByIndex(0);

  if (tempC >= temperature_max && !notificationSentDS18B20) {
    Serial.println("Suhu Air Melebihi Batas !!!");
    Blynk.notify("Suhu Air Melebihi Batas !!!");  
    Blynk.email("yanuarkevinbwi31@gmail.com", "SMART REEF TANK", "Suhu Air Melebihi Batas !!!");
    notificationSentDS18B20 = true;                    
  } else if (tempC <= temperature_min && notificationSentDS18B20) {
    notificationSentDS18B20 = false;
  }

  if (autoMode_fan) {
    if (tempC >= temperature_max && !previousFanState) {
      relayStates[0] = 1;  // Nyalakan relay (relay 0) jika suhu lebih dari temperature_max
      updateShiftRegister();
      Serial.println("Kipas Dinyalakan (Auto)");
      previousFanState = true;  // Simpan status kipas sebagai menyala
    } else if (tempC <= temperature_min && previousFanState) {
      relayStates[0] = 0;
      updateShiftRegister();
      Serial.println("Kipas Dimatikan (Auto)");
      previousFanState = false;
    }
  }
  Serial.print("suhu air: ");
  Serial.println(tempC);
  Blynk.virtualWrite(V10, tempC);
}

void updateLCDDisplay() {

  // Tampilan Suhu Air
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(1));
  lcd.print(" AIR : ");
  lcd.print(tempC);
  lcd.write(byte(0));
  lcd.print("C");

  // Tampilan Suhu Ruang
  lcd.setCursor(0, 1);
  lcd.write(byte(1));
  lcd.print(" RUANG : ");
  lcd.print(temp_ruang, 1);
  lcd.write(byte(0));
  lcd.print("C");
  delay(15000);  // Tunggu 15 detik
  lcd.clear();


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(1));
  lcd.print("SET:");
  lcd.print(temperature_min, 1);
  lcd.print("-");
  lcd.print(temperature_max, 1);
  lcd.write(byte(0));
  lcd.print("C");

  // Tampilan FAN MODE
  lcd.setCursor(0, 1);
  lcd.print("FAN MODE: ");
  lcd.print(autoMode_fan ? "Auto" : "Manual");
  delay(5000);  // Tunggu 15 detik
  lcd.clear();
}
