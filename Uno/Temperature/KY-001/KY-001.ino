
#include "OneWire.h"
#include "LiquidCrystal_I2C.h"
#include "DallasTemperature.h"

const int one_wire_bus = 2;

OneWire oneWire(one_wire_bus);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {

    sensors.begin();
    lcd.init();
    lcd.backlight();
    
}

void loop() {
    sensors.requestTemperatures();
    

    lcd.setCursor(0,0);
    lcd.print("Temperature:");
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print(sensors.getTempCByIndex(0));
    lcd.print("C");

    // Wait for 1s
    delay(1000);
}
