#include <SPI.h>
#include <EEPROM.h>
#include "MFRC522.h"

#define RST_PIN         6         // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::MIFARE_Key key;

#include "Common.h"

const uint8_t green_led = 3;
const uint8_t yellow_led = 2;

void setup() {
	Serial.begin(9600);		// Initialize serial communications with the PC
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522

    pinMode(green_led,OUTPUT);
    pinMode(yellow_led,OUTPUT);



    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;  //keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
    }

	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
    check_eeprom();


}

void loop() {


	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

    if(has_name("marwan") == true){
        digitalWrite(green_led,HIGH);
        delay(3000);
        digitalWrite(green_led,LOW);
    }else{
        digitalWrite(yellow_led,HIGH);
        delay(2000);
        digitalWrite(yellow_led,LOW);
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}



int writeBlock(int blockNumber, byte arrayAddress[])
{
  //this makes sure that we only write into data blocks. Every 4th block is a trailer block for the access/security info.
  int largestModulo4Number=blockNumber/4*4;
  int trailerBlock=largestModulo4Number+3;//determine trailer block for the sector
  if (blockNumber > 2 && (blockNumber+1)%4 == 0){Serial.print(blockNumber);Serial.println(" is a trailer block:");return 2;}
  Serial.print(blockNumber);
  Serial.println(" is a data block:");

  //authentication of the desired block for access
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
         Serial.print("PCD_Authenticate() failed: ");
         Serial.println(mfrc522.GetStatusCodeName(status));
         return 3;//return "3" as error message
  }

  //writing the block
  status = mfrc522.MIFARE_Write(blockNumber, arrayAddress, 16);
  //status = mfrc522.MIFARE_Write(9, value1Block, 16);
  if (status != MFRC522::STATUS_OK) {
           Serial.print("MIFARE_Write() failed: ");
           Serial.println(mfrc522.GetStatusCodeName(status));
           return 4;//return "4" as error message
  }
  Serial.println("block was written");
}



//Read specific block
int readBlock(int blockNumber, byte arrayAddress[])
{
    int largestModulo4Number=blockNumber/4*4;
    int trailerBlock=largestModulo4Number+3;//determine trailer block for the sector

    //authentication of the desired block for access
    byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));

    if (status != MFRC522::STATUS_OK) {
        Serial.print("PCD_Authenticate() failed (read): ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return 3;//return "3" as error message
    }

    //reading a block
    byte buffersize = 18;//we need to define a variable with the read buffer size, since the MIFARE_Read method below needs a pointer to the variable that contains the size...
    status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);//&buffersize is a pointer to the buffersize variable; MIFARE_Read requires a pointer instead of just a number
    if (status != MFRC522::STATUS_OK) {
        Serial.print("MIFARE_read() failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return 4;//return "4" as error message
    }
    Serial.println("block was read");
    return 0;
}

