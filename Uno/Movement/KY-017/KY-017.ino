
const int led_pin = 5;
const int tilt_pin = 3;


void setup(){
    pinMode(led_pin,OUTPUT);
    pinMode(tilt_pin,INPUT);
}
void loop(){
    if(digitalRead(tilt_pin)){
        digitalWrite(led_pin,HIGH);
    }else{
        digitalWrite(led_pin,LOW);
    }
}
