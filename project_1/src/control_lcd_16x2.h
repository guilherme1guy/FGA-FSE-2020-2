//
// Created by gguy on 14/03/2021.
//

#ifndef PROJECT_1_CONTROL_LCD_16X2_H
#define PROJECT_1_CONTROL_LCD_16X2_H

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif


void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);

// added by Lewis
void typeInt(int i);
void typeFloat(float myFloat);
void lcdLoc(int line); //move cursor
void ClrLcd(void); // clr LCD return home
void typeln(const char *s);
void typeChar(char val);

#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif //PROJECT_1_CONTROL_LCD_16X2_H
