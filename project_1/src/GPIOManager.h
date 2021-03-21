#ifndef PROJECT_1_GPIOMANAGER_H
#define PROJECT_1_GPIOMANAGER_H

#include <set>
#include <thread>

using namespace std;

class GPIOManager{

public:

    static const int GPIO_RESISTOR_PIN = 4;
    static const int GPIO_FAN_PIN = 5;

    GPIOManager();
    ~GPIOManager();

    void set_value(int pin, int value);

};


#endif //PROJECT_1_GPIOMANAGER_H
