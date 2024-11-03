#include "NewPing.h"
const uint8_t green = 9;
const uint8_t yellow = 10;
const uint8_t red = 11;


const uint8_t trig = 6;
const uint8_t echo = 5;


NewPing sonar(trig,echo,60);

void setup() {
  Serial.begin(115200);
  pinMode(green, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(red, OUTPUT);

  pinMode(trig,OUTPUT);
  digitalWrite(trig,LOW);
  pinMode(echo,INPUT);

}




void loop() {
  float dist = sonar.ping_cm();
  if(dist == 0)dist = 60;
  if(dist < 10){
    digitalWrite(green,LOW);
  digitalWrite(yellow, LOW);
    digitalWrite(red,HIGH);
  }else if(dist < 20){
    digitalWrite(red, LOW);
    ditalWrite(green,LOW);
    digitalWrite(yellow,HIGH);
  }else{
    digitalWrite(red, LOW);
  digitalWrite(yellow, LOW);
    digitalWrite(green,HIGH);
  }
  Serial.println(dist);
}



float Ping(){
  const static double speedCmPerUs = 34300/1e6;
  static double t;
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);  
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  t = pulseIn(echo,HIGH,100*1e3);
  

 
  return t*speedCmPerUs/2;
}