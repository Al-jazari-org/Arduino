
const int leds[] = {3,4,5};
const int touch_pin = 8;
const int led_count = sizeof(leds) / sizeof(int);

void setup(){

    for(int i=0;i<led_count;i++){
        pinMode(leds[i],OUTPUT);
    }
    pinMode(touch_pin,INPUT);



}

int counter = 0;
bool pressed = false;

void loop(){

    if(pressed != true && digitalRead(touch_pin) == HIGH){
        pressed = true;
        counter++;
        if(counter > led_count){
            counter = 0;
        }
    }
    else if(pressed == true && digitalRead(touch_pin) == LOW){
        pressed = false;

    }

    for(int l=0;l<led_count;l++){
        if(l < counter){
            if(l == 0){
                analogWrite(leds[l],30);
            }else{
                digitalWrite(leds[l],HIGH);
            }
        }else{
            digitalWrite(leds[l],LOW);
        }
    }

}
