
static void pcf_write(uint8_t data) ;
static void lcd_pulse(uint8_t data) ;
static void lcd_write4(uint8_t nibble, uint8_t rs) ;
static void lcd_cmd(uint8_t cmd) ;
static void lcd_data(uint8_t data) ;
void lcd_set_cursor(uint8_t col, uint8_t row) ;
void lcd_clear();
void lcd_print(const char *s) ;
void lcd_init() ;
