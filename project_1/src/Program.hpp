#ifndef PROJECT_1_PROGRAM_H
#define PROJECT_1_PROGRAM_H

#include "PotentiometerManager.hpp"
#include "LCDManager.hpp"
#include "BMEManager.hpp"

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
