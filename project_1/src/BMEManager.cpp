// based on https://github.com/bitbank2/BitBang_I2C

#include "BMEManager.hpp"

#include "bme280.h"

using namespace std;


void BMEManager::open_device() {
    int opened = bme280Init(1, 0x76);

    if (!opened){
        //error
    }
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
