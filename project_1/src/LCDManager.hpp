#ifndef PROJECT_1_LCDMANAGER_HPP
#define PROJECT_1_LCDMANAGER_HPP

#include <string>
#include <thread>

#include "control_lcd_16x2.h"

using namespace std;

class LCDManager {

private:

    int fd;
    const int LCD_ADDRESS = 0x27;

    string line1;
    string line2;

    bool execute;
    thread* execution_thread;
    void execution_loop();

    void write_line(int line, const string& text);
    void _write_on_screen();

    int get_fd();

public:

    LCDManager();
    ~LCDManager();

    void write_on_screen(string l1, string l2);

};


#endif //PROJECT_1_LCDMANAGER_HPP
