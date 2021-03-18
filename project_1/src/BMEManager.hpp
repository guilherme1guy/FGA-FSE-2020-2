#ifndef PROJECT_1_BMEMANAGER_HPP
#define PROJECT_1_BMEMANAGER_HPP

#include "bme280.h"
#include <tuple>

using namespace std;

class BMEManager {

    void open_device();

    // temperature, pression, humidity
    tuple<float, float, float> read_from_device();

public:
    BMEManager();


};


#endif //PROJECT_1_BMEMANAGER_HPP
