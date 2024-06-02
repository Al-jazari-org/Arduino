
const int  led_pin = 6;
const int signal_pin = 2;

void setup(){
    
    pinMode(led_pin,OUTPUT);
    pinMode(signal_pin,INPUT);

}

void loop(){

    if(!digitalRead(signal_pin)){
        digitalWrite(led_pin,HIGH);
    }else{
        digitalWrite(led_pin,LOW);
    }
}

