// based on https://github.com/bitbank2/BitBang_I2C

#include "BMEManager.hpp"

#include <tuple>

#include "bme280.h"
#include "Logger.h"

using namespace std;


int BMEManager::open_device() {
    return bme280Init(1, 0x76);
}

tuple<float, float, float> BMEManager::read_from_device() {

    int i_temperature, i_pressure, i_humidity;
    float temperature, pressure, humidity;

    bme280ReadValues(&i_temperature, &i_pressure, &i_humidity);
  
    // value conversion for float
    temperature = (float) i_temperature/100.0;
    i_humidity =  (float) i_pressure / 256.0;
    humidity = (float) i_humidity / 1024.0;

    return make_tuple(temperature, pressure, humidity);
}

tuple<float, float, float> BMEManager::get_data() {

    int open_status = open_device();

    if (open_status != 0){
        Logger::log_to_screen("Error openning BME!");
        return make_tuple(0.0, 0.0, 0.0);
    }

    return read_from_device();    
}
