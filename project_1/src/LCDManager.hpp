//
// Created by gguy on 18/03/2021.
//

#ifndef PROJECT_1_LCDMANAGER_HPP
#define PROJECT_1_LCDMANAGER_HPP

#include <string>

#include "control_lcd_16x2.h"

using namespace std;

class LCDManager {

private:

    int fd;
    const int LCD_ADDRESS = 0x27;

    void write_line(int line, string text);

public:

    LCDManager();

    void write_on_screen(string line1, string line2);

};


#endif //PROJECT_1_LCDMANAGER_HPP
