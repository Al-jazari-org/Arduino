#line 1 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/as608.h"
#include <Arduino.h>
#include <cstdint>

//#define AS608_DEBUG

#define AS608_CMD_LIST(X) \
    X(GetImage,         0) \
    X(GenChar,          1) \
    X(Match,            0) \
    X(Search,           5) \
    X(RegModel,         0) \
    X(StoreChar,        3) \
    X(LoadChar,         3) \
    X(UpChar,           1) \
    X(DownChar,         1) \
    X(UpImage,          0) \
    X(DownImage,        0) \
    X(DeletChar,        4) \
    X(Empty,            0) \
    X(WriteReg,         2) \
    X(ReadSysPara,      0) \
    X(Enroll,           0) \
    X(Identify,         0) \
    X(SetPwd,           4) \
    X(VfyPwd,           4) \
    X(GetRandomCode,    4) \
    X(SetChipAddr,      4) \
    X(ReadINFpage,      0) \
    X(Port_Control,     1) \
    X(WriteNotepad,     33) \
    X(ReadNotepad,      1) \
    X(BurnCode,         2) \
    X(HighSpeedSearch,  4) \
    X(GenBinImage,      2) \
    X(ValidTemplateNum, 0)

// 2) Expand into the enum
#define ENUM_GEN(name, count) AS608_CMD_##name,
enum AS608_CMD : uint8_t {
    AS608_CMD_NONE,
    AS608_CMD_LIST(ENUM_GEN)
    AS608_CMD_MAX
};
#undef ENUM_GEN


// 3) Expand into PARAM_COUNT table automatically
#define COUNT_GEN(name, count) count,
const uint8_t PARAM_COUNT[AS608_CMD_MAX] = {
    0,
    AS608_CMD_LIST(COUNT_GEN)
};
#undef COUNT_GEN

enum AS608_RESP : uint8_t{
    AS608_RESP_OK,
    AS608_RESP_RECEPTION_ERROR,
    AS608_RESP_NOFINGER,
    AS608_RESP_CAPTURE_FAILED,
    AS608_RESP_IMAGE_DRY_FAINT,
    AS608_RESP_IMAGE_WET_SMUGED,
    AS608_RESP_IMAGE_CHOATIC,
    AS608_RESP_IMAGE_INSUFFICIENT_FEATURES,
    AS608_RESP_MATCH_MISMATCH,
    AS608_RESP_MATCH_NONE,
    AS608_RESP_FEATURE_MERGE_FAILED,
    AS608_RESP_DB_OUTOFRANGE,
    AS608_RESP_READ_INVALID_TEMPLATE,
    AS608_RESP_UPLOAD_FEATURE_FAILED,
    AS608_RESP_DATA_INACCAPTABLE,
    AS608_RESP_UPLOAD_IMAGE_FAILED,
    AS608_RESP_DELETE_TEMPLATE_FAILED,
    AS608_RESP_DB_CLEAR_FAILED,
    AS608_RESP_LOWPOWER_FAILED,
    AS608_RESP_PWD_INCORRECT,
    AS608_RESP_SYS_RESET_FAILED,
    AS608_RESP_GEN_IMAGE_BUFFER_EMPTY,
    AS608_RESP_UPDATE_FAILED,
    AS608_RESP_MODEL_FINGERS_MISMATCH,
    AS608_RESP_FLASH_RW_ERROR,
    AS608_RESP_DATA_ACK_ON_FINISH,
    AS608_RESP_DATA_ACK_TO_SEND,
    AS608_RESP_FLASH_CHECKSUM_ERROR,
    AS608_RESP_FLASH_ID_ERROR,
    AS608_RESP_FLASH_LENGTH_ERROR,
    AS608_RESP_FLASH_LENGTH_TOO_LARGE_ERROR,
    AS608_RESP_FLASH_W_ERROR,
    AS608_RESP_UNDEFINED_ERROR,
    AS608_RESP_REG_INVALID,
    AS608_RESP_REG_INCORRECT,
    AS608_RESP_NT_INCORRECT,
    AS608_RESP_PORT_FAILED,
    AS608_RESP_ENTROLL_AUTO_FAILED,
    AS608_RESP_DB_FULL,
    AS608_RESP_MAX,
};

struct __attribute__((packed)) AS608_BasicPTable {
    uint16_t status;
    uint16_t sensor_type;
    uint16_t db_capacity;
    uint16_t security_level;
    uint32_t address;
    uint16_t packet_size;
    uint16_t baud_mul_of_9600;
};

size_t WriteCommandPacket(HardwareSerial& serial,uint32_t device_address,uint8_t ins_code,uint8_t* parameter = nullptr);
int AwaitResponse(HardwareSerial& serial,uint32_t addr,uint8_t* data_buffer,size_t size,unsigned long timeout_ms);
int VerifyPassword(HardwareSerial& serial,uint32_t addr,uint32_t pwd);
int CaptureImage(HardwareSerial& serial,uint32_t addr);
int ReadBasicParams(HardwareSerial& serial,uint32_t addr,AS608_BasicPTable* table);
int ValidTemplateCount(HardwareSerial& serial,uint32_t addr,uint16_t* count);
int GenerateChar(HardwareSerial& serial,uint32_t addr,uint8_t dest);
int StoreChar(HardwareSerial& serial,uint32_t addr,uint8_t src,uint16_t dest);
int GenerateModel(HardwareSerial& serial,uint32_t addr);
int MatchChars(HardwareSerial& serial,uint32_t addr,uint32_t* score);
int SearchChar(HardwareSerial& serial,uint32_t addr,uint8_t src_char,uint16_t dest,uint16_t count,uint16_t* score,uint16_t* page_idx);
int HighSpeedSearchChar(HardwareSerial& serial,uint32_t addr,uint8_t src_char,uint16_t dest,uint16_t count,uint16_t* score,uint16_t* page_idx);
int ReadINFpage(HardwareSerial& serial,uint32_t addr);
int EmptyDB(HardwareSerial& serial,uint32_t addr);
