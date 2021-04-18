#ifndef PROJECT_2_GPIOConnection_H
#define PROJECT_2_GPIOConnection_H

#include <set>
#include <thread>

#include <unistd.h>
#include <wiringPi.h>
#include "softPwm.h"

using namespace std;

class GPIOConnection
{

private:
    int pin;
    int mode;
    int value;

public:
    // expose wiringPI macros to other classes to avoid
    // including wiringPI into then
    static const int GPIO_INPUT = INPUT;
    static const int GPIO_OUTPUT = OUTPUT;

    static const int ON_VALUE = 100;
    static const int OFF_VALUE = 0;

    GPIOConnection(int pin, int mode)
    {

        this->pin = pin;
        this->mode = mode;

        // uses GPIO through softPwm from wiringPI
        // src: https://projects.drogon.net/raspberry-pi/wiringpi/software-pwm-library/
        wiringPiSetup();

        // create virtual fan gpio pin
        // mode is OUTPUT or INPUT
        pinMode(pin, mode);
        softPwmCreate(pin, 1, 100);

        // always initialize at value 0
        this->value = OFF_VALUE;
        setValue(OFF_VALUE);
    }

    ~GPIOConnection()
    {
        // turn off device before leaving

        if (mode == OUTPUT)
        {
            setValue(0);
        }

        softPwmStop(pin);
    }

    void setValue(int value)
    {
        if (mode == OUTPUT)
        {
            softPwmWrite(pin, value);
            usleep(100000);

            this->value = value;
        }
    }

    int getValue()
    {
        if (mode == INPUT)
        {
            value = digitalRead(pin);
        }

        return value;
    }

    int getPin()
    {
        return pin;
    }
};

#endif //PROJECT_2_GPIOConnection_H
