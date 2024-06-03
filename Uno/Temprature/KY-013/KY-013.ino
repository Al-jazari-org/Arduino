#include "LiquidCrystal_I2C.h"

LiquidCrystal_I2C lcd(0x27,16,2);

const float B = 3435;

const float R0 = 10000;
const float ABS_ZERO = -273.15;
const float T0 = 25-ABS_ZERO;

float calculateTemperature(float resistance) {
    return 1.0 / (log(resistance/R0)/B+1.0/T0) + ABS_ZERO;
}

float convertTemperatureFahrenheit(float tDegree) {
    return tDegree/5.0*9.0 + 32.0;
}

float convertTemperatureC(float tDegree) {
    return tDegree - 273.15;
}
// Fixed resistor in voltage divider
const float R_FIXED = 10000;

float calculateSensorResistance(float voltage) {
    return R_FIXED / (5.0/voltage - 1);
}

void setup() {
    // Init Serial Port
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    
}

void loop() {
    // Measurements and Calculations
    float voltage = analogRead(A0)*5/1024.0;
    float resistance = calculateSensorResistance(voltage);
    float temperature = calculateTemperature(resistance);
    //float temp_c = convertTemperatureC(temperature);

    lcd.setCursor(0,0);
    lcd.print("Termprature:");
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print(temperature);
    lcd.print("C");

    // Wait for 1s
    delay(1000);
}
