#ifndef PROJECT_2_GPIOMANAGER_H
#define PROJECT_2_GPIOMANAGER_H

#include <set>
#include <thread>

#include <unistd.h>
#include <wiringPi.h>
#include "softPwm.h"


using namespace std;

class GPIOConnection{

private:

    int pin;

public:

    GPIOConnection(int pin, int mode)
    {
        // uses GPIO through softPwm from wiringPI
        // src: https://projects.drogon.net/raspberry-pi/wiringpi/software-pwm-library/
        wiringPiSetup();

        // create virtual fan gpio pin
        pinMode(pin, mode);
        softPwmCreate(pin, 1, 100);
    }

    ~GPIOConnection()
    {
        // turn off device before leaving
        setValue(0);
        softPwmStop(pin);
    }

    void setValue(int value)
    {
        softPwmWrite(pin, value);
        usleep(100000);
    }

};


#endif //PROJECT_2_GPIOMANAGER_H
