
#include "PotentiometerManager.hpp"
#include "modbus.hpp"

#include <vector>

float PotentiometerManager::update_temperature(unsigned char operation_code)
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

    // request is automatically deleted (cpp mem management)
    delete response;

    return _local_received_data;
}

float PotentiometerManager::get_internal_temperature()
{
    return this->update_temperature(0xC1);
}

float PotentiometerManager::get_potentiometer_temperature()
{
    return this->update_temperature(0xC2);
}
