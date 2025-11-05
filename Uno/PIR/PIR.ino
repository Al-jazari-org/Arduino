#include <LiquidCrystal_I2C.h>

int ledPin = 5;
int inputPin = 3;
int pirState = LOW;
int val = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);


void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(inputPin, INPUT);
  lcd.init();
  lcd.backlight();
}

void loop() {
  val = digitalRead(inputPin);

  if (val == HIGH) {
    digitalWrite(ledPin, HIGH);
    if (pirState == LOW) {
      lcd.clear();
      lcd.print("Motion detected!");
      pirState = HIGH;
    }
  }

  else {
    digitalWrite(ledPin, LOW);
    if (pirState == HIGH) {
      lcd.clear();
      lcd.print("Motion ended!");
      pirState = LOW;
    }
  }
}
