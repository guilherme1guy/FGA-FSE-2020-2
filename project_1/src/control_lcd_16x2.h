#ifndef PROJECT_1_CONTROL_LCD_16X2_H
#define PROJECT_1_CONTROL_LCD_16X2_H

/*! CPP guard */
#ifdef __cplusplus
extern "C"
{
#endif

    int lcd_get_fd(int address);

    void lcd_init(int fd);
    void lcd_byte(int bits, int mode, int fd);
    void lcd_toggle_enable(int bits, int fd);

    // added by Lewis
    void typeInt(int i, int fd);
    void typeFloat(float myFloat, int fd);
    void lcdLoc(int line, int fd); //move cursor
    void ClrLcd(int fd);           // clr LCD return home
    void typeln(const char *s, int fd);
    void typeChar(char val, int fd);

    int get_line_location(int line_number);

#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif //PROJECT_1_CONTROL_LCD_16X2_H
