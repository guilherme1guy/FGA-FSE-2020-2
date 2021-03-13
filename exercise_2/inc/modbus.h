#ifndef MODBUS_H_
#define MODBUS_H_

struct tModbusCommand
{
    unsigned char device_address; // 1 byte
    unsigned char operation_code; // 1 byte

    unsigned char *data; // data_length bytes
    int data_length;     // data length already includes the initial type definition byte
                         // its minimal value is 1

    short crc; // 2 bytes

    // total bytes =  4 + data_length bytes
};
typedef struct tModbusCommand tModbusCommand;

void modbus_free_command(tModbusCommand *command);

int get_raw_length(tModbusCommand *command);
unsigned char *get_raw_command(tModbusCommand *command);

short get_crc(tModbusCommand *command);

void modbus_send(tModbusCommand *command);

tModbusCommand *modbus_create_command(unsigned char device_address, unsigned char operation_code, unsigned char *data, int data_length);
void modbus_decode(unsigned char* raw_command, int size);

tModbusCommand *modbus_create_request_int();
tModbusCommand *modbus_create_request_float();
tModbusCommand *modbus_create_request_string();

tModbusCommand *modbus_create_send_int(int number);
tModbusCommand *modbus_create_send_float(float number);
tModbusCommand *modbus_create_send_string(char *string);

#endif