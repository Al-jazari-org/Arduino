#include <Arduino.h>
#line 1 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
#include "Adafruit_Fingerprint.h"
#include <HardwareSerial.h>
#include <Preferences.h>
#include "common.h"
#include "as608.h"
#include "buffer.h"

#define UART_BUF_SIZE 4096
#define IMAGE_SIZE 73728

void handleClearDB(AsyncWebServerRequest* request);
void handleMode(AsyncWebServerRequest *request);
void handleAdminData(AsyncWebServerRequest *request);


enum FP_MODE {
  FP_CAPTURE,
  FP_ENROLL,
  FP_MATCH,
  FP_HQ_ENROLL,
};

enum FP_RET : int32_t{
  FP_RET_TIMEOUT = -2,
  FP_RET_ERR = -1,
  FP_RET_SUCCESS = 1,
  FP_RET_BUSY = 0,
};

struct LEDDriverConf {
  uint8_t pin = 0;
  unsigned long t_abs = 0;
  unsigned long t_0 = 0;
  bool on = false;

  LEDDriverConf(uint8_t led_pin){
    pin = led_pin;
    pinMode(pin,OUTPUT);
    digitalWrite(pin,LOW);
  };

  void setState(bool state){
    on = state;
    digitalWrite(pin,on);
  };


  void turnOffAfter(unsigned long ms){
    setState(true);
    t_abs = millis();
    t_0 = ms;
  };
  void turnOnAfter(unsigned long ms){
    setState(false);
    t_abs = millis();
    t_0 = ms;
  };
  void reset(){
    t_0 = 0;
    t_abs = 0;

  }
  void drive(){
    if(t_0 == 0 && t_abs == 0)
      return;
    if(t_abs + t_0 <= millis()){
      reset();
      setState(!on);
    };
  };
};

size_t sendFingerprintImageChunk(uint8_t *buffer, size_t maxLen, size_t actualLen);
void dump_byte_array(byte *buffer, size_t bufferSize);
int ChangeFPBaud(Adafruit_Fingerprint& device,uint8_t mul_of_9600);
bool FPStoreChar(uint8_t src, uint8_t dest);
bool FPRegModel();
bool FPGenChar(uint8_t id);
FP_RET FPCapAndReg(bool reset= false);

uint8_t *image_buf = NULL;
bool image_valid= false;
//res :256x288
const size_t image_buf_size = IMAGE_SIZE;
size_t image_buf_count = 0;
RAMBuffer<40000> uart_buf;

HardwareSerial fp_serial(2);
Adafruit_Fingerprint fp = Adafruit_Fingerprint(&fp_serial);

const uint8_t touch_pin = 25;

struct FPSettings{
  uint8_t baudrate = 113;
  uint16_t capacity = 128;
  uint16_t template_count = 0;
}fp_settings;

AS608_BasicPTable fp_table;

bool stop = false;
bool downloading = false;
bool download_failed = false;
bool upload_image = true;

#line 106 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
void OnReceiveCall();
#line 122 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
void setup();
#line 285 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
void printHexByte(byte value);
#line 300 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
void loop();
#line 469 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
void handleImage(AsyncWebServerRequest *request);
#line 502 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
void handleImageValidity(AsyncWebServerRequest *request);
#line 513 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
bool IsFingerFound();
#line 604 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
bool FPCapture();
#line 660 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
void DownloadINFPage(uint8_t* buffer, size_t size);
#line 753 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
void DownloadImage();
#line 892 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
bool FPSearchChar(uint8_t src_buf, uint16_t start_page, uint16_t page_count, uint16_t* score, uint16_t* page_idx);
#line 918 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
bool FPHiSearchChar(uint8_t src_buf, uint16_t start_page, uint16_t page_count, uint16_t* score, uint16_t* page_idx);
#line 944 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
FP_RET FPCapAndGen(uint8_t dest_char);
#line 968 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
FP_RET FPEmptyDB();
#line 978 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
FP_RET FPCapAndReg(bool reset);
#line 1020 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
FP_RET FPChangeMode(FP_MODE new_mode);
#line 1029 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
void DisableImageUploading();
#line 1032 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
void EnableImageUploading();
#line 106 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/FP-Stand.ino"
void OnReceiveCall(){
  /*
  static char txt_buf[32] = {0};
  static unsigned long t0 = 0;
  static unsigned long t1 = 0;
  t0 = micros();
  if(t1 != 0){
    int size = sprintf(txt_buf,"%lu\n",t0-t1);
    Serial.write(txt_buf,size);
  }
  */
  uart_buf.put_serial(fp_serial);
  //t1 = micros();
  return;
}

void setup(){
  if(!initializeCommon()){
    return;
  };


  server.on("/fingerprint/image", handleImage );
  server.on("/fingerprint/valid", handleImageValidity);
  server.on("/admin/database/clear", handleClearDB );
  server.on("/admin/set-mode", handleMode);
  attachServerHandlers();
  beginServer();


  fp_serial.setRxBufferSize(UART_BUF_SIZE+2048);
  fp_serial.begin(fp_settings.baudrate*9600ULL,SERIAL_8N2,16,17);
  while(fp_serial.available()){
    fp_serial.read();
  };
  delay(100);
  Serial.print("esp32 sensor serial baudrate : ");
  Serial.println(fp_serial.baudRate());

  if(false){
    ChangeFPBaud(fp,96);
  };

  fp_serial.onReceiveError([](hardwareSerial_error_t err){
    //Serial.print("FP SERIAL ERROR: ");
    //Serial.println(err);
  });



  pinMode(32,OUTPUT);
  pinMode(33,OUTPUT);
  pinMode(25,INPUT);

  digitalWrite(33,HIGH);
  delay(250);
  digitalWrite(33,LOW);
  digitalWrite(32,HIGH);
  delay(250);
  digitalWrite(32,LOW);

  if(WiFi.status() != WL_CONNECTED){
    digitalWrite(33,HIGH);
    delay(250);
    digitalWrite(33,LOW);
    digitalWrite(33,HIGH);
    delay(250);
    digitalWrite(33,LOW);
  };

test_serial:
  {
    static uint8_t test_baud = 0;
    bool auto_detect = false;
    if (VerifyPassword(fp_serial,0xFFFFFFFF,0x0)) {
      Serial.println("Found fingerprint sensor!");
      Serial.print("Baudrate = ");
      Serial.println(fp_serial.baudRate());
      if( test_baud != 0 ){
        fp_settings.baudrate = test_baud;

      };
    } else {
      Serial.print("Did not find fingerprint sensor at ");
      Serial.print(fp_serial.baudRate());
      if(test_baud != 0){
        Serial.print("baudrate ");
        Serial.println(test_baud);
      }else{
        Serial.println("baudrate");
      }

      if(test_baud == 0){
        Serial.println("Entering baudrate test mode");
      }
      if(test_baud < 500){
        test_baud ++;
        fp_serial.updateBaudRate(9600ULL*test_baud);
        goto test_serial;
      }

      return;
    }
  }
  fp.LEDcontrol(false);

  Serial.println(F("Reading sensor parameters"));
  uint16_t template_count = 0;
  ReadBasicParams(fp_serial,0xFFFFFFFF,&fp_table);
  ValidTemplateCount(fp_serial,0xFFFFFFFF,&template_count);


  //fp.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(fp_table.status, HEX);
  Serial.print(F("Sen Type ID: 0x")); Serial.println(fp_table.sensor_type, HEX);
  Serial.print(F("Capacity: ")); Serial.println(fp_table.db_capacity);
  Serial.print(F("Security level: ")); Serial.println(fp_table.security_level);
  Serial.print(F("Device address: ")); Serial.println(fp_table.address, HEX);
  Serial.print(F("Packet Size: ")); Serial.println(fp_table.packet_size);
  Serial.print(F("Baud rate: ")); Serial.println(fp_table.baud_mul_of_9600*9600ULL);
  Serial.print(F("Templates in DB: ")); Serial.println(template_count);

  fp_settings.capacity = fp_table.db_capacity;
  fp_settings.template_count = template_count;

  Serial.println();
  Serial.println();

  /*
  uint8_t* inf_page = (uint8_t*)malloc(512);
  memset(inf_page,0,512);
  if(ReadINFpage(fp_serial,0xFFFFFFFF) == 1){
    fp_serial.onReceive(OnReceiveCall,false);
    downloading = true;
    while(downloading){
      DownloadINFPage(inf_page,512);
    };
    if(download_failed){
      Serial.println("Download INF page Failed");
    }else{
      Serial.println("Downloaded INF page. Dump:");
      dump_byte_array(inf_page,512);
    }

  }else{
    Serial.println("Failed to intiate INF page download");

  }
  free(inf_page);
  */


  image_buf = (uint8_t*)malloc(image_buf_size);
  memset(image_buf,0xFF,image_buf_size);
  //decoding_ctx.data_output = image_buf;
  //decoding_ctx.data_count = &image_buf_count;


  while(fp_serial.available()){
    fp_serial.read();
  };

  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap()); // Prints the current free heap memory

  Serial.print("Total heap size: ");
  Serial.println(ESP.getHeapSize()); // Prints the total heap size

  Serial.print("Minimum free heap since boot: ");
  Serial.println(ESP.getMinFreeHeap()); // Prints the minimum free heap size recorded

  // If using external PSRAM
  if(psramFound()){
    Serial.print("Free PSRAM: ");
    Serial.println(ESP.getFreePsram());
  }

};

void printHexByte(byte value) {
  if (value < 16) {
    Serial.print('0'); // Add leading zero
  }
  Serial.print(value, HEX);
}

FP_MODE mode = FP_MATCH;
bool mode_changed = true;
uint8_t id = 0;

LEDDriverConf green_led(32);
LEDDriverConf red_led(33);

unsigned long stop_t_abs = 0;
void loop(){
  //server.handleClient();
  //dnsServer.processNextRequest();
  green_led.drive();
  red_led.drive();



  uint16_t score = 0;
  uint16_t page_idx = 0;
  if(mode == FP_CAPTURE){

    FPCapture();

  }else if(mode == FP_MATCH){

    if(FPCapAndGen(1)){
      if(FPHiSearchChar(1,0,fp_settings.capacity, &score,&page_idx) == 1){
        Serial.print("/////HiSearchChar Success:");
        Serial.print("Score = ");
        Serial.print(score);
        Serial.print(" At ");
        Serial.println(page_idx);
        ev_sink.send("popup 3");
        char str[128];
        sprintf(str,R"(message --key finger_matched "fingerprint id {id} matched with {score} score" id=%lu score=%lu)",page_idx,score);
        ev_sink.send(str);
        green_led.turnOffAfter(1000);
        red_led.setState(false);
      }
      else{
        Serial.println("/////HiSearchChar Failed");
        red_led.turnOffAfter(1000);
        green_led.setState(false);

      }

    };

  }
  else if(mode == FP_ENROLL){
    if(mode_changed){
      ev_sink.send("popup 0 --indeterminate");
      ev_sink.send(R"(message --key finger_press_req "press the sensor")");

      mode_changed = false;
    }
    static uint16_t last_idx = 0;
    static bool first = false;

    FP_RET ret = FPCapAndReg();
    if(ret == FP_RET_SUCCESS){
      if(FPStoreChar(1,fp_settings.template_count+1)){
        char buf[128];
        sprintf(buf,R"(message --key finger_enrolled "fingerprint id {id} added" id=%lu)",fp_settings.template_count+1);

        ev_sink.send(R"(popup_close)");
        ev_sink.send(R"(popup 2)");
        ev_sink.send(buf);

        last_idx = fp_settings.template_count+1;
        Serial.print("/////StoreChar ");
        Serial.print(fp_settings.template_count+1);
        Serial.println(" Success");
        uint16_t template_count = 0;
        ValidTemplateCount(fp_serial,0xFFFFFFFF,&template_count);
        fp_settings.template_count = template_count;
        Serial.print("/////New Template count = ");
        Serial.println(template_count);
        FPChangeMode(FP_MATCH);
      }else{
        Serial.print("/////StoreChar ");
        Serial.println("Failed");
        ev_sink.send(R"(message --key finger_enroll_failed_0 "Failed press sensor again")");
      }
    }else if(ret == FP_RET_ERR){
      ev_sink.send(R"(message --key finger_enroll_failed_0 "Failed press sensor again")");
    }

  }
  else if(mode == FP_HQ_ENROLL){

  };
};



void handleAdminData(AsyncWebServerRequest *request){
  int32_t resp_code = isAuthenticated(request);
  if(resp_code != 202){
    request->send(resp_code,"","");
    return;
  }

  char *mode_str = nullptr;
  const char json_temp[] = R"({"Info":{"Capacity":%u,"Baud":%llu,"Template Count":%u},"Mode":"%s"})";

  switch(mode){
    case FP_ENROLL:
      mode_str = "ENROLL";break;
    case FP_CAPTURE:
      mode_str = "CAPTURE";break;
    case FP_MATCH:
      mode_str = "MATCH";break;
  }

  char buf[256] ;
  size_t len = sprintf(buf,json_temp,fp_table.db_capacity,fp_table.baud_mul_of_9600*9600ULL,fp_settings.template_count,mode_str);

  request->send(200,"application/json",buf);
  return;
};

void handleMode(AsyncWebServerRequest *request){
  int32_t resp_code = isAuthenticated(request);
  if(resp_code != 202){
    request->send(resp_code,"","");
    return;
  }

  if(!request->hasParam("mode")){
    request->send(400,"","");
    Serial.println("NOPE");
    return;
  };

  AsyncWebParameter* mode_para = request->getParam("mode");

  String new_mode = mode_para->value();

  if(new_mode == "CAPTURE"){
    mode = FP_CAPTURE;
  }
  else if(new_mode == "ENROLL"){
    mode = FP_ENROLL;
  }
  else if(new_mode == "MATCH"){
    mode = FP_MATCH;
  }
  else{
    request->send(400,"","");
    return;
  }
  if(FPChangeMode(mode) != FP_RET_SUCCESS){
    request->send(500,"","");
    return;
  };
  request->send(200,"","");
  return;
};

void handleClearDB(AsyncWebServerRequest* request){
  int32_t resp_code = isAuthenticated(request);
  if(resp_code != 202){
    request->send(resp_code,"","");
    return;
  }
  if(request->method() != HTTP_POST){
    request->send(400,"","");
    return;
  }
  if(FPEmptyDB() != FP_RET_SUCCESS){
    request->send(500,"","");
    return;
  }
  request->send(200,"","");
  return;
};

void handleImage(AsyncWebServerRequest *request){
    if (!image_valid) {
      request->send(404, "text/plain", "404 Not Found");
      return;
    }

    const char *pgmHeader = "P5\n256 288\n255\n";
    size_t headerLen = strlen(pgmHeader);

    auto *response = request->beginChunkedResponse("image/x-portable-graymap",
                                                   [pgmHeader, headerLen](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                                                   // first send PGM header
                                                   if (index < headerLen) {
                                                   size_t toSend = (headerLen - index) > maxLen ? maxLen : (headerLen - index);
                                                   memcpy(buffer, pgmHeader + index, toSend);
                                                   return toSend;
                                                   }

                                                   // send image data after header
                                                   size_t imgIndex = index - headerLen;
                                                   if (imgIndex >= image_buf_count) return 0; // done

                                                   size_t remaining = image_buf_count - imgIndex;
                                                   size_t toSend = remaining > maxLen ? maxLen : remaining;
                                                   memcpy(buffer, image_buf + imgIndex, toSend);
                                                   return toSend;
                                                   }
                                                   );

    response->addHeader("Content-Disposition", "attachment; filename=fingerprint.pgm");
    request->send(response);
}

void handleImageValidity(AsyncWebServerRequest *request){
    if(image_valid){
      request->send(200,"","");
      Serial.println("RETURNED 200");
    }else{
      request->send(404,"","");
      Serial.println("RETURNED 404");
    }
};


bool IsFingerFound(){
  static bool pressed = 0;
  static bool actived = 0;
  static uint32_t t0 = 0;
  if(digitalRead(touch_pin) == HIGH){
    pressed = 0;
    actived = 0;
    fp.LEDcontrol(false);
    return false;
  }else if(pressed == 0){
    t0 = millis();
    pressed = 1;
  }
  int8_t ret = -1;
  if(!actived){
    fp.LEDcontrol(true);
  }
  if(!actived && millis() - t0 > 500) {
    ret = CaptureImage(fp_serial,0xFFFFFFFF);
    if( ret == 1){
      actived = 1;
      return true;
    }
  }
  return false;
}

int ChangeFPBaud(Adafruit_Fingerprint& device,uint8_t mul_of_9600){
  uint8_t v = mul_of_9600;
  if(v < 1){
    return -1;
  }
  else if(v > 96){
    return -1;
  }

  uint8_t ret = device.setBaudRate(mul_of_9600);

  if(ret ==FINGERPRINT_PACKETRECIEVEERR){
    return -2;
  }

  return (int)mul_of_9600;
};

size_t sendFingerprintImageChunk(uint8_t *buffer, size_t maxLen, size_t actualLen) {
    static size_t offset = 0;
    static bool headerSent = false;

    actualLen = 0;

    // PGM header
    if (!headerSent) {
        const char *pgmHeader = "P5\n256 288\n255\n";
        size_t headerLen = strlen(pgmHeader);
        size_t toCopy = (headerLen > maxLen) ? maxLen : headerLen;
        memcpy(buffer, pgmHeader, toCopy);
        actualLen = toCopy;
        headerSent = true;
        return headerSent ? 1 : 0;
    }

    if (offset >= image_buf_count) return 0;

    size_t remaining = image_buf_count - offset;
    size_t toSend = remaining > maxLen ? maxLen : remaining;
    memcpy(buffer, image_buf + offset, toSend);
    actualLen = toSend;
    offset += toSend;

    return 1; // more data to send
}


void dump_byte_array(byte *buffer, size_t bufferSize) {
  for (size_t i = 0; i < bufferSize; i++) {
    // Print a leading space for readability
    Serial.print(" ");

    // Check if value is less than 0x10 (16) to print a leading zero
    if (buffer[i] < 0x10) {
      Serial.print("0");
    }

    // Print the byte value in HEX format
    Serial.print(buffer[i], HEX);
  }
  // Print a newline character after the dump is complete
  Serial.println();
}

bool FPCapture(){

  if(!downloading && IsFingerFound()){
    delay(100);

    while(fp_serial.available()){
      fp_serial.read();
    };

    Serial.println(uart_buf.size());


    stop_t_abs = millis();
    fp.LEDcontrol(false);

    WriteCommandPacket(fp_serial,0xFFFFFFFF,AS608_CMD_UpImage);
    int ret = AwaitResponse(fp_serial,0xFFFFFFFF,nullptr,0,500);
    bool failed =false;
    if(ret == AS608_RESP_OK){
      Serial.println("INCOMING IMAGE PACKETS");
      uart_buf.reset();
      image_buf_count = 0;
    }
    else if(ret == AS608_RESP_RECEPTION_ERROR){
      Serial.println("IMAGE RECEPTION ERROR");
      failed = true;
    }
    else if(ret ==AS608_RESP_UPLOAD_IMAGE_FAILED ){
      Serial.println("IMAGE DOWNLOAD FAILED");
      failed = true;
    }
    else{
      Serial.print("IMAGE ERR: ");
      Serial.println(ret,HEX);
      failed = true;
    }




    if(!failed){
      stop = upload_image == true;
      downloading = upload_image == true;
      if(downloading)
        fp_serial.onReceive(OnReceiveCall,false);

      while(downloading){
        DownloadImage();
      };
      return true;
    }
  };
  return false;

};

void DownloadINFPage(uint8_t* buffer, size_t size){
  if(uart_buf.size() == 534 ){
    downloading = false;

    uint32_t total_data = 0;
    size_t inf_data_size = 0;
    bool running = true;
    download_failed = false;


    while (running && uart_buf.size() >= 9) {

      // 1) Sync to 0xEF 0x01 header
      uint8_t sync2[2];
      while (uart_buf.size() >= 2) {
        uart_buf.peek(sync2, 2);
        if (sync2[0] == 0xEF && sync2[1] == 0x01)
          break;
        uint8_t drop;
        uart_buf.get(&drop);
      }

      if (uart_buf.size() < 9)
        break;

      // 2) Read 9-byte message header
      uint8_t header[9];
      uart_buf.read(header, 9);

      // 3) Extract packet type and big-endian length
      uint8_t type = header[6];

      // read MSB-first 16-bit length correctly
      uint16_t L = (header[7] << 8) | header[8];
      uint16_t data_len = L - 2; // payload only
      total_data += data_len;
      Serial.println(data_len);

      // 4) Parse pixel payload, 1 byte = 2 pixels (4bit + 4bit)
      if (uart_buf.size() >= data_len + 2) {

        // ensure we have space in image buffer
        if (inf_data_size + data_len * 2 > size) {
          Serial.println("INF BUFFER OVERFLOW — aborting!");
          uart_buf.reset();
          download_failed = true;
          downloading = false;
          return;
        }

        inf_data_size += uart_buf.read(buffer+inf_data_size,size-inf_data_size);

        // discard checksum cleanly instead of retreat()
        uint16_t checksum;
        uart_buf.read((uint8_t*)&checksum, 2); // just pop it

      } else {
        break;
      }

      // 5) Print packet summary if needed (disabled for speed)
      // Serial.print("TYPE=0x"); Serial.println(type, HEX);

      // 6) End packet detected (0x08)
      if (type == 0x08) {
        Serial.println("\n---- END PACKET 0x08 RECEIVED ----");
        Serial.print("TOTAL PAYLOAD BYTES = ");
        Serial.println(total_data);
        downloading = false;
        download_failed = false;

        running = false;
      }
    }
  }

  static unsigned long stop_t0 =0;

  stop_t0 = millis();

  if(stop_t0 - stop_t_abs > 3000){
    downloading = false;
    download_failed = true;
    fp_serial.onReceive(nullptr);
    uart_buf.reset();
    //dump_byte_array(uart_buf.data_ptr,uart_buf.size());
    while(fp_serial.available()){
      fp_serial.read();
    };
  }

};

void DownloadImage(){
    if(uart_buf.size() == 38448 ){
      downloading = false;

      uint32_t total_data = 0;
      bool running = true;

      const size_t IMAGE_CAP = image_buf_size; // safety cap


      while (running && uart_buf.size() >= 9) {

        // 1) Sync to 0xEF 0x01 header
        uint8_t sync2[2];
        while (uart_buf.size() >= 2) {
          uart_buf.peek(sync2, 2);
          if (sync2[0] == 0xEF && sync2[1] == 0x01)
            break;
          uint8_t drop;
          uart_buf.get(&drop);
        }

        if (uart_buf.size() < 9)
          break;

        // 2) Read 9-byte message header
        uint8_t header[9];
        uart_buf.read(header, 9);

        // 3) Extract packet type and big-endian length
        uint8_t type = header[6];

        // read MSB-first 16-bit length correctly
        uint16_t L = (header[7] << 8) | header[8];
        uint16_t data_len = L - 2; // payload only
        total_data += data_len;

        // 4) Parse pixel payload, 1 byte = 2 pixels (4bit + 4bit)
        if (uart_buf.size() >= data_len + 2) {

          // ensure we have space in image buffer
          if (image_buf_count + data_len * 2 > IMAGE_CAP) {
            Serial.println("IMAGE BUFFER OVERFLOW — aborting!");
            uart_buf.reset();
            image_valid = false;
            return;
          }

          for (size_t i = 0; i < data_len; i++) {
            uint8_t pixel_pair;
            uart_buf.get(&pixel_pair);

            // Convert 0xXY → [X, Y] pixels
            image_buf[image_buf_count++] = (pixel_pair >> 4) * 17;
            image_buf[image_buf_count++] = (pixel_pair & 0x0F) * 17;
          }

          // discard checksum cleanly instead of retreat()
          uint16_t checksum;
          uart_buf.read((uint8_t*)&checksum, 2); // just pop it

        } else {
          break;
        }

        // 5) Print packet summary if needed (disabled for speed)
        // Serial.print("TYPE=0x"); Serial.println(type, HEX);

        // 6) End packet detected (0x08)
        if (type == 0x08) {
          Serial.println("\n---- END PACKET 0x08 RECEIVED ----");
          Serial.print("TOTAL PAYLOAD BYTES = ");
          Serial.println(total_data);
          Serial.print("NEW IMAGE SIZE = ");
          Serial.println(image_buf_count);
          ev_sink.send("new image");
        downloading = false;

          image_valid = true;
          image_valid = total_data > 0;
          running = false;
        }
      }
      fp_serial.onReceive(nullptr);
      uart_buf.reset();
      //dump_byte_array(uart_buf.data_ptr,uart_buf.size());
      while(fp_serial.available()){
        fp_serial.read();
      };

    };

    //decodeFast(uart_buf,fp_serial);

    static unsigned long stop_t0 =0;

    stop_t0 = millis();

    if(stop_t0 - stop_t_abs > 3000){
      downloading = false;
      fp_serial.onReceive(nullptr);
      uart_buf.reset();
      //dump_byte_array(uart_buf.data_ptr,uart_buf.size());
      while(fp_serial.available()){
        fp_serial.read();
      };
    }

};

bool FPGenChar(uint8_t id){
  if(id == 0 || id > 2)
    return false;
  if(GenerateChar(fp_serial,0xFFFFFFFF,id) != 1){
    return false;
  }
  return true;
};

bool FPRegModel(){
  if(GenerateModel(fp_serial,0xFFFFFFFF)  != 1){
    return false;
  }
  return true;
};

bool FPStoreChar(uint8_t src, uint8_t dest){
  if(src == 0 || src > 2)
    return false;
  if(dest >= fp_settings.capacity)
    return false;
  if(StoreChar(fp_serial,0xFFFFFFFF,src,dest) != 1){
    return false;
  }
  ev_sink.send("data changed");
  return true;

};

bool FPSearchChar(uint8_t src_buf, uint16_t start_page, uint16_t page_count, uint16_t* score, uint16_t* page_idx){
  if(page_count == 0 )
    return false;
  if(start_page > fp_settings.capacity){
    return false;
  }
  if(page_count+start_page > fp_settings.capacity){
    page_count = fp_settings.capacity - start_page;
    if(page_count == 0)
      return false;
  }
  if(src_buf == 0 || src_buf > 2){
    return false;
  }
  if(score == nullptr)
    return false;
  if(page_idx == nullptr)
    return false;

  if(SearchChar(fp_serial,0xFFFFFFFF,src_buf,start_page,page_count,score,page_idx) == 1){
    return true;
  }

  return false;
};

bool FPHiSearchChar(uint8_t src_buf, uint16_t start_page, uint16_t page_count, uint16_t* score, uint16_t* page_idx){
  if(page_count == 0 )
    return false;
  if(start_page > fp_settings.capacity){
    return false;
  }
  if(page_count+start_page > fp_settings.capacity){
    page_count = fp_settings.capacity - start_page;
    if(page_count == 0)
      return false;
  }
  if(src_buf == 0 || src_buf > 2){
    return false;
  }
  if(score == nullptr)
    return false;
  if(page_idx == nullptr)
    return false;

  if(HighSpeedSearchChar(fp_serial,0xFFFFFFFF,src_buf,start_page,page_count,score,page_idx) == 1){
    return true;
  }

  return false;
};

FP_RET FPCapAndGen(uint8_t dest_char){

  if(dest_char != 0 || dest_char < 3){
    if(FPCapture()){
      if(FPGenChar(dest_char)){
        Serial.print("/////GenChar ");
        Serial.print(dest_char);
        Serial.println(" Successeded");
        return FP_RET_SUCCESS;

      }else{
        Serial.print("/////GenChar ");
        Serial.print(dest_char);
        Serial.println(" Failed");
        return FP_RET_ERR;
      }
    }
  }
  else{
    return FP_RET_ERR;
  }
  return FP_RET_BUSY;
};

FP_RET FPEmptyDB(){
  int ret = EmptyDB(fp_serial,0xFFFFFFFF);
  if(ret == 1){
    fp_settings.template_count = 0;
    ev_sink.send("data changed");
    return FP_RET_SUCCESS;
  }
  return FP_RET_ERR;
};

FP_RET FPCapAndReg(bool reset){
  static uint16_t id = 0;

  static uint16_t last_idx = 0;
  if(reset){
    id = 0;
    last_idx = 0;
    return FP_RET_SUCCESS;
  }
  if(id == 0 || id == 1){
    if(FPCapture()){
      if(FPGenChar(id+1)){
        if(id == 0){
          ev_sink.send(R"(message --key finger_enroll_success_0 "First image success. press again")");
        }
        Serial.print("/////GenChar ");
        Serial.print(id+1);
        Serial.println(" Successeded");
        id++;

      }else{
        Serial.print("/////GenChar ");
        Serial.print(id);
        Serial.println(" Failed");
        id = 0;
        return FP_RET_ERR;
      }
    }
  }
  if(id == 2){
    if(FPRegModel()){
      id = 0;
      return FP_RET_SUCCESS;
    }
    else{
      id = 0;
      return FP_RET_ERR;
    }
  }
  return FP_RET_BUSY;
};

FP_RET FPChangeMode(FP_MODE new_mode){
  mode = new_mode;
  mode_changed = true;
  ev_sink.send("data changed");
  FPCapAndReg(true);
  return FP_RET_SUCCESS;
};


void DisableImageUploading(){
  upload_image = false;
};
void EnableImageUploading(){
  upload_image = true;
};

