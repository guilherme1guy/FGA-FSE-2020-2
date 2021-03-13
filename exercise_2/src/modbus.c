#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crc16.h>
#include <uart.h>
#include <endian_utils.h>

// self include to get tModbusCommand definition
#include <modbus.h>

void modbus_free_command(tModbusCommand *command)
{

    if (command->data != NULL)
    {
        free(command->data);
    }

    free(command);
}

int get_raw_length(tModbusCommand *command)
{
    // if the crc was not calculated the raw command will no contain it
    // thus the length 'loses' 2 bytes
    int length = (!command->crc ? 2 : 4) + command->data_length;

    return length;
}

unsigned char *get_raw_command(tModbusCommand *command)
{

    int allocation_size = get_raw_length(command);

    unsigned char *raw_command = calloc(allocation_size, sizeof(char));

    raw_command[0] = command->device_address;
    raw_command[1] = command->operation_code;

    for (int i = 0; i < command->data_length; i++)
    {
        raw_command[2 + i] = command->data[i];
    }

    if (command->crc)
    {
        memcpy(&(raw_command[2 + command->data_length]), &(command->crc), 2 * sizeof(unsigned char));
    }

    return raw_command;
}

short get_crc(tModbusCommand *command)
{

    if (command->crc)
    {
        return command->crc;
    }

    unsigned char *raw_command = get_raw_command(command);
    short crc = compute_CRC(raw_command, get_raw_length(command));
    //short crc = ALT_CRC16(raw_command, get_raw_length(command));

    free(raw_command);

    return crc;
}

tModbusCommand *modbus_create_command(unsigned char device_address, unsigned char operation_code, unsigned char *data, int data_length)
{

    tModbusCommand *command = calloc(1, sizeof(tModbusCommand));

    command->device_address = device_address;
    command->operation_code = operation_code;
    command->data = data;
    command->data_length = data_length;

    command->crc = get_crc(command);

    return command;
}

void modbus_decode(unsigned char *raw_command, int size)
{

    int is_crc_valid = validate_CRC(raw_command, size);

    if (!is_crc_valid)
    {
        printf("Invalid CRC for received message!\n");
        return;
    }

    int actual_data_length = (size - 4); // removes 2 bytes for op code
                                         // and device addres and 2 for crc
    unsigned char *data_copy = calloc(actual_data_length, sizeof(unsigned char));

    memcpy(&raw_command[2], data_copy, actual_data_length);

    tModbusCommand *interpreted_command = modbus_create_command(
        raw_command[0],
        raw_command[1],
        data_copy,
        actual_data_length);


    int string_length;
    char *received_string;
    switch (interpreted_command->operation_code)
    {
    case 0xA1:
        printf("Received INT: %d\n", (int)*interpreted_command->data);
        break;
    case 0xA2:
        printf("Received FLOAT: %f\n", (double)*interpreted_command->data);
        break;
    case 0xA3:

        string_length = interpreted_command->data[0];
        received_string = calloc(string_length + 1, sizeof(char)); //adds 1 for '\0'
    
        memcpy(&interpreted_command->data[1], received_string, string_length);
    
        received_string[string_length+1] = '\0'; // set last char as '\0'
    
        printf("Received STRING: %s", received_string);

        free(received_string);
        break;
    default:
        printf("Invalid operation when deconding message!\n");
    }
}

void modbus_send(tModbusCommand *command)
{

    unsigned char *raw_command = get_raw_command(command);
    int raw_length = get_raw_length(command);

    printf("RAW: [ ");
    for (int i = 0; i < raw_length; i++)
    {
        printf("0x%02X ", raw_command[i]);
    }
    printf("]\n");

    uart_communication(raw_command, raw_length);

    free(raw_command);
}

tModbusCommand *modbus_create_request_int()
{

    unsigned char *data = calloc(1, sizeof(unsigned char));
    data[0] = 0xA1;
    tModbusCommand *command = modbus_create_command(0x01, 0x23, data, 1);

    return command;
}

tModbusCommand *modbus_create_request_float()
{

    unsigned char *data = calloc(1, sizeof(unsigned char));
    data[0] = 0xA2;
    tModbusCommand *command = modbus_create_command(0x01, 0x23, data, 1);

    return command;
}

tModbusCommand *modbus_create_request_string()
{

    unsigned char *data = calloc(1, sizeof(unsigned char));
    data[0] = 0xA3;
    tModbusCommand *command = modbus_create_command(0x01, 0x23, data, 1);

    return command;
}

tModbusCommand *modbus_create_send_int(int number)
{

    unsigned char *data = calloc(5, sizeof(unsigned char));
    data[0] = 0xB1;

    unsigned char *converted_endian = endian_swap((unsigned char *)&number, sizeof(int));

    memcpy(&(data[1]), converted_endian, sizeof(int));
    tModbusCommand *command = modbus_create_command(0x01, 0x16, data, 5);

    free(converted_endian);

    return command;
}

tModbusCommand *modbus_create_send_float(float number)
{

    unsigned char *data = calloc(5, sizeof(unsigned char));
    data[0] = 0xB2;

    unsigned char *converted_endian = endian_swap((unsigned char *)&number, sizeof(float));

    memcpy(&(data[1]), converted_endian, sizeof(float));
    tModbusCommand *command = modbus_create_command(0x01, 0x16, data, 5);

    free(converted_endian);

    return command;
}

tModbusCommand *modbus_create_send_string(char *string)
{
    // string must be a properly terminated string!
    // this function does not free string
    // it creates a copy of it on 'data'
    // it is safe to free string after this function executes

    int size = strlen(string);

    if (size > 255)
    {
        // cap at maximum string size, ignoring extra text
        size = 255;
    }

    // 1 byte for operation id
    // 1 byte for length (max 255 == FF)
    // the rest is for the string
    int data_size = 2 + size;
    unsigned char *data = calloc(data_size, sizeof(unsigned char));
    data[0] = 0xB3;

    data[1] = (char)size;

    for (int i = 0; i < size; i++)
    {
        data[2 + i] = string[i];
    }

    tModbusCommand *command = modbus_create_command(0x01, 0x16, data, data_size);

    return command;
}
