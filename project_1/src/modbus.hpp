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

    vector<unsigned char> get_raw_message();

    static ModbusMessage* send(ModbusMessage* message);
    static void* decode(ModbusMessage* message);

    static ModbusMessage* create_request_int();
    static ModbusMessage* create_request_float();
    static ModbusMessage* create_request_string();

    static ModbusMessage* create_send_int(int number);
    static ModbusMessage* create_send_float(float number);
    static ModbusMessage* create_send_string(string str);

    static ModbusMessage* from_pointer(unsigned char* raw_message, int size);


    static vector<unsigned char> u_char_pointer_to_vector(unsigned char* origin, int size);
    static unsigned char* u_char_vector_to_u_char_pointer(vector<unsigned char> origin);
};


#endif