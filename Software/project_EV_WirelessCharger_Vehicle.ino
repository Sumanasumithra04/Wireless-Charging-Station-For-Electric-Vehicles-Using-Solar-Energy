#define BLYNK_TEMPLATE_ID "TMPL36-hz8L_c"
#define BLYNK_TEMPLATE_NAME "EV Wireless Charger"
#define BLYNK_AUTH_TOKEN "OvDHMA30Cf67bR5GzTlRsA5Ru6aCL4UL"

// Libraries:
#include <WiFi.h>
/* Add JSON Package https://dl.espressif.com/dl/package_esp32_index.json
  and Install ESP32 Board */

#include <BlynkSimpleEsp32.h>
/* Install: Blynk by Volodymyr Shymanskyy */

#include<LiquidCrystal_I2C.h>
/* Add Zip: https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library */

#include <DallasTemperature.h>
/* Install: "DallasTemperature" by Miles Burton */

#include <OneWire.h>
/* Install: "OneWire" by Paul Stoffregen */

// Pin Numbers:
#define OneWireBus_Pin 18
#define Voltage_Pin 35  // (Input) Connect to AO/out pin.
#define Flame_Pin 32    // (Input) Connect to DO/out pin.
#define Relay_Pin 23    // (Output) Connect to the IN pin of Relay Module.
#define Buzzer_Pin 33   // (Output) Connect to the I/O pin of Buzzer Module.
#define OnBoardLED_Pin 2  // Arduino Pin 13, ESP Pin 2, ESP-12 Pin 2, NodeMCU Pin 16

// Pin Numbers: Blynk Virtual Pins
#define VoltValue_vPin V0
#define VoltPct_vPin V1
#define TempValue_vPin V2

// Configuration: LCD Display
const uint8_t I2C_Addr = 0x27; // I2C Address
const uint8_t lcdNumCols = 16; // LCD's number of columns
const uint8_t lcdNumRows = 2;  // LCD's number of rows

// Blynk Project Authentication Key:
const char auth[] = BLYNK_AUTH_TOKEN;

// WiFi Credentials:
const char ssid[] = "Galaxy A3181A4";  // Name of your network (Hotspot or Router Name)
const char pass[] = "keerthana07";  // Corresponding Password

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(OneWireBus_Pin);

// Pass oneWire reference to Dallas Temperature Library
DallasTemperature objDT(&oneWire);

// Objects:
LiquidCrystal_I2C lcd(I2C_Addr, lcdNumCols, lcdNumRows);

// Variables:
int Analog_Value;
float Calib = 0.0049;
int Voltage_Percentage;
float Voltage_Value;
int Flame_State;
int Temperature_Value;

unsigned long prevFlameNotifyTime;
unsigned long prevTempNotifyTime;

void setup() {
  // Pin Mode Configuration:
  pinMode(Flame_Pin, INPUT);
  pinMode(Relay_Pin, OUTPUT);
  digitalWrite(Relay_Pin, LOW);
  pinMode(Buzzer_Pin, OUTPUT);
  digitalWrite(Buzzer_Pin, LOW);

  pinMode(OnBoardLED_Pin, OUTPUT);
  digitalWrite(OnBoardLED_Pin, LOW);


  /* Begin serial communication with Arduino and Arduino IDE (Serial Monitor) */
  Serial.begin(9600);

  // Initialise the LCD display:
  lcd.begin();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Connecting to    ");
  lcd.setCursor(0, 1);
  lcd.print("Blynk            ");
  /* Begin communication with Blynk App */
  Blynk.begin(auth, ssid, pass);
  lcd.clear();
  lcd.print("Connected!       ");
  delay(2000);
  digitalWrite(OnBoardLED_Pin, HIGH);
}

void loop() {
  Blynk.run();

  Analog_Value = analogRead(Voltage_Pin);
  Voltage_Value = Analog_Value * Calib;
  Voltage_Percentage = (Voltage_Value - 3) * 83.333;
  Serial.print("Analog Value : ");
  Serial.println(Analog_Value);
  Serial.print("Voltage Value: ");
  Serial.println(Voltage_Value);

  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(Voltage_Value);
  lcd.print("V,");
  lcd.setCursor(7, 0);
  lcd.print(Voltage_Percentage);
  lcd.print("%");

  objDT.requestTemperatures();
  Temperature_Value = objDT.getTempCByIndex(0);
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(Temperature_Value);
  lcd.print((char)223); // Prints degree symbol
  lcd.print("C   ");

  Flame_State = digitalRead(Flame_Pin);
  Serial.print("Flame State  : ");
  Serial.print(Flame_State);

  if (Flame_State == 0) {
    Serial.print(": Flame Detected!");
    digitalWrite(Relay_Pin, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Flame Detected! ");
    if (millis() - prevFlameNotifyTime > 60000) {  // 1000ms = 1sec
      Blynk.logEvent("flame_event");
      prevFlameNotifyTime = millis();
    }
    Buzzer(6, 200, 200);
  } else if (Temperature_Value > 50) {
    Serial.print(": High Temperature!");
    digitalWrite(Relay_Pin, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("High Temperature");
    if (millis() - prevTempNotifyTime > 60000) {  // 1000ms = 1sec
      Blynk.logEvent("temperature_event");
      prevTempNotifyTime = millis();
    }
    Buzzer(3, 500, 500);
  } else {
    digitalWrite(Relay_Pin, LOW);
  }
  Serial.println();
  Serial.println();

  Blynk.virtualWrite(VoltValue_vPin, Voltage_Value);
  Blynk.virtualWrite(VoltPct_vPin, Voltage_Percentage);
  Blynk.virtualWrite(TempValue_vPin, Temperature_Value);
  delay(200);
}

void Buzzer(int n, int onDelay, int offDelay) {
  for (int i = 1; i <= n; i++) {
    digitalWrite(Buzzer_Pin, HIGH);
    delay(onDelay);
    digitalWrite(Buzzer_Pin, LOW);
    delay(offDelay);
  }
}
