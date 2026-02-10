#include <Arduino.h>
#include <Wire.h>

#define LCD_ADDR 0x27

// PCF8574 bit masks
#define LCD_RS 0x01
#define LCD_RW 0x02
#define LCD_EN 0x04
#define LCD_BL 0x08

static uint8_t backlight = LCD_BL;

static void pcf_write(uint8_t data) {
    Wire.beginTransmission(LCD_ADDR);
    Wire.write(data | backlight);
    Wire.endTransmission();
}

static void lcd_pulse(uint8_t data) {
    pcf_write(data | LCD_EN);
    delayMicroseconds(1);
    pcf_write(data & ~LCD_EN);
    delayMicroseconds(50);
}

static void lcd_write4(uint8_t nibble, uint8_t rs) {
    uint8_t data = (nibble << 4) | rs;
    lcd_pulse(data);
}

static void lcd_cmd(uint8_t cmd) {
    lcd_write4(cmd >> 4, 0);
    lcd_write4(cmd & 0x0F, 0);
}

static void lcd_data(uint8_t data) {
    lcd_write4(data >> 4, LCD_RS);
    lcd_write4(data & 0x0F, LCD_RS);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    static const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    lcd_cmd(0x80 | (col + row_offsets[row]));
}
void lcd_clear() {
    lcd_cmd(0x01);
    delay(2);
}
void lcd_print(const char *s) {
    while (*s) {
        lcd_data(*s++);
    }
}


void lcd_init() {
    Wire.begin();
    delay(50);

    // Force 4-bit mode
    lcd_write4(0x03, 0);
    delay(5);
    lcd_write4(0x03, 0);
    delay(5);
    lcd_write4(0x03, 0);
    delay(1);
    lcd_write4(0x02, 0);

    // Function set: 4-bit, 2-line, 5x8
    lcd_cmd(0x28);
    // Display ON, cursor OFF
    lcd_cmd(0x0C);
    // Entry mode
    lcd_cmd(0x06);
    // Clear
    lcd_cmd(0x01);
    delay(2);
}

