#include "LCDManager.hpp"
#include "control_lcd_16x2.h"

using namespace std;

LCDManager::LCDManager()
{

    this->fd = lcd_get_fd(this->LCD_ADDRESS);
    lcd_init(fd);
}

void LCDManager::write_line(int line, string text)
{

    // converting to c-type strings for libary integration
    const char *cstr = text.c_str();
    // there is no need to free c_srt, it gets cleaned when
    // text get out of scope
    // https://stackoverflow.com/questions/8843604/string-c-str-deallocation-necessary

    lcdLoc(get_line_location(line), this->fd);
    typeln(cstr, this->fd);
}

void LCDManager::write_on_screen(string line1, string line2)
{
    // use an empty string when the line is not needed

    ClrLcd(this->fd);
    write_line(1, line1);
    write_line(2, line2);
}
