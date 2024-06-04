const int buzzer_pin = 5;
const int button_pin = 3;

void setup(){
    pinMode(button_pin,INPUT);
    pinMode(buzzer_pin,OUTPUT);
}

void loop(){
    if(digitalRead(button_pin)){
        digitalWrite(buzzer_pin,HIGH);
    }else{
        digitalWrite(buzzer_pin,LOW);
    }
}
