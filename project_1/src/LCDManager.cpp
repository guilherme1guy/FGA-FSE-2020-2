#include <thread>
#include <string>
#include <chrono>

#include "LCDManager.hpp"
#include "control_lcd_16x2.h"
#include "Logger.h"

using namespace std;

LCDManager::LCDManager()
{
    this->fd = -1;
    get_fd();

    this->execute = true;
    this->execution_thread = new thread(&LCDManager::execution_loop, this);
}

LCDManager::~LCDManager(){

    this->execute = false;
    this->execution_thread->join();

    ClrLcd(get_fd());

    delete this->execution_thread;
}

void LCDManager::execution_loop(){

    while (this->execute){
        this_thread::sleep_for(chrono::milliseconds(500));

        this->_write_on_screen();
    }

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


void LCDManager::_write_on_screen()
{
    write_line(1, line1);
    write_line(2, line2);
}


void LCDManager::write_on_screen(string line1, string line2)
{
    // use an empty string when the line is not needed

    this->line1 = line1;
    this->line2 = line2;
}
