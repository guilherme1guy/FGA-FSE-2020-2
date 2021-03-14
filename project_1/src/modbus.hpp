#ifndef MODBUS_H_
#define MODBUS_H_

#include <vector>
#include <string>

using namespace std;

class ModbusMessage
{
private:
    bool is_crc_generated;

public:
    unsigned char device_address; // 1 byte
    unsigned char operation_code; // 1 byte

    vector<unsigned char> data; // data_length bytes

    short crc; // 2 bytes

    // total bytes =  4 + data_length bytes

    ModbusMessage(unsigned char device_address, unsigned char operation_code, vector<unsigned char> data);

    short get_crc();

    tuple<unsigned char*, int> get_raw_message();

    static void send(ModbusMessage* message);
    static void decode(unsigned char* raw_message, int size);

    static ModbusMessage* create_request_int();
    static ModbusMessage* create_request_float();
    static ModbusMessage* create_request_string();

    static ModbusMessage* create_send_int(int number);
    static ModbusMessage* create_send_float(float number);
    static ModbusMessage* create_send_string(string str);
};


#endif