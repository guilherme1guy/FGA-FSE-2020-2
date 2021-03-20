#ifndef PROJECT_1_PROGRAM_H
#define PROJECT_1_PROGRAM_H

#include "TemperatureController.hpp"

using namespace std;

class Program{

private:

    PotentiometerManager* potentiometer; 
    BMEManager* bme_manager; 
    LCDManager* lcd_manager;

    int menu();


public:

    Program();
    ~Program();

    void run();

};


#endif //PROJECT_1_PROGRAM_H
