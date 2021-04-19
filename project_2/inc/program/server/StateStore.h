#ifndef PROJECT_2_STATE_H
#define PROJECT_2_STATE_H

#include <map>
#include "Constants.h"
#include "gpio/GPIOConnection.h"

using namespace std;

class StateStore
{

    float temperature;
    float humidity;

    bool alarmEnabled = false;

    map<int, int> outputDevicesStates;

public:
    StateStore()
    {
        auto outputDevices = Constants::getOutputDevices();

        for (auto device : outputDevices)
        {
            outputDevicesStates[device] = GPIOConnection::OFF_VALUE;
        }
    }

    ~StateStore() = default;

    float getTemperature()
    {
        return temperature;
    }

    void setTemperature(float t)
    {
        temperature = t;
    }

    float getHumidity()
    {
        return humidity;
    }

    void setHumidity(float h)
    {
        humidity = h;
    }

    int getDeviceState(int device)
    {
        return outputDevicesStates[device];
    }

    string getDeviceStateString(int device)
    {
        auto state = getDeviceState(device);

        if (state == GPIOConnection::ON_VALUE)
        {
            return "ON";
        }
        else
        {
            return "OFF";
        }
    }

    int invertState(int state)
    {
        if (state == GPIOConnection::ON_VALUE)
        {
            return GPIOConnection::OFF_VALUE;
        }
        else
        {
            return GPIOConnection::ON_VALUE;
        }
    }

    void setDeviceState(int device, int state)
    {
        outputDevicesStates[device] = state;
    }

    bool getAlarmEnabled()
    {
        return alarmEnabled;
    }

    void setAlarmEnabled(int value)
    {
        alarmEnabled = value;
    }
};

#endif // PROJECT_2_STATE_H
