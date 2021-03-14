#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <utility>
#include <tuple>
#include <string>
#include <iostream>

#include "uart.hpp"

#include "crc16.hpp"


// self include to get tModbusCommand definition
#include "modbus.hpp"

ModbusMessage::ModbusMessage(unsigned char device_address, unsigned char operation_code, vector<unsigned char> data) {

    this->device_address = device_address;
    this->operation_code = operation_code;
    this->data = data;

    this->is_crc_generated = false;
    this->crc = this->get_crc();
    this->is_crc_generated = true;

}

short ModbusMessage::get_crc() {

    if (this->is_crc_generated)
    {
        return this->crc;
    }

    auto raw_message = this->get_raw_message();
    short computed_crc = compute_CRC(get<0>(raw_message), get<1>(raw_message));

    free(get<0>(raw_message));

    return computed_crc;
}

tuple<unsigned char*, int> ModbusMessage::get_raw_message() {
    // raw_message needs to be freed after its use!

    // using a list internally to manage byte insertion
    std::vector<unsigned char> byte_list;

    byte_list.push_back(this->device_address);
    byte_list.push_back(this->operation_code);

    for (unsigned char c : this->data)
    {
        byte_list.push_back(c);
    }

    if (this->is_crc_generated)
    {
        // unsigned char crc_bytes[2];
        // memcpy(&(this->crc), &(crc_bytes), 2 * sizeof(unsigned char));

        byte_list.push_back(*((unsigned char *) &(this->crc)));
        byte_list.push_back(*(((unsigned char *) &(this->crc)) + 1));
    }

    // convert list to a memory region (and returns pointer)
    auto* raw_message = (unsigned char *)calloc(byte_list.size(), sizeof (unsigned char));
    copy(byte_list.begin(), byte_list.end(), raw_message);

    return make_tuple(raw_message, byte_list.size());
}

void ModbusMessage::send(ModbusMessage* message) {

    auto raw_message_tuple = message->get_raw_message();
    auto raw_message = get<0>(raw_message_tuple);
    auto size = get<1>(raw_message_tuple);

    printf("RAW: [ ");
    for (int i = 0; i < size; i++)
    {
        printf("0x%02X ", raw_message[i]);
    }
    printf("]\n");

    uart_communication(raw_message, size);

    free(raw_message);
}

void ModbusMessage::decode(unsigned char *raw_message, int size) {

    int is_crc_valid = validate_CRC(raw_message, size);

    if (!is_crc_valid)
    {
        printf("Invalid CRC for received message!\n");
        return;
    }

    int actual_data_length = (size - 4); // removes 2 bytes for op code and device address and 2 for crc
    vector<unsigned char> data;
    for(int i = 0; i < actual_data_length; i++){
        data.push_back(raw_message[2 + 1]);
    }

    auto interpreted_message = ModbusMessage(
            raw_message[0],
            raw_message[1],
            data);


    // preemptive variable declaration
    unsigned char int_bytes[2];
    int received_int;
    unsigned char float_bytes[2];
    float received_float;
    int string_length;
    string received_string;

    switch (interpreted_message.operation_code)
    {
        case 0xA1:

            int_bytes[0] = interpreted_message.data[1];
            int_bytes[1] = interpreted_message.data[0];
            received_int = *((int *)&int_bytes);
            printf("Received INT: %d\n", received_int);
            break;

        case 0xA2:

            float_bytes[0] = interpreted_message.data[1];
            float_bytes[1] = interpreted_message.data[0];
            received_float = *((float *)&float_bytes);
            printf("Received FLOAT: %f\n", received_float);
            break;

        case 0xA3:

            string_length = interpreted_message.data[0];
            for (int i = 0; i < string_length; i++){
                received_string.push_back(interpreted_message.data[0]);
            }
            cout << "Received STRING: \"" << received_string << "\"" << endl;
            break;

        default:
            printf("Invalid operation when deconding message!\n");
    }
}

ModbusMessage* ModbusMessage::create_request_int()
{
    vector<unsigned char> data;
    data.push_back(0xA1);
    return new ModbusMessage(0x01, 0x23, data);
}

ModbusMessage* ModbusMessage::create_request_float()
{
    vector<unsigned char> data;
    data.push_back(0xA2);
    return new ModbusMessage(0x01, 0x23, data);
}

ModbusMessage* ModbusMessage::create_request_string()
{
    vector<unsigned char> data;
    data.push_back(0xA3);
    return new ModbusMessage(0x01, 0x23, data);
}

ModbusMessage* ModbusMessage::create_send_int(int number)
{
    vector<unsigned char> data;
    data.push_back(0xB1);

    auto* number_bytes = (unsigned char *)&number;
    data.push_back(number_bytes[1]); // inverted order to compensate endianess
    data.push_back(number_bytes[0]);

    return new ModbusMessage(0x01, 0x16, data);
}

ModbusMessage* ModbusMessage::create_send_float(float number)
{

    vector<unsigned char> data;
    data.push_back(0xB2);
    // the same idea as an int request
    auto* number_bytes = (unsigned char *)&number;
    data.push_back(number_bytes[1]); // inverted order to compensate endianess
    data.push_back(number_bytes[0]);

    return new ModbusMessage(0x01, 0x16, data);
}

ModbusMessage* ModbusMessage::create_send_string(string str)
{
    // cap at maximum string size, ignoring extra text
    const int MAX_STR_SIZE = 200;

    int str_size = (MAX_STR_SIZE < str.length()) ? MAX_STR_SIZE : str.length();

    // 1 byte for operation id
    // 1 byte for length (max 255 == FF)
    // the rest is for the string

    vector<unsigned char> data;
    data.push_back(0xB3);
    data.push_back((char)str_size);

    for (char c : str)
    {
        data.push_back(c);
    }

    return new ModbusMessage(0x01, 0x16, data);

}


