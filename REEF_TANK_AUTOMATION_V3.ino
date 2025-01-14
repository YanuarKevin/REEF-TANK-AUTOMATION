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

bool relayStates[NUM_CHANNELS] = { false, false, false, false }; 
bool notificationSentDHT = false;                               
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
unsigned long displayDurations[] = {2000, 5000, 7000, 10000};
unsigned int sensorIndex = 0;
unsigned long previousMillis = 0;

float tempC = 0.0;
int salinitasair = 0;
float temp_ruang = 0.0;
int hum_ruang = 0;
float temperature_max = 30.0;
float temperature_min = 27.0;
int salinitas_max = 1026;
int salinitas_min = 1024;

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
  B00100,
  B00100,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110,
  B00000 
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
  pinMode(SALINITAS_PIN, INPUT);

  dht.begin();
  sensors.begin();

  for (int i = 0; i < NUM_CHANNELS; i++) {
    relayStates[i] = true;
  }

  updateShiftRegister();
  servo.attach(SERVO_PIN);

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
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    readDHT();
    readDS18B20();
    readSalinity();
  }
  updateLCDDisplay();
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
    Blynk.virtualWrite(V2, 1);
    autoMode_fan = true;
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
  int pinValue = param.asInt();
  if (pinValue == 1 || pinValue == 0) {
    relayStates[0] = pinValue;
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
    for (int pos = 180; pos >= 0; pos--) { 
      servo.write(pos);
      // delay(1);
    }
    servo.write(180); 
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
  temperature_max = param.asFloat();
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
  salinitas_max = param.asInt();
  Serial.print("SETTING SALINITAS MAX: ");
  Serial.println(salinitas_max);
  updateTemperatureSettings(salinitas_min, salinitas_max);
}

BLYNK_WRITE(V9) {
  salinitas_min = param.asInt();
  Serial.print("SETTING SALINITAS MIN: ");
  Serial.println(salinitas_min);
  updateSalinitasSettings(salinitas_min, salinitas_max);
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

void updateSalinitasSettings(float minSal, float maxSal) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SALINITAS UPDATE");
  lcd.setCursor(4, 1);
  lcd.print(minSal);
  lcd.print(" - ");
  lcd.print(maxSal);
  delay(3000);
  lcd.clear();
}

void readDHT() {
  float hum_ruang = dht.readHumidity();
  float temp_ruang = dht.readTemperature();

  if (temp_ruang >= 33 && !notificationSentDHT) {
    Blynk.notify("Suhu Ruangan Tinggi !!!");
    Blynk.email("yanuarkevinbwi31@gmail.com", "SMART REEF TANK", "Suhu Ruangan Sangat Tinggi !!!");
    notificationSentDHT = true;  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SUHU DI RUANGAN");
    lcd.setCursor(0, 1);
    lcd.print("SANGAT TINGGI !!");
    delay(4000);
    lcd.clear();
  } else if (temp_ruang <= 30.5) {
    notificationSentDHT = false;
  }
  Serial.print("suhu ruang: ");
  Serial.print(temp_ruang);
  Serial.println("°C"); 
  Blynk.virtualWrite(V11, temp_ruang);
  Blynk.virtualWrite(V12, hum_ruang);
}

void readDS18B20() {
  sensors.requestTemperatures();    
  float tempC = sensors.getTempCByIndex(0);

  if (tempC >= temperature_max && !notificationSentDS18B20) {
    // Blynk.virtualWrite(V2, 0);
    Serial.println("Suhu Air Melebihi Batas !!!");
    Blynk.notify("Suhu Air Melebihi Batas !!!");  
    Blynk.email("yanuarkevinbwi31@gmail.com", "SMART REEF TANK", "Suhu Air Melebihi Batas !!!");
    notificationSentDS18B20 = true;
    lcd.clear();                     
    lcd.setCursor(0, 0);
    lcd.print("SUHU PADA AIR");  
    lcd.setCursor(0, 1);
    lcd.print("MELEBIHI BATAS !!");  
    delay(4000);                     
    lcd.clear();                     
  } else if (tempC <= temperature_min) {
    notificationSentDS18B20 = false;
  }

  if (autoMode_fan) {
    if (tempC >= temperature_max && !previousFanState) {
      relayStates[0] = 0;  // Nyalakan relay (relay 0) jika suhu lebih dari temperature_max
      updateShiftRegister();
      Serial.println("Kipas Dinyalakan (Auto)");
      previousFanState = true;  // Simpan status kipas sebagai menyala
    } else if (tempC <= temperature_min && previousFanState) {
      relayStates[0] = 1;
      updateShiftRegister();
      Serial.println("Kipas Dimatikan (Auto)");
      previousFanState = false;
    }
  }
  Serial.print("suhu air: ");
  Serial.print(tempC);
  Serial.println("°C");  
  Blynk.virtualWrite(V10, tempC);
}

void readSalinity() {
  int salinitasair = analogRead(SALINITAS_PIN);

  if (salinitasair > salinitas_max && !notificationSentSalinitasup) {
    Serial.println("Salinitas Air Tinggi !!!");
    Blynk.notify("Salinitas Air Tinggi !!!");  
    Blynk.email("yanuarkevinbwi31@gmail.com", "SMART REEF TANK", "Salinitas Air Tinggi !!!");
    notificationSentSalinitasup = true;
    notificationSentSalinitasdown = false;
  }
  if (salinitasair < salinitas_min && !notificationSentSalinitasdown) {
    Serial.println("Salinitas Air Rendah !!!");
    Blynk.notify("Salinitas Air Rendah !!!");  
    Blynk.email("yanuarkevinbwi31@gmail.com", "SMART REEF TANK", "Salinitas Air Rendah !!!");
    notificationSentSalinitasdown = true;
    notificationSentSalinitasup = false;
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

void updateLCDDisplay() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= displayDurations[sensorIndex]) {
    previousMillis = currentMillis;
    sensorIndex = (sensorIndex + 1) % 4;
    lcd.clear();
  }

  switch (sensorIndex) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.write(byte(1));
      lcd.print(" AIR : ");
      lcd.print(tempC);
      lcd.write(byte(0));
      lcd.print("C");

      lcd.setCursor(0, 1);
      lcd.write(byte(2));
      lcd.print(" S.G : ");
      lcd.print(salinitasair);
      break;
    case 1:
      lcd.setCursor(0, 0);
      lcd.write(byte(1));
      lcd.print(" RUANG : ");
      lcd.print(temp_ruang, 1);
      lcd.write(byte(0));
      lcd.print("C");

      lcd.setCursor(1, 1);
      lcd.write(byte(2));
      lcd.print(" RUANG : ");
      lcd.print(hum_ruang);
      lcd.print("%");
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("S.G: ");
      lcd.print(salinitas_min);
      lcd.print(" - ");
      lcd.print(salinitas_max);

      lcd.setCursor(0, 1);
      lcd.write(byte(1));
      lcd.print(" AIR: ");
      lcd.print(temperature_min, 1);
      lcd.print("-");
      lcd.print(temperature_min, 1);
      break;
    case 3:
      lcd.setCursor(4, 0);
      lcd.print("FAN MODE :");
      lcd.setCursor(6, 1);
      lcd.print(autoMode_fan ? "Auto" : "Manual");
      break;
  }
}
