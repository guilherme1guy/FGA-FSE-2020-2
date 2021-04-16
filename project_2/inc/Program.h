#ifndef PROJECT_2_PROGRAM_H
#define PROJECT_2_PROGRAM_H

#include <iostream>
#include <string>
#include <thread>

#include "curses.h"

#include "Logger.h"
#include "GPIOConnection.h"
#include "BMEManager.h"

using namespace std;

class Program{

private:

    void menu()
    {

        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);
        nonl();
        cbreak();
        echo();

        if (has_colors())
        {
            start_color();

            init_pair(1, COLOR_RED, COLOR_BLACK);
            init_pair(2, COLOR_GREEN, COLOR_BLACK);
            init_pair(3, COLOR_YELLOW, COLOR_BLACK);
            init_pair(4, COLOR_BLUE, COLOR_BLACK);
            init_pair(5, COLOR_CYAN, COLOR_BLACK);
            init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(7, COLOR_WHITE, COLOR_BLACK);
        }

        for (;;)
        {
            clear();
            move(1, 1);

            drawInformation();
            drawDivision();

            printw("Running...");

            int c = getch(); /* refresh, accept single keystroke of input */

            if (c == 'q')
            {
                break;
            }

            refresh();

            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }

    void drawDivision()
    {
        printw("-------------------------------\n");
    }


    void drawInformation()
    {

    }


public:

    Program()
    {

    }

    ~Program()
    {
    }


    void run()
    {
        menu();
    }


    void quit()
    {
        delete Logger::getInstance();
        endwin();
    }

};


#endif //PROJECT_2_PROGRAM_H
