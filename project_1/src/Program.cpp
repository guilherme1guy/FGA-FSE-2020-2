#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#include <ncurses.h>

#include "Program.hpp"

#include "Logger.h"
#include "TemperatureController.hpp"

using namespace std;

Program::Program()
{
    this->temperature_controller = new TemperatureController();
}

Program::~Program()
{
    // TODO: call method to safely stop temperature_controller
    delete temperature_controller;
}

void Program::draw_logs()
{
    auto lines = Logger::get_log_lines();

    for (auto line : lines)
    {
        printw(line.c_str());
    }
}

void Program::draw_information()
{
    printw(
        "IT %.2f ET %.2f RT %.2f PID %.2f\n",
        this->temperature_controller->get_internal_temperature(),
        this->temperature_controller->get_external_temperature(),
        this->temperature_controller->get_reference_temperature(),
        this->temperature_controller->get_temperature_adjustment());
}

void Program::draw_division()
{
    printw("-------------------------------\n");
}

void Program::menu()
{

    initscr();
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
    string temperature_chars = "";

    for (;;)
    {
        clear();
        move(1, 1);

        draw_logs();
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

                    this->temperature_controller->set_reference_temperature(temperature);
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

        draw_division();

        refresh();

        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

void Program::run()
{
    this->temperature_controller->start();
    menu();
}

void Program::quit()
{

    this->temperature_controller->end();
    Logger::end_logger();
    
    endwin();
}