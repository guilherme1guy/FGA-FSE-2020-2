
#include "PotentiometerManager.hpp"
#include "modbus.hpp"

#include <vector>

float PotentiometerManager::update_temperature(unsigned char operation_code, vector<float> *save_vector)
{

    vector<unsigned char> data;

    data.push_back(operation_code);
    data.insert(data.end(), this->REQUEST_BYTES.begin(), this->REQUEST_BYTES.end());

    auto request = ModbusMessage(0x01, 0x23, data);

    ModbusMessage *response = nullptr;

    do
    {
        response = ModbusMessage::send(&request);
    } while (response == nullptr); // keep trying while message is not answered
                                   // it can happen because of crc or reading errors

    float *received_data = (float *)ModbusMessage::decode(response);
    float _local_received_data = *received_data; // copy pointer value to local var
    free(received_data);                         // received data needs to be freed

    // saves received temperature on a temperature vector
    save_vector->push_back(_local_received_data);

    // request is automatically deleted (cpp mem management)
    delete response;

    return _local_received_data;
}

vector<float> PotentiometerManager::get_all_internal_temperatures()
{
    return this->internal_temperatures;
}

vector<float> PotentiometerManager::get_all_potentiometer_temperatures()
{
    return this->potentiometer_temperatures;
}

float PotentiometerManager::get_internal_temperature()
{
    return this->update_temperature(0xc1, &this->internal_temperatures);
}

float PotentiometerManager::get_potentiometer_temperature()
{
    return this->update_temperature(0xc2, &this->potentiometer_temperatures);
}
