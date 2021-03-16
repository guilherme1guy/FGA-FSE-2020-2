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

ModbusMessage* ModbusMessage::from_pointer(unsigned char *raw_message, int size) {

    // creates a early copy of the message, before any operations are made
    auto raw_message_vector = ModbusMessage::u_char_pointer_to_vector(raw_message, size);

    int is_crc_valid = validate_CRC(raw_message, size);

    if (!is_crc_valid)
    {
        printf("Invalid CRC for received message!\n");
        return nullptr;
    }

    int actual_data_length = (size - 4); // removes 2 bytes for op code and device address and 2 for crc

    vector<unsigned char> local_data;
    local_data.reserve(actual_data_length);
    for(int i = 0; i < actual_data_length; i++){
        local_data.push_back(raw_message_vector[2 + i]);
    }

    return new ModbusMessage(
            raw_message_vector[0],
            raw_message_vector[1],
            local_data);
}

short ModbusMessage::get_crc() {

    if (this->is_crc_generated)
    {
        return this->crc;
    }

    auto raw_message = this->get_raw_message();
    auto* message_pointer = ModbusMessage::u_char_vector_to_u_char_pointer(raw_message);
    short computed_crc = compute_CRC(message_pointer, raw_message.size());

    free(message_pointer);

    return computed_crc;
}

vector<unsigned char> ModbusMessage::get_raw_message() {

    std::vector<unsigned char> raw_message;

    raw_message.push_back(this->device_address);
    raw_message.push_back(this->operation_code);

    for (unsigned char c : this->data)
    {
        raw_message.push_back(c);
    }

    if (this->is_crc_generated)
    {
        // unsigned char crc_bytes[2];
        // memcpy(&(this->crc), &(crc_bytes), 2 * sizeof(unsigned char));

        raw_message.push_back(*((unsigned char *) &(this->crc)));
        raw_message.push_back(*(((unsigned char *) &(this->crc)) + 1));
    }

    return raw_message;
}

ModbusMessage* ModbusMessage::send(ModbusMessage* message) {

    auto raw_message = message->get_raw_message();

    printf("RAW: [ ");
    for (int i = 0; i < raw_message.size(); i++)
    {
        printf("0x%02X ", raw_message[i]);
    }
    printf("]\n");

    auto* raw_message_pointer = ModbusMessage::u_char_vector_to_u_char_pointer(raw_message);
    ModbusMessage* response = uart_communication(raw_message_pointer, raw_message.size());

    free(raw_message_pointer);

    return response;
}

void* ModbusMessage::decode(ModbusMessage* message) {
    // returns a pointer to a memory region containing the decoded data
    // it needs to be freed after use
    // acces should be made casting the void* to the desired type pointer

    void* decoded_data = nullptr;

    // data[0] is the operation code (which implies on a data type)
    // INT: 0xA1
    // FLOAT: 0xA2, 0xC1, 0xC2
    // STRING: 0xA3
    switch (message->data[0])
    {
        case 0xA1:

            decoded_data = calloc(1, sizeof(int));

            // when receiving a message the program is not following
            // modbus standards for endianess (for ints and floats)
            // c/cpp uses little endian internally and modbus uses big endian
            // when sending a message I need to convert from little to big endian
            // but the message I receive has the number in little endian (thus breaking
            // the modbus standard -- I guess)
            for(int i = 0; i < 4; i++){
                // for correct endian conversion:
                // int_bytes[i] = message->data[4 - i];

                // interprest void* region as u_char to translate bytes
                // to desired data type
                ((unsigned char *)decoded_data)[i] = message->data[i + 1];
            }
            printf("Received INT: %d\n", *(int *)decoded_data);
            break;

        case 0xA2:
        case 0xC1:
        case 0xC2:

            decoded_data = calloc(1, sizeof(float));

            for(int i = 0; i < 4; i++){
                // the same thing as integers
                ((unsigned char *)decoded_data)[i] = message->data[i + 1];
            }

            printf("Received FLOAT: %f\n", *(float *)decoded_data);
            break;

        case 0xA3:

            decoded_data = calloc(1, sizeof(string));

            // data[1] represents string length
            for (int i = 0; i < message->data[1]; i++){
                ((string *)decoded_data)->push_back(message->data[2 + i]);
            }

            cout << "Received STRING: \"" << *((string *)decoded_data) << "\"" << endl;
            break;

        default:
            printf("Invalid operation code when deconding message!\n");
    }

    return decoded_data;
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

vector<unsigned char> ModbusMessage::u_char_pointer_to_vector(unsigned char *origin, int size) {

    vector<unsigned char> target_vector;
    target_vector.resize(size);
    for(int i = 0; i < size; i++){
        target_vector[i] = origin[i];
    }

    return target_vector;
}

unsigned char *ModbusMessage::u_char_vector_to_u_char_pointer(vector<unsigned char> origin) {

    // convert list to a memory region (and returns pointer)
    auto* target = (unsigned char *)calloc(origin.size(), sizeof (unsigned char));
    copy(origin.begin(), origin.end(), target);

    return target;
}




