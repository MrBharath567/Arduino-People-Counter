#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int ir1 = 2;
const int ir2 = 3;

const int emptyPin = 4;
const int occupiedPin = 5;

const int switchPin = 8;

const int ldrPin = A0;   // LDR module analog pin

#define DHTPIN 6
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

int peopleCount = 0;

int prevIR1 = HIGH;
int prevIR2 = HIGH;

bool ir1Triggered = false;
bool ir2Triggered = false;

void setup() {

  pinMode(ir1, INPUT);
  pinMode(ir2, INPUT);

  pinMode(emptyPin, OUTPUT);
  pinMode(occupiedPin, OUTPUT);

  pinMode(switchPin, INPUT);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  dht.begin();

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("System OFF");
}

void loop() {

  // -------- CHECK SWITCH ----------
  if (digitalRead(switchPin) == LOW) {

    digitalWrite(emptyPin, LOW);
    digitalWrite(occupiedPin, LOW);

    lcd.setCursor(0,0);
    lcd.print("System OFF     ");
    lcd.setCursor(0,1);
    lcd.print("Switch to Start");

    return;
  }

  int ir1State = digitalRead(ir1);
  int ir2State = digitalRead(ir2);

  // ENTRY START
  if (prevIR1 == HIGH && ir1State == LOW) {
    ir1Triggered = true;
  }

  // ENTRY CONFIRM
  if (ir1Triggered && prevIR2 == HIGH && ir2State == LOW) {
    peopleCount++;
    ir1Triggered = false;
    ir2Triggered = false;
    updateLCD();
    updatePins();
    delay(300);
  }

  // EXIT START
  if (prevIR2 == HIGH && ir2State == LOW) {
    ir2Triggered = true;
  }

  // EXIT CONFIRM
  if (ir2Triggered && prevIR1 == HIGH && ir1State == LOW) {
    peopleCount--;
    if (peopleCount < 0) peopleCount = 0;
    ir1Triggered = false;
    ir2Triggered = false;
    updateLCD();
    updatePins();
    delay(300);
  }

  if (ir1State == HIGH && ir2State == HIGH) {
    ir1Triggered = false;
    ir2Triggered = false;
  }

  prevIR1 = ir1State;
  prevIR2 = ir2State;

  static unsigned long lastTemp = 0;
  if (millis() - lastTemp >= 1000) {
    lastTemp = millis();
    updateLCD();
    updatePins();
  }
}

// LCD UPDATE
void updateLCD() {

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Count=");
  lcd.print(peopleCount);

  lcd.setCursor(0,1);
  lcd.print("T:");

  if (isnan(t)) lcd.print("--");
  else lcd.print((int)t);

  lcd.print("C H:");

  if (isnan(h)) lcd.print("--");
  else lcd.print((int)h);

  lcd.print("%");
}

// LED UPDATE WITH LDR
void updatePins() {

  int ldrValue = analogRead(ldrPin);
  Serial.println(ldrValue);

  if (peopleCount == 0) {
    digitalWrite(emptyPin, LOW);
    digitalWrite(occupiedPin, LOW);
  }

  else {

    if (ldrValue < 700) {        // DARK ROOM
      digitalWrite(emptyPin, LOW);
      digitalWrite(occupiedPin, LOW);
    }

    else if (ldrValue >= 700 && ldrValue < 800) {   // MEDIUM LIGHT
      digitalWrite(emptyPin, HIGH);
      digitalWrite(occupiedPin, LOW);
    }

    else {                       // BRIGHT ROOM
      digitalWrite(emptyPin, HIGH);
      digitalWrite(occupiedPin, HIGH);
    }

  }
}