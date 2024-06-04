
const int Red_pin = 3;
const int Green_pin = 5;
const int Blue_pin = 6;

void setup(){

    pinMode(Red_pin,OUTPUT);
    pinMode(Green_pin,OUTPUT);
    pinMode(Blue_pin ,OUTPUT);
}

const double time_to_max = 2000.0f;


static double delta = 0.0f;


void loop(){
    static double wave_x = 0.0f ;
    static double interval = (1.0/time_to_max) ;
    double t0 = millis();

    if(wave_x >= 3.000f){
        wave_x = 0.0f;
    }

    wave_x += delta * (interval);

    
    double wave_y = triangle_wave(wave_x); 

    if(wave_x < 1.00f){
        analogWrite(Red_pin,wave_y * 255);
        analogWrite(Green_pin,0);
        analogWrite(Blue_pin,0);
    }
    else if(wave_x < 2.00f){
        analogWrite(Red_pin,0);
        analogWrite(Green_pin,wave_y * 255);
        analogWrite(Blue_pin,0);
    }
    else if(wave_x < 3.00f){
        analogWrite(Red_pin,0);
        analogWrite(Green_pin,0);
        analogWrite(Blue_pin,wave_y * 255);
    }
double t1 = millis();

    delta = t1-t0;

}

double triangle_wave(double x){
    return 2 * abs(x - floor(x + 3.0f/4.0f + 0.75) + 1.0f/4.0f + 0.75);
}

