constexpr uint32_t fnv1a(const char* data, uint32_t hash = 0x811c9dc5) {
    return (*data == '\0') ? hash
                           : fnv1a(data + 1, (hash ^ static_cast<uint8_t>(*data)) * 0x01000193);
}

int readBlock(int blockNumber, byte arrayAddress[]);
int writeBlock(int blockNumber, byte arrayAddress[]);

#define MAGIC_NUMBER 0xA5A5A5A5

#define EEPROM_LEN E2END+1



static bool has_name(const char* name){
    const uint32_t hash_pass = fnv1a(name);

    static char block[18] = {0};

    readBlock(1,block);

    uint32_t hash = *((uint32_t*) block);

    /*
    Serial.print("Comparing ");
    Serial.print(hash);
    Serial.print(" To ");
    Serial.println(hash_pass);
    */

    if(hash == hash_pass){
        Serial.println("Pass");
        return true;
    }else{
        Serial.println("Block");
        return false;
    }

    memset(block,0,sizeof(block));

    return false;

}


static int input(){
    uint32_t uid = 0;
    uint8_t block[18] = {0};
    char msg[64] = {0};
    Serial.println("Insert Card");
    while(true){
        if(!mfrc522.PICC_IsNewCardPresent())continue;

        if ( ! mfrc522.PICC_ReadCardSerial()) {
            Serial.println("Failed to read card");
            continue;}
        Serial.println("card selected");
        uid = *((uint32_t*) mfrc522.uid.uidByte);
        Serial.print("card UID ");
        Serial.println(*(uint32_t*)(mfrc522.uid.uidByte),HEX);
        break;
    }

    Serial.println("Type your name");
    while(true){
        if(!Serial.available())continue;
        size_t read_num = Serial.readBytesUntil('\n',msg,sizeof(msg));
        msg[read_num -1] = (msg[read_num-1] == '\r')? '0': msg[read_num-1];

        uint32_t hash = fnv1a(msg);
        memset(block,0,sizeof(block));
        /*
        Serial.print("Name ");
        Serial.print(msg);
        Serial.print(" Hash ");
        Serial.println(hash);
        */


        *((uint32_t*) block) = hash;
        writeBlock(1,block);

        memset(msg,0,sizeof(msg));
        break;
    }


    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

}


bool once = false;
bool init_eeprom = false;

static void check_eeprom(){
    uint32_t stored_value = 0;

    stored_value |= (uint32_t)EEPROM.read(EEPROM_LEN-4) << 24;
    stored_value |= (uint32_t)EEPROM.read(EEPROM_LEN-3) << 16;
    stored_value |= (uint32_t)EEPROM.read(EEPROM_LEN-2) << 8;
    stored_value |= (uint32_t)EEPROM.read(EEPROM_LEN-1);

    if(MAGIC_NUMBER == stored_value && !init_eeprom){
       // Serial.print("Got ");
       // Serial.println(MAGIC_NUMBER,HEX);
        return;
    }

    init_eeprom = false;

    /*
    Serial.print("Got ");
    Serial.print(stored_value,HEX);
    Serial.print(" Instead of ");
    Serial.println(MAGIC_NUMBER,HEX);
    */

    for(int i=0;i<EEPROM_LEN ;i++){
        EEPROM[i] = 0;
    }

    EEPROM.write(EEPROM_LEN-4,MAGIC_NUMBER >> 24);
    EEPROM.write(EEPROM_LEN-3,MAGIC_NUMBER >> 16);
    EEPROM.write(EEPROM_LEN-2,MAGIC_NUMBER >> 8);
    EEPROM.write(EEPROM_LEN-1,MAGIC_NUMBER );

    input();

    if(once)return;
    once = true;
    check_eeprom;
    return;



};
