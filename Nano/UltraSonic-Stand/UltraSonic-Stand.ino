#include <Arduino.h>
#include <Wire.h>
#include "lcd_i2c.h"


#define R_PIN 5
#define Y_PIN 6
#define G_PIN 7

#define BUZZ_PIN 3

#define ECHO_PIN 2
#define TRIG_PIN 4




volatile bool new_pulse = false;
volatile unsigned long pulse_length = 0;

ISR(echoed){
    volatile static unsigned long t0;
    volatile static unsigned long t1;
    if(PIND & (1 << PIND2) ){
        new_pulse = false;
        t0 = micros();
    }else{
        t1 = micros();
        pulse_length = t1 - t0;
        if ((pulse_length < 150 || pulse_length > 25000)){
            new_pulse = false;
        }else{
            new_pulse = true;
        }
    }

};

void setup(){
    Serial.begin(115200);
    pinMode(R_PIN,OUTPUT);
    pinMode(Y_PIN,OUTPUT);
    pinMode(G_PIN,OUTPUT);

    pinMode(BUZZ_PIN,OUTPUT);

    pinMode(ECHO_PIN,INPUT);
    pinMode(TRIG_PIN,OUTPUT);

    digitalWrite(R_PIN,HIGH);
    digitalWrite(Y_PIN,HIGH);
    digitalWrite(G_PIN,HIGH);

    attachInterrupt(digitalPinToInterrupt(ECHO_PIN), echoed, CHANGE);

    lcd_init();
    lcd_set_cursor(0, 0);
    lcd_print("Distance cm:");
    lcd_set_cursor(0, 1);

};


unsigned long last_ping = 0;
unsigned long last_write = 0;
bool ping = true;


void loop(){

    unsigned long last_ping_d = (millis() - last_ping);
    if(ping && last_ping_d > 60){
        PORTD = PORTD & ~(1 << TRIG_PIN);
        delayMicroseconds(2);
        PORTD = PORTD | (1 << TRIG_PIN);
        delayMicroseconds(10);
        PORTD = PORTD & ~(1 << TRIG_PIN);
        last_ping = millis();
    };

    if(!new_pulse){
        return;
    };
    new_pulse = false;

    unsigned long cm = pulse_length/58;

    if(millis()-last_write >= 450){
        lcd_set_cursor(0, 1);
        lcd_print("                ");
        lcd_set_cursor(0, 1);
        static char buf[4];
        utoa(cm,buf,10);
        lcd_print(buf);
        last_write = millis();
    };

    if(cm > 20){
    digitalWrite(R_PIN,LOW);
    digitalWrite(Y_PIN,LOW);
    digitalWrite(G_PIN,HIGH);
    noTone(BUZZ_PIN);


    }
    else if(cm > 5){
    digitalWrite(R_PIN,LOW);
    digitalWrite(Y_PIN,HIGH);
    digitalWrite(G_PIN,LOW);
    digitalWrite(BUZZ_PIN,LOW);
    noTone(BUZZ_PIN);


    }
    else{
    digitalWrite(R_PIN,HIGH);
    digitalWrite(Y_PIN,LOW);
    digitalWrite(G_PIN,LOW);
    tone(BUZZ_PIN,1000);


    }


}
