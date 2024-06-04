const int led_pin = 5;
bool led_state = false;
const int mic_pin = 3;

void setup(){

    pinMode(led_pin,OUTPUT);
    pinMode(mic_pin,INPUT);

}

int claps = 0;

void loop(){

    if(digitalRead(mic_pin)){
        static double t_start = 0.0f;
        if(claps == 0){
            t_start = millis();
            claps++;
        }else{
            double t_now = millis();
            if(t_now - t_start  < 750.0f){
                claps ++;
            }else{
                claps = 0;
            }
        }
        if(claps >= 2){
            led_state = !led_state;
            digitalWrite(led_pin,led_state);
            claps = 0;
        }
    delay(25);
    }

    


}
