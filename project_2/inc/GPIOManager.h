#ifndef PROJECT_2_GPIOMANAGER_H
#define PROJECT_2_GPIOMANAGER_H

#include <set>
#include <thread>

#include <unistd.h>
#include <wiringPi.h>
#include "softPwm.h"


using namespace std;

class GPIOManager{

public:

    static const int GPIO_RESISTOR_PIN = 4;
    static const int GPIO_FAN_PIN = 5;

    GPIOManager()
    {
        // uses GPIO through softPwm from wiringPI
        // src: https://projects.drogon.net/raspberry-pi/wiringpi/software-pwm-library/
        wiringPiSetup();

        // create virtual fan gpio pin
        pinMode(GPIOManager::GPIO_FAN_PIN, OUTPUT);
        softPwmCreate(GPIOManager::GPIO_FAN_PIN, 1, 100);

        // create virtual resistor gpio pin
        pinMode(GPIOManager::GPIO_RESISTOR_PIN, OUTPUT);
        softPwmCreate(GPIOManager::GPIO_RESISTOR_PIN, 1, 100);

    }

    ~GPIOManager()
    {
        // turn off device before leaving
        softPwmWrite(GPIOManager::GPIO_FAN_PIN, 0);
        usleep(100000);
        softPwmWrite(GPIOManager::GPIO_RESISTOR_PIN, 0);
        usleep(100000);

        softPwmStop(GPIOManager::GPIO_FAN_PIN);
        softPwmStop(GPIOManager::GPIO_RESISTOR_PIN);
    }

    void set_value(int pin, int value)
    {

        softPwmWrite(pin, value);
        usleep(100000);

    }

};


#endif //PROJECT_2_GPIOMANAGER_H
