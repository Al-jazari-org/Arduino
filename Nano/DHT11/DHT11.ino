#include "DHT.h"
#include "LiquidCrystal_I2C.h"

DHT dht(2, DHT11);
LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {
  dht.begin();
  lcd.init();
  lcd.backlight();
}

void loop() {
 
  delay(500);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    return;
  }

  lcd.setCursor(0,0);
  lcd.print(h);
  lcd.print("%");
  lcd.setCursor(0,1);
  lcd.print(t);
  lcd.print("C");

}
