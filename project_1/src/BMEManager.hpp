#ifndef PROJECT_1_BMEMANAGER_HPP
#define PROJECT_1_BMEMANAGER_HPP

#include "bme280.h"
#include <tuple>

using namespace std;

class BMEManager {

    int open_device();

    // temperature, pression, humidity
    tuple<float, float, float> read_from_device();

public:
    BMEManager() = default;

    tuple<float, float, float> get_data();
};


#endif //PROJECT_1_BMEMANAGER_HPP
