#include "as608.h"

struct __attribute__((packed)) Packet_upper{
    uint16_t header = 0x01EF;
    uint32_t device_address = 0x0;
    uint8_t id = 0x00;
    uint16_t length = 0x0000;
};

void dump_byte_array(byte *buffer, size_t bufferSize);


size_t WriteCommandPacket(HardwareSerial& serial,uint32_t device_address,uint8_t ins_code,uint8_t* parameter){

    while(serial.available()){
        serial.read();
    };
    Packet_upper upper;
    upper.device_address = __builtin_bswap32(device_address);
    upper.id = 0x01;

    uint16_t length_lsb = 3;


    AS608_CMD cmd = (AS608_CMD) ins_code;

    if(cmd == AS608_CMD_NONE){
        return -4;
    };

    uint8_t param_count = PARAM_COUNT[cmd];

    if(param_count > 0 && parameter == nullptr){
#ifdef AS608_DEBUG
        Serial.println("Passing nullptr to a ins with return");
#endif
        return -3;
    };

    length_lsb += param_count;

    uint16_t checksum_msb = ((uint16_t)((uint8_t*)&length_lsb)[0] + ((uint8_t*)&length_lsb)[1])
        +ins_code+upper.id;

    for(size_t i = 0;i<param_count;i++){
        checksum_msb += parameter[i];
    };

#ifdef AS608_DEBUG
    Serial.print("Check LSB ");
    Serial.println(checksum_msb,HEX);
#endif

    checksum_msb = __builtin_bswap16(checksum_msb);

#ifdef AS608_DEBUG
    Serial.print("Check MSB ");
    Serial.println(checksum_msb,HEX);
#endif


    upper.length = __builtin_bswap16(length_lsb);

    size_t ret = 0;

    ret =  serial.write((uint8_t*)&upper,sizeof(upper));
    ret += serial.write(ins_code);
    ret += serial.write(parameter,param_count);
    ret += serial.write((uint8_t*)&checksum_msb,sizeof(checksum_msb));
#ifdef AS608_DEBUG
    Serial.print("Sent ");
    Serial.println(ret);
    dump_byte_array((uint8_t*)&upper,sizeof(upper));
    dump_byte_array((uint8_t*)&ins_code,1);
    dump_byte_array(parameter,param_count);
    dump_byte_array((uint8_t*)&checksum_msb,sizeof(checksum_msb));
#endif

    return ret;
};

int AwaitResponse(HardwareSerial& serial,uint32_t addr,uint8_t* data_buffer,size_t size,unsigned long timeout_ms){

    uint8_t resp_upper_buf[10] = {0};

    unsigned long t_abs = millis();
    while(serial.available() < 10){
        if(millis() - t_abs > timeout_ms){
            return -1;
        };
    };
    serial.readBytes(resp_upper_buf,10);

#ifdef AS608_DEBUG
    Serial.print("Got: ");
    dump_byte_array(resp_upper_buf,sizeof(resp_upper_buf));
#endif

    if(*((uint16_t*)resp_upper_buf) != 0x01EF){
        return -2;
    }

    uint16_t length = __builtin_bswap16(*(uint16_t*)(resp_upper_buf+7));

#ifdef AS608_DEBUG
    Serial.print("Length ");
    Serial.println(length);
#endif

    if(length-3 > size){
        return -3;
    }

    uint16_t checksum_msb =  (uint16_t)((uint8_t*)&length)[0] + ((uint8_t*)&length)[1]  + 0x07 + resp_upper_buf[9];

    size_t read = 0;
    t_abs = millis();
    while((read+1) != (length-2)){
        size_t avail = serial.available();
        size_t to_read = min(size-read,avail);
        if(to_read > 0){
            serial.readBytes(data_buffer+read,to_read);
            read += to_read;
            t_abs = millis();
            continue;
        };
        if(millis() - t_abs >timeout_ms){
#ifdef AS608_DEBUG
            Serial.print("Read only ");
            Serial.println(read);
            dump_byte_array(resp_upper_buf,sizeof(resp_upper_buf));
#endif
            return -4;
        };
    }

    for (size_t i = 0; i < read; i++) {
        checksum_msb += data_buffer[i];
    }

    checksum_msb = __builtin_bswap16(checksum_msb);

    uint16_t resp_checksum = 0;
    t_abs = millis();
    while(serial.available() < 2){
        if(millis() - t_abs > timeout_ms){
            return -5;
        };
    };
    serial.readBytes((uint8_t*)&resp_checksum,sizeof(resp_checksum));

    if(resp_checksum != checksum_msb){
#ifdef AS608_DEBUG
        Serial.print(resp_checksum,HEX);
        Serial.print(" != ");
        Serial.println(checksum_msb,HEX);
        Serial.println("FULL RECIEVED PACKET:");
        dump_byte_array(resp_upper_buf,sizeof(resp_upper_buf));
        dump_byte_array(data_buffer,read);
        dump_byte_array((uint8_t*)&resp_checksum,sizeof(resp_checksum));
#endif

        return -6;
    };

#ifdef AS608_DEBUG
            Serial.print("RESPONSE CODE =");
            Serial.println(resp_upper_buf[9]);
#endif

    return resp_upper_buf[9];
};

int VerifyPassword(HardwareSerial& serial,uint32_t addr,uint32_t pwd){
    uint32_t resp_pwd;
    uint32_t pwd_msb =
        (pwd >> 24) |
        ((pwd >> 8)  & 0x0000FF00) |
        ((pwd << 8)  & 0x00FF0000) |
        (pwd << 24);
    WriteCommandPacket(serial,addr,AS608_CMD_VfyPwd,(uint8_t*)&pwd_msb);
    int ret = AwaitResponse(serial,addr,(uint8_t*)&resp_pwd,sizeof(resp_pwd),100);
    if(ret < 0){
#ifdef AS608_DEBUG
        Serial.print("PWD RESPONE ERRORED WITH ");
        Serial.println(ret);
#endif
        return -2;
    }
    if(ret == AS608_RESP_OK){
        return 1;
    }else if(ret == AS608_RESP_PWD_INCORRECT){
        return 0;
    }else{
#ifdef AS608_DEBUG
        Serial.print("PWD ERRORED WITH ");
        Serial.println(ret);
#endif
        return -1;
    }
};

int CaptureImage(HardwareSerial& serial,uint32_t addr){
    WriteCommandPacket(serial,addr,AS608_CMD_GetImage,nullptr);
    int ret = AwaitResponse(serial,addr,nullptr,0,500);
    if(ret < 0){
#ifdef AS608_DEBUG
        Serial.print("RESPONE ERRORED WITH ");
        Serial.println(ret);
#endif
        return -2;
    }
    if(ret == AS608_RESP_OK){
        return 1;
    }else{
#ifdef AS608_DEBUG
        Serial.print("ERRORED WITH ");
        Serial.println(ret);
#endif
        return 0;
    }

    return 0;
};

int ReadBasicParams(HardwareSerial& serial,uint32_t addr,AS608_BasicPTable* table){
    AS608_BasicPTable table_msb;
    WriteCommandPacket(serial,addr,AS608_CMD_ReadSysPara);
    int ret = AwaitResponse(serial,addr,(uint8_t*)&table_msb,sizeof(table_msb),500);
    if(ret < 0){
#ifdef AS608_DEBUG
        Serial.print("ReadSysPara RESPONSE ERRORED WITH ");
        Serial.println(ret);
#endif
        return -2;
    }
    if(ret == AS608_RESP_OK){
        table->status = __builtin_bswap16(table_msb.status);
        table->sensor_type = __builtin_bswap16(table_msb.sensor_type);
        table->db_capacity = __builtin_bswap16(table_msb.db_capacity);
        table->security_level = __builtin_bswap16(table_msb.security_level);
        table->address = __builtin_bswap32(table_msb.address);
        table->packet_size = __builtin_bswap16(table_msb.packet_size);
        table->baud_mul_of_9600 = __builtin_bswap16(table_msb.baud_mul_of_9600);
        return 1;
    }else{
#ifdef AS608_DEBUG
        Serial.print("ReadSysPara ERRORED WITH ");
        Serial.println(ret);
#endif
        return -1;
    }
};

int ValidTemplateCount(HardwareSerial& serial,uint32_t addr,uint16_t* count){
    uint16_t count_msb = 0;
    WriteCommandPacket(serial,addr,AS608_CMD_ValidTemplateNum);
    int ret = AwaitResponse(serial,addr,(uint8_t*)&count_msb,sizeof(count_msb),500);
    if(ret < 0){
#ifdef AS608_DEBUG
        Serial.print("ValidTemplateNum RESPONSE ERRORED WITH ");
        Serial.println(ret);
#endif
        return -2;
    }
    if(ret == AS608_RESP_OK){
        *count = __builtin_bswap16(count_msb);
        return 1;
    }else{
#ifdef AS608_DEBUG
        Serial.print("ValidTemplateNum ERRORED WITH ");
        Serial.println(ret);
#endif
        return -1;
    }

}


int GenerateChar(HardwareSerial& serial,uint32_t addr,uint8_t dest){
    if(dest <= 0 || dest > 2){
        return -3;
    }

    WriteCommandPacket(serial,addr,AS608_CMD_GenChar,&dest);
    int ret = AwaitResponse(serial,addr,nullptr,0,10000);
    if(ret < 0){
#ifdef AS608_DEBUG
        Serial.print("GenChar RESPONSE ERRORED WITH ");
        Serial.println(ret);
#endif
        return -2;
    }
    if(ret == AS608_RESP_OK){
        return 1;
    }else{
#ifdef AS608_DEBUG
        Serial.print("GenChar ERRORED WITH ");
        Serial.println(ret);
#endif
        return -1;
    }

};

int StoreChar(HardwareSerial& serial,uint32_t addr,uint8_t src,uint16_t dest){

    if(src <= 0 || src > 2){
        return -3;
    }

    uint8_t buf[] = {src,((uint8_t*)&dest)[1],((uint8_t*)&dest)[0]};

    WriteCommandPacket(serial,addr,AS608_CMD_StoreChar,buf);
    int ret = AwaitResponse(serial,addr,nullptr,0,500);
    if(ret < 0){
#ifdef AS608_DEBUG
        Serial.print("StoreChar RESPONSE ERRORED WITH ");
        Serial.println(ret);
#endif
        return -2;
    }
    if(ret == AS608_RESP_OK){
        return 1;
    }else{
#ifdef AS608_DEBUG
        Serial.print("StoreChar ERRORED WITH ");
        Serial.println(ret);
#endif
        return -1;
    }

};

int GenerateModel(HardwareSerial& serial,uint32_t addr){


    WriteCommandPacket(serial,addr,AS608_CMD_RegModel);
    int ret = AwaitResponse(serial,addr,nullptr,0,500);
    if(ret < 0){
#ifdef AS608_DEBUG
        Serial.print("RegChar RESPONSE ERRORED WITH ");
        Serial.println(ret);
#endif
        return -2;
    }
    if(ret == AS608_RESP_OK){
        return 1;
    }else if(ret == AS608_RESP_FEATURE_MERGE_FAILED){
        return 0;
    }
    else{
#ifdef AS608_DEBUG
        Serial.print("RegChar ERRORED WITH ");
        Serial.println(ret);
#endif
        return -1;
    }


}

int SearchChar(HardwareSerial& serial,uint32_t addr,uint8_t src_char,uint16_t dest,uint16_t count,uint16_t* score,uint16_t* page_idx){

    uint8_t buf[] = {src_char,((uint8_t*)&dest)[1],((uint8_t*)&dest)[0],((uint8_t*)&count)[0],((uint8_t*)&count)[1]};
    struct __attribute__((packed)) {
        uint16_t ret_page_idx_msb = 0;
        uint16_t score_msb = 0;
    }ret_msg;

    WriteCommandPacket(serial,addr,AS608_CMD_Search,buf);
    int ret = AwaitResponse(serial,addr,(uint8_t*)&ret_msg,sizeof(ret_msg),4000);
    if(ret < 0){
#ifdef AS608_DEBUG
        Serial.print("Search RESPONSE ERRORED WITH ");
        Serial.println(ret);
#endif
        return -2;
    }
    if(ret == AS608_RESP_OK){
        *score = __builtin_bswap16(ret_msg.score_msb);
        *page_idx = __builtin_bswap16(ret_msg.ret_page_idx_msb);
        return 1;
    }else if(ret == AS608_RESP_MATCH_NONE){
        return 0;
    }
    else{
#ifdef AS608_DEBUG
        Serial.print("Search ERRORED WITH ");
        Serial.println(ret);
#endif
        return -1;
    }


};

int HighSpeedSearchChar(HardwareSerial& serial,uint32_t addr,uint8_t src_char,uint16_t dest,uint16_t count,uint16_t* score,uint16_t* page_idx){

    uint8_t buf[] = {src_char,((uint8_t*)&dest)[1],((uint8_t*)&dest)[0],((uint8_t*)&count)[0],((uint8_t*)&count)[1]};
    struct __attribute__((packed)) {
        uint16_t ret_page_idx_msb = 0;
        uint16_t score_msb = 0;
    }ret_msg;

    WriteCommandPacket(serial,addr,AS608_CMD_HighSpeedSearch,buf);
    int ret = AwaitResponse(serial,addr,(uint8_t*)&ret_msg,sizeof(ret_msg),4000);
    if(ret < 0){
#ifdef AS608_DEBUG
        Serial.print("HighSpeedSearch RESPONSE ERRORED WITH ");
        Serial.println(ret);
#endif
        return -2;
    }
    if(ret == AS608_RESP_OK){
        *score = __builtin_bswap16(ret_msg.score_msb);
        *page_idx = __builtin_bswap16(ret_msg.ret_page_idx_msb);
        return 1;
    }else if(ret == AS608_RESP_MATCH_NONE){
        return 0;
    }
    else{
#ifdef AS608_DEBUG
        Serial.print("HighSpeedSearch ERRORED WITH ");
        Serial.println(ret);
#endif
        return -1;
    }


};


int ReadINFpage(HardwareSerial& serial,uint32_t addr){
    WriteCommandPacket(serial,addr,AS608_CMD_ReadINFpage);
    int ret = AwaitResponse(serial,addr,nullptr,0,500);
    if(ret < 0){
#ifdef AS608_DEBUG
        Serial.print("ReadINFpage RESPONSE ERRORED WITH ");
        Serial.println(ret);
#endif
        return -2;
    }
    if(ret == AS608_RESP_OK){
        return 1;
    }
    else{
#ifdef AS608_DEBUG
        Serial.print("ReadINFpage ERRORED WITH ");
        Serial.println(ret);
#endif
        return -1;
    }

};

int EmptyDB(HardwareSerial& serial,uint32_t addr){
    WriteCommandPacket(serial,addr,AS608_CMD_Empty);
    int ret = AwaitResponse(serial,addr,nullptr,0,500);
    if(ret < 0){
#ifdef AS608_DEBUG
        Serial.print("Empty RESPONSE ERRORED WITH ");
        Serial.println(ret);
#endif
        return -2;
    }
    if(ret == AS608_RESP_OK){
        return 1;
    }
    else{
#ifdef AS608_DEBUG
        Serial.print("Empty ERRORED WITH ");
        Serial.println(ret);
#endif
        return -1;
    }

};
