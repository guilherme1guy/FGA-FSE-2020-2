
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART
#include <crc16.h>

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
    short crc = calcula_CRC(raw_command, get_raw_length(command));
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

void uart_communication(unsigned char *data, int data_length)
{
    int uart0_filestream = -1;

    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY); //Open in non blocking read/write mode

    if (uart0_filestream == -1)
    {
        printf("Error - Unable to initialize UART.\n");
    }
    else
    {
        printf("UART initialized!\n");
    }

    struct termios options;
    tcgetattr(uart0_filestream, &options);
    //options.c_cflag = B115200 | CS8 | CLOCAL | CREAD; //<Set baud rate
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;

    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    printf("Memory buffers created!\n");

    if (uart0_filestream != -1)
    {
        printf("Writing characters to UART ...");
        int count = write(uart0_filestream, &data[0], data_length);
        if (count < 0)
        {
            printf("UART TX error\n");
        }
        else
        {
            printf("Done writing.\n");
        }
    }

    sleep(3);

    //----- CHECK FOR ANY RX BYTES -----
    if (uart0_filestream != -1)
    {

        // Read up to 255 characters from the port if they are there
        unsigned char rx_buffer[256];

        int rx_length = read(uart0_filestream, (void *)rx_buffer, 255); //Filestream, buffer to store in, number of bytes to read (max)

        if (rx_length < 0)
        {
            printf("Reading error.\n"); //An error occured (will occur if there are no bytes)
        }
        else if (rx_length == 0)
        {
            printf("No data available.\n"); //No data waiting
        }
        else
        {
            //Bytes received
            rx_buffer[rx_length] = '\0';
            printf("%i Bytes read : ", rx_length);
            for (int i = 0; i < rx_length; i++)
            {
                printf("0x%02X ", rx_buffer[i]);
            }
            printf("\n");
        }
    }

    close(uart0_filestream);
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
    memcpy(&(data[1]), &number, sizeof(int));
    tModbusCommand *command = modbus_create_command(0x01, 0x16, data, 5);

    return command;
}

tModbusCommand *modbus_create_send_float(float number)
{

    unsigned char *data = calloc(5, sizeof(unsigned char));
    data[0] = 0xB2;
    memcpy(&(data[1]), &number, sizeof(float));
    tModbusCommand *command = modbus_create_command(0x01, 0x16, data, 5);

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

int menu()
{

    printf("\n\n\n-------------------------------\n");
    printf("0 - Exit\n");
    printf("1 - A1 Request Int\n");
    printf("2 - A2 Request Float\n");
    printf("3 - A3 Request String\n");
    printf("4 - B1 Send Int\n");
    printf("5 - B2 Send Float\n");
    printf("6 - B3 Send String\n");

    int option;
    printf("Select option:");
    scanf("%d", &option);

    tModbusCommand *command = NULL;

    // variables used inside switch statement
    int i_number;
    float f_number;
    char string[255];

    switch (option)
    {
    case 0:
        printf("Exiting...\n");
        return -1;
    case 1:
        command = modbus_create_request_int();
        break;
    case 2:
        command = modbus_create_request_float();
        break;
    case 3:
        command = modbus_create_request_string();
        break;
    case 4:
        printf("Insert Integer: ");
        scanf("%d", &i_number);
        command = modbus_create_send_int(i_number);
        break;
    case 5:
        printf("Insert Float: ");
        scanf("%f", &f_number);
        command = modbus_create_send_float(f_number);
        break;
    case 6:
        printf("Insert String: ");

        //clear input buffer
        while ((getchar()) != '\n')
            ;

        fgets(string, sizeof(string), stdin);

        // properly terminate string, since fgets reads '\n'
        char *end_prt = strchr(string, '\n');
        if (end_prt)
        {
            *end_prt = '\0';
        }

        command = modbus_create_send_string(string);
        break;
    default:
        printf("Invalid option!\n");
        break;
    }

    if (command)
    {
        modbus_send(command);
        modbus_free_command(command); //also frees data
    }

    return 0;
}

int main(int argc, const char *argv[])
{

    int menu_result = 0;
    while (menu_result == 0)
    {
        menu_result = menu();
    }

    return 0;
}