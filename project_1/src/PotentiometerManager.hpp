
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
    float update_temperature(unsigned char operation_code);

public:
    PotentiometerManager() = default;
    float get_internal_temperature();
    float get_potentiometer_temperature();

};

#endif //PROJECT_1_POTENTIOMETER_HPP
