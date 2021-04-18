#ifndef PROJECT_2_BMEMANAGER_HPP
#define PROJECT_2_BMEMANAGER_HPP

#include "external/bme280.h"

#include <tuple>
#include <errno.h>
#include <string.h>

#include "log/Logger.h"

using namespace std;

class BMEManager
{

    int i2cFile;

    int openDevice()
    {
        int fd = bme280Init(1, 0x76);

        if (fd < 0)
        {
            Logger::logToScreen("Error opening I2C");
        }

        return fd;
    }

    void closeDevice()
    {

        Logger::logToScreen("Closing I2C");

        if (i2cFile >= 0)
        {
            if (close(this->i2cFile) < 0)
            {
                perror(strerror(errno));
            }
            i2cFile = -1;
        }
    }

    // temperature, pression, humidity
    tuple<float, float, float> readFromDevice()
    {

        int iTemperature, iPressure, iHumidity;
        float temperature, pressure, humidity;

        int r = bme280ReadValues(&iTemperature, &iPressure, &iHumidity);

        if (r == -1)
        {
            // error reading from device, close i2cFile so it can be
            // reopened later
            Logger::logToScreen("I2C reading error");
            closeDevice();
            return make_tuple(0, 0, 0);
        }

        // value conversion for float
        temperature = (float)iTemperature / 100.0;
        iHumidity = (float)iPressure / 256.0;
        humidity = (float)iHumidity / 1024.0;

        return make_tuple(temperature, pressure, humidity);
    }

public:
    BMEManager()
    {

        this->i2cFile = openDevice();
    };

    ~BMEManager()
    {
        closeDevice();
    }

    // temperature, pression, humidity
    tuple<float, float, float> getData()
    {

        // if i2c is not open, try opening it again
        if (i2cFile < 0)
        {
            i2cFile = openDevice();
        }

        if (i2cFile < 0)
        {
            return make_tuple(0.0, 0.0, 0.0);
        }

        return readFromDevice();
    }
};

#endif //PROJECT_2_BMEMANAGER_HPP
