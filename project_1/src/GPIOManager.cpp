#include <wiringPi.h>
#include "softPwm.h"

#include "GPIOManager.h"

using namespace std;

GPIOManager::GPIOManager()
{
    // uses GPIO through softPwm from wiringPI
    // src: https://projects.drogon.net/raspberry-pi/wiringpi/software-pwm-library/
    wiringPiSetupGpio();

    // create virtual fan gpio pin
    softPwmCreate(this->GPIO_FAN_PIN, 0, 100);

    // create virtual resistor gpio pin
    softPwmCreate(this->GPIO_RESISTOR_PIN, 0, 100);
 
}

GPIOManager::~GPIOManager()
{
    // turn off device before leaving
    softPwmWrite(this->GPIO_FAN_PIN, 0);
    softPwmWrite(this->GPIO_RESISTOR_PIN, 0);

    softPwmStop(this->GPIO_FAN_PIN);
    softPwmStop(this->GPIO_RESISTOR_PIN);
}

void GPIOManager::set_value(int pin, int value)
{
    softPwmWrite(pin, value);
}
