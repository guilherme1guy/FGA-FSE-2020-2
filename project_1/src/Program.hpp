#ifndef PROJECT_1_PROGRAM_H
#define PROJECT_1_PROGRAM_H

#include "TemperatureController.hpp"

using namespace std;

class Program{

private:

    TemperatureController* temperature_controller;

    void menu();

    void draw_logs();
    void draw_division();
    void draw_information();

public:

    Program();
    ~Program();

    void run();
    void quit();

};


#endif //PROJECT_1_PROGRAM_H
