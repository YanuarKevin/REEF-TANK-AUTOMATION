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
char ssid[] = "OKE_JON";
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

unsigned long previousMillis = 0;
const long interval = 1000;
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
}

void loop() {
  Blynk.run();
  unsigned long currentMillis = millis(); 

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    readDHT();
    readDS18B20();
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
    Blynk.email("yanuarkevinbwi31@gmail.com", "SMART REEF TANK (BY YK REEF STORE)", "IKAN SUDAH DIBERI PAKAN !!!");
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
    Blynk.email("yanuarkevinbwi31@gmail.com", "SMART REEF TANK (BY YK REEF STORE)", "SUHU RUANGAN SANGAT TINGGI !!!");
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
  sensors.requestTemperatures();            
  float tempC = sensors.getTempCByIndex(0); 

  if (tempC >= temperature_max && !notificationSentDS18B20) {
    // Blynk.virtualWrite(V2, 0); 
    Serial.println("Suhu Air Melebihi Batas !!!");
    Blynk.notify("Suhu Air Melebihi Batas !!!");  
    Blynk.email("yanuarkevinbwi31@gmail.com", "SMART REEF TANK (BY YK REEF STORE)", "SUHU AIR MELEBIHI BATAS  !!!");
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
      relayStates[0] = 0;
      updateShiftRegister();
      Blynk.virtualWrite(V2, 0);
      Serial.println("Kipas Dinyalakan (Auto)");
      previousFanState = true;
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
