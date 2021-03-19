
#include <vector>

#ifndef PROJECT_1_POTENTIOMETER_HPP
#define PROJECT_1_POTENTIOMETER_HPP

using namespace std;

class PotentiometerManager {

    // this class manages Potentiometer temperatures collected through modbus
    // internal temperature
    // potentiometer temperature

private:
    const vector<unsigned char> REQUEST_BYTES = {0x3, 0x1, 0x8, 0x6};

    vector<float> internal_temperatures;
    vector<float> potentiometer_temperatures;

    float update_temperature(unsigned char operation_code, vector<float>* save_vector);

public:
    PotentiometerManager() = default;

    vector<float> get_all_internal_temperatures();
    vector<float> get_all_potentiometer_temperatures();

    float get_internal_temperature();
    float get_potentiometer_temperature();

};

#endif //PROJECT_1_POTENTIOMETER_HPP
