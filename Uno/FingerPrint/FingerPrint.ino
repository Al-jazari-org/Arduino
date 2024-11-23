#include "Adafruit_Fingerprint.h"

#define ENROLL_PIN A5

const uint8_t finger_rx_pin = 2, finger_tx_pin = 3;
const uint8_t green_led_pin = 5, yellow_led_pin = 4;

SoftwareSerial fingerSerial(finger_rx_pin,finger_tx_pin);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

bool enroll = false;


void setup(){

    pinMode(ENROLL_PIN,INPUT_PULLUP);
    pinMode(LED_BUILTIN,OUTPUT);

    pinMode(green_led_pin,OUTPUT);
    pinMode(yellow_led_pin,OUTPUT);

    enroll = !digitalRead(ENROLL_PIN);

    Serial.begin(9600);

    finger.begin(57600);

    if (finger.verifyPassword()) {
        Serial.println("Found fingerprint sensor!");
    } else {
        Serial.println("Did not find fingerprint sensor");
        while (1) { delay(1); }
    }


    if(enroll){
        Serial.println(F("Reading sensor parameters"));
        finger.getParameters();
        Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
        Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
        Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
        Serial.print(F("Security level: ")); Serial.println(finger.security_level);
        Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
        Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
        Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

        digitalWrite(HIGH,LED_BUILTIN);
        finger.LEDcontrol(0);
        delay(250);
        digitalWrite(LOW,LED_BUILTIN);
        finger.LEDcontrol(1);
        delay(250);
        digitalWrite(HIGH,LED_BUILTIN);
        finger.LEDcontrol(0);
        delay(250);
        digitalWrite(LOW,LED_BUILTIN);
        finger.LEDcontrol(1);
        delay(250);
    }


    Serial.flush();
}


void loop(){
if(enroll){
    Enroll();
    }else{
    Match();
    }
}


void Enroll(){
    Serial.println("Ready to enroll a fingerprint!");
    /*
    Serial.print("There Are ");
    Serial.print(finger.getTemplateCount());
    Serial.println(" Templates");
    */
    Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
    Serial.println("Or type -1 to clear the database");

    int8_t id = readnumber();
    id = (id < -1)? 0 : id;
    switch(id){
        case(0):
            Serial.println("error 0 Is an invalid ID");
            break;
        case(-1):
            finger.emptyDatabase();
            break;
        default:{
            Serial.print("Enrolling ID #");
            Serial.println(id);

            while(!EnrollFingerprintID(id));
        }
            break;
    }
}

void Match(){
    uint8_t ret = finger.getImage();
    if(ret == FINGERPRINT_OK){
        bool matched = MatchFingerprint();
        if(matched){
            digitalWrite(green_led_pin,HIGH);
            delay(1000);
            digitalWrite(green_led_pin,LOW);
        }else{
            digitalWrite(yellow_led_pin,HIGH);
            delay(1000);
            digitalWrite(yellow_led_pin,LOW);
        }
    }
    delay(50);
}


int8_t readnumber(void) {
    int8_t num = 0;
    while (num == 0) {
        while (! Serial.available());
        num = Serial.parseInt();
    }
    return num;
}




bool MatchFingerprint() {
  uint8_t p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return 0;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return 0;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return 1;
}


uint8_t EnrollFingerprintID(uint8_t id) {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

