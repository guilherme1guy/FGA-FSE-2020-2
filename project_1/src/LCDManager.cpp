#include "LCDManager.hpp"
#include "control_lcd_16x2.h"
#include "Logger.h"

using namespace std;

LCDManager::LCDManager()
{
    this->fd = -1;
    get_fd();
}

int LCDManager::get_fd()
{

    if (this->fd == -1)
    {
        try
        {
            this->fd = lcd_get_fd(this->LCD_ADDRESS);
            lcd_init(fd);
        }
        catch(const std::exception& e)
        {
            Logger::log_to_screen(e.what());
            Logger::log_to_screen("Failed to acquire LCD fd");
        }
    }

    return this->fd;
}

void LCDManager::write_line(int line, string text)
{

    // converting to c-type strings for libary integration
    const char *cstr = text.c_str();
    // there is no need to free c_srt, it gets cleaned when
    // text get out of scope
    // https://stackoverflow.com/questions/8843604/string-c-str-deallocation-necessary

    lcdLoc(get_line_location(line), get_fd());
    typeln(cstr, get_fd());
}

void LCDManager::write_on_screen(string line1, string line2)
{
    // use an empty string when the line is not needed

    ClrLcd(get_fd());
    write_line(1, line1);
    write_line(2, line2);
}
