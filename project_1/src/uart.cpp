#include <cstdio>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART
#include <cstdlib>
#include <cstring>

#include "modbus.hpp"


ModbusMessage* uart_communication(unsigned char *data, int data_length)
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

    sleep(1);

    bool closed_stream = false;
    ModbusMessage* response = nullptr;

    //----- CHECK FOR ANY RX BYTES -----
    if (uart0_filestream != -1)
    {

        // Read up to 255 characters from the port if they are there
        unsigned char rx_buffer[256];

        int rx_length = read(uart0_filestream, (void *)rx_buffer, 255); //Filestream, buffer to store in, number of bytes to read (max)
        close(uart0_filestream); // close as early as possible
        closed_stream = true;

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

            auto* raw_commnad = (unsigned char *)calloc(rx_length, sizeof(unsigned char));
            std::memcpy(raw_commnad, rx_buffer, rx_length * sizeof(unsigned char));

            response = ModbusMessage::from_pointer(raw_commnad, rx_length);

            free(raw_commnad);
        }
    }

    if(!closed_stream){
        close(uart0_filestream);
    }

    return response;
}