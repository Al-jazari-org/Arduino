#include <MQUnifiedsensor.h>
#include <LiquidCrystal_I2C.h>

MQUnifiedsensor MQ2("Arduino UNO", 5, 10, A2, "MQ-2");
LiquidCrystal_I2C lcd(0x27, 16, 2);


void setup() {
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

  MQ2.setRegressionMethod(1);
  MQ2.setA(574.25);
  MQ2.setB(-2.222);
  MQ2.init();

  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ2.update();  // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ2.calibrate(9.83);
  }
  MQ2.setR0(calcR0 / 10);
}

void loop() {
  MQ2.update();
  float ppm = MQ2.readSensor();
  if (ppm > 10) {
    lcd.setCursor(0, 1);
    lcd.print("GAS GPL DETECTED");
    digitalWrite(2, HIGH);
    tone(3,500,300);
  } else {
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
  }
  lcd.setCursor(0, 0);
  lcd.print(ppm);
  lcd.print("PPM");
  delay(500);
  lcd.clear();
}
