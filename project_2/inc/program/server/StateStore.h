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

    void setTemperature(float t)
    {
        temperature = t;
    }

    void setHumidity(float h)
    {
        humidity = h;
    }

    int getDeviceState(int device)
    {
        return outputDevicesStates[device];
    }

    void setDeviceState(int device, int state)
    {
        outputDevicesStates[device] = state;
    }
};

#endif // PROJECT_2_STATE_H
