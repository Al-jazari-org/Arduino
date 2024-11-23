/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-motion-sensor
 */

const int PIN_TO_SENSOR = 2;   // the pin that OUTPUT pin of sensor is connected to
int pinStateCurrent   = LOW; // current state of pin
int pinStatePrevious  = LOW; // previous state of pin
const int LED_PIN = 3;

void setup() {
    Serial.begin(9600);            // initialize serial
    pinMode(PIN_TO_SENSOR, INPUT); // set arduino pin to input mode to read value from OUTPUT pin of sensor
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN,LOW);
}

void loop() {
    pinStatePrevious = pinStateCurrent; // store old state
    pinStateCurrent = digitalRead(PIN_TO_SENSOR);   // read new state

    if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {   // pin state change: LOW -> HIGH
        Serial.println("Motion detected!");
        digitalWrite(LED_PIN,HIGH);
    }
    else
    if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {   // pin state change: HIGH -> LOW
        Serial.println("Motion stopped!");
        digitalWrite(LED_PIN,LOW);
    }
}
