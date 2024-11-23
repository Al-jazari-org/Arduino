#include "RotaryEncoder.h"


const uint8_t clk_pin = 3;
const uint8_t dt_pin = 2;

const uint8_t led_pin = 5;

uint8_t led_brightness = 0;
uint8_t step = 16;

RotaryEncoder *encoder = NULL;

void checkPosition()
{
    encoder->tick(); // just call tick() to check the state.
    int8_t dir = (int8_t)encoder->getDirection();
    uint8_t delta = (dir == 1) ? (255 - led_brightness) : led_brightness;
    uint8_t steps = (delta / step > 0);
    int16_t adjusted_brightness = led_brightness + steps * dir * step;
    led_brightness = (adjusted_brightness > 255) ? 255 : (adjusted_brightness < 0) ? 0 : adjusted_brightness;
}



void setup(){
    Serial.begin(9600);

    pinMode(led_pin,OUTPUT);

    encoder = new RotaryEncoder(clk_pin, dt_pin, RotaryEncoder::LatchMode::TWO03);

    attachInterrupt(digitalPinToInterrupt(clk_pin),checkPosition,CHANGE);
    attachInterrupt(digitalPinToInterrupt(dt_pin),checkPosition,CHANGE);
};




void loop(){

    analogWrite(led_pin,led_brightness);

};
