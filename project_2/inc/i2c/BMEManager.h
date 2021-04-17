#ifndef PROJECT_2_BMEMANAGER_HPP
#define PROJECT_2_BMEMANAGER_HPP

#include <tuple>

#include "external/bme280.h"

#include "log/Logger.h"

using namespace std;

class BMEManager {

    int openDevice() {
        return bme280Init(1, 0x76);
    }

    // temperature, pression, humidity
    tuple<float, float, float> readFromDevice() {

        int iTemperature, iPressure, iHumidity;
        float temperature, pressure, humidity;

        bme280ReadValues(&iTemperature, &iPressure, &iHumidity);

        // value conversion for float
        temperature = (float) iTemperature / 100.0;
        iHumidity = (float) iPressure / 256.0;
        humidity = (float) iHumidity / 1024.0;

        return make_tuple(temperature, pressure, humidity);
    }

public:
    BMEManager() = default;

    tuple<float, float, float> getData() {

        int open_status = openDevice();

        if (open_status != 0){
            Logger::logToScreen("Error openning BME!");
            return make_tuple(0.0, 0.0, 0.0);
        }

        return readFromDevice();
    }

};


#endif //PROJECT_2_BMEMANAGER_HPP
