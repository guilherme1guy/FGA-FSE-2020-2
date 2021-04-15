#ifndef PROJECT_2_PROGRAM_H
#define PROJECT_2_PROGRAM_H

#include <iostream>
#include <string>
#include <thread>

#include "curses.h"

#include "Logger.h"
#include "GPIOManager.h"
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

        bool temperature_set = false;
        string temperature_chars;

        for (;;)
        {
            clear();
            move(1, 1);

            //draw_logs();
            draw_division();

            draw_information();
            draw_division();

            int c = getch(); /* refresh, accept single keystroke of input */

            if (temperature_set)
            {

                printw("Input desired temperature. A negative value will set it to sensor mode.\n");
                printw("Press ENTER when done.\n");
                printw("> %s", temperature_chars.c_str());
                if (c == '\n' || c == 13) // 13 is carriage return char
                {
                    try
                    {

                        float temperature = stof(temperature_chars);
                        if (temperature > 100)
                            temperature = 100.0;

                        temperature_chars = "";
                        temperature_set = false;
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';

                        temperature_chars = "";
                    }
                }
                else if (c != -1)
                {
                    temperature_chars += c;
                }
            }
            else
            {
                printw("Commands: q - Exit // s - Set Reference Temperature\n");
                printw("> ");

                if (c == 'q')
                {
                    break;
                }
                else if (c == 's')
                {
                    temperature_set = true;
                }
            }

            refresh();

            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }

    void draw_logs()
    {
        auto lines = Logger::get_log_lines();

        for (const auto& line : lines)
        {
            printw(line.c_str());
        }
    }

    void draw_division()
    {
        printw("-------------------------------\n");
    }


    void draw_information()
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
        Logger::end_logger();

        endwin();
    }

};


#endif //PROJECT_2_PROGRAM_H
