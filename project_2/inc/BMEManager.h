#ifndef PROJECT_2_BMEMANAGER_HPP
#define PROJECT_2_BMEMANAGER_HPP

#include "external/bme280.h"
#include <tuple>

#include "Logger.h"

using namespace std;

class BMEManager {

    int open_device() {
        return bme280Init(1, 0x76);
    }

    // temperature, pression, humidity
    tuple<float, float, float> read_from_device() {

        int i_temperature, i_pressure, i_humidity;
        float temperature, pressure, humidity;

        bme280ReadValues(&i_temperature, &i_pressure, &i_humidity);

        // value conversion for float
        temperature = (float) i_temperature/100.0;
        i_humidity =  (float) i_pressure / 256.0;
        humidity = (float) i_humidity / 1024.0;

        return make_tuple(temperature, pressure, humidity);
    }

public:
    BMEManager() = default;

    tuple<float, float, float> get_data() {

        int open_status = open_device();

        if (open_status != 0){
            Logger::log_to_screen("Error openning BME!");
            return make_tuple(0.0, 0.0, 0.0);
        }

        return read_from_device();
    }

};


#endif //PROJECT_2_BMEMANAGER_HPP
