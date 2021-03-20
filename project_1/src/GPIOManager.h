#ifndef PROJECT_1_GPIOMANAGER_H
#define PROJECT_1_GPIOMANAGER_H

#include <set>

using namespace std;

class GPIOManager{

public:

    static const int GPIO_RESISTOR_PIN = 23;
    static const int GPIO_FAN_PIN = 24;

    GPIOManager();
    ~GPIOManager();

    void set_value(int pin, int value);

};


#endif //PROJECT_1_GPIOMANAGER_H
