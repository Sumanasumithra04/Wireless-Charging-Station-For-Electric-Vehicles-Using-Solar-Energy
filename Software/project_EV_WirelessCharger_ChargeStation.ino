// Libraries:
#include <Keypad.h>
/* Install: Keypad by Mark Stanley, Alexander Brevig */

#include<LiquidCrystal_I2C.h>
/* Add Zip: https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library */

// Pin Numbers: Relay Module
#define Relay_Pin 10    // (Output) Connect to the IN pin of Relay Module.

// Pin Numbers: Buzzer Module
#define Buzzer_Pin 11   // (Output) Connect to the I/O pin of Buzzer Module.

// Pin Numbers: LED
#define RLED_Pin A1
#define GLED_Pin A0

// Constants:
const uint8_t I2C_Addr = 0x27; // I2C Address: 0x3F or 0x27
const uint8_t lcdNumCols = 16; // LCD's number of columns
const uint8_t lcdNumRows = 2;  // LCD's number of rows

const byte kpNumRows = 4;  // Keypad's number of rows
const byte kpNumCols = 4;  // Keypad's number of columns

// Variables:
char hexaKeys[kpNumRows][kpNumCols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
char keyChar;

// Keypad Pin Numbers:
byte rowPins[kpNumRows] = {9, 8, 7, 6};  //R1, R2, R3, R4
byte colPins[kpNumCols] = {5, 4, 3, 2};  //C1, C2, C3, C4

// Objects:
LiquidCrystal_I2C lcd(I2C_Addr, lcdNumCols, lcdNumRows);
Keypad kp = Keypad(makeKeymap(hexaKeys), rowPins, colPins, kpNumRows, kpNumCols);

// Variables:
int durMin = 0;

void setup() {
  // Pin Mode Configuration:
  pinMode(Relay_Pin, OUTPUT);

  pinMode(Buzzer_Pin, OUTPUT);
  digitalWrite(Buzzer_Pin, LOW);

  pinMode(RLED_Pin, OUTPUT);
  pinMode(GLED_Pin, OUTPUT);
  digitalWrite(RLED_Pin, LOW);
  digitalWrite(GLED_Pin, HIGH);

  /* Begin serial communication with Arduino and Arduino IDE (Serial Monitor) */
  Serial.begin(9600);

  // Initialise the LCD display:
  lcd.begin();
  lcd.backlight();
}

void loop() {
  durMin = KeypadInput_IntRange(1, 10);
  lcd.setCursor(0, 0);
  lcd.print("Duration:       ");
  lcd.setCursor(10, 0);
  lcd.print(durMin);
  lcd.print("min");
  Buzzer(1, 1000, 0);
  delay(1000);

  digitalWrite(Relay_Pin, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Timer:          ");
  lcd.setCursor(0, 1);
  lcd.print("Charging...     ");
  lcd.setCursor(7, 0);
  Serial.println("RED");
  digitalWrite(RLED_Pin, HIGH);
  digitalWrite(GLED_Pin, LOW);

  WaitingCountDownTimer(durMin * 60000L);
  digitalWrite(Relay_Pin, LOW);
  lcd.setCursor(0, 1);
  lcd.print("Charging Stopped");
  Serial.println("GREEN");
  digitalWrite(RLED_Pin, LOW);
  digitalWrite(GLED_Pin, HIGH);
  Buzzer(3, 300, 300);
  delay(2000);
}

int KeypadInput_IntRange(int min, int max) {
  lcd.clear();
  lcd.setCursor(0, 1);  //(colIdx, rowIdx)
  lcd.print("Start,   C:Clear");

  String keyStr = "";
  char keyChar = '\0';
  int value;

  bool clearInput = true;
  while (true) {
    if (clearInput || keyStr.length() > 2) {
      keyStr = "";
      lcd.setCursor(0, 0);
      lcd.print("Dur. (min):     ");
      lcd.setCursor(12, 0);
      lcd.blink();
      clearInput = false;
    }

    keyChar = kp.getKey();
    if (keyChar == '*') {
      value = keyStr.toInt();
      if (value >= min && value <= max) {
        break;
      } else {
        clearInput = true;
      }
    } else if (keyChar == 'C') {
      clearInput = true;
    } else if (keyChar) {
      lcd.print(keyChar);
      keyStr += keyChar;
    }
  }
  lcd.noBlink();
  lcd.clear();
  return value;
}

void WaitingCountDownTimer(unsigned long waitingDuration) {
  unsigned long millisB4Entry;
  unsigned long elapsedTime;
  long remainingTime;

  millisB4Entry = millis();
  do {
    elapsedTime = millis() - millisB4Entry;
    remainingTime = waitingDuration - elapsedTime;
    lcd.setCursor(7, 0);
    lcd.print(Millis2TimerMMSS(remainingTime));
  } while (remainingTime > 0);
}

String Millis2TimerMMSS(long duration) {
  int min, sec;
  String str = "";

  if (duration > 0) {
    duration /= 1000;       // Converting millisec to sec.
    min = duration / 60;    // Converting sec to min.
    sec = duration % 60;    // Calculating remaining sec (remainder).

    if (min < 10)
      str += "0";
    str += String(min) + ":";

    if (sec < 10)
      str += "0";
    str += String(sec);
  } else {
    str = "00:00";
  }
  return str;
}

void Buzzer(int n, int onDelay, int offDelay) {
  for (int i = 1; i <= n; i++) {
    digitalWrite(Buzzer_Pin, HIGH);
    delay(onDelay);
    digitalWrite(Buzzer_Pin, LOW);
    delay(offDelay);
  }
}
