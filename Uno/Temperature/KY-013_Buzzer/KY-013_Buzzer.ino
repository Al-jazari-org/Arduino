

const float B = 3435;

const float R0 = 10000;
const float ABS_ZERO = -273.15;
const float T0 = 25-ABS_ZERO;

const int led_pin = 5;
const int buzzer_pin = 6;
const int temp_pin = A0;

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

double t_start;
void setup() {
    pinMode(led_pin,OUTPUT);
    pinMode(buzzer_pin,OUTPUT);
    pinMode(temp_pin,INPUT);
     
}
float voltage ;
float resistance ;
float temperature ;
float temp_threshold = 50.0f;
void loop() {
    double t_start = millis();
    static double t_delta = 0.0f;
    static double t0 = 1000.0f;
    static double t_buzzer = 0.0f;
    static bool buzzer_off_on = true;
    t0 += t_delta;
    // Wait for 1s
    if(t0>=500.0f){
        // Measurements and Calculations
        voltage = analogRead(A0)*5/1024.0;
        resistance = calculateSensorResistance(voltage);
        temperature = calculateTemperature(resistance);
        t0 = 0.0f;
    }
    if(temperature >= temp_threshold){
        t_buzzer += t_delta;
        if(t_buzzer >= 1000.0f){
            buzzer_off_on = !buzzer_off_on;
            t_buzzer = 0.0f;
        }
        if(buzzer_off_on){
            analogWrite(buzzer_pin,255);
        }else{
            analogWrite(buzzer_pin,0);
        }

    
        digitalWrite(led_pin,HIGH);
    }else{
        t_buzzer = 0.0f;
        buzzer_off_on = true;
        analogWrite(buzzer_pin,0);
        digitalWrite(led_pin,LOW);
    }

    double t_end = millis();
    t_delta = t_end - t_start;
}
