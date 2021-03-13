#include <stdio.h>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART
#include <stdlib.h>
#include <string.h> 
#include <modbus.h>


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

            unsigned char* raw_commnad = calloc(rx_length, sizeof(unsigned char));
            memcpy(rx_buffer, raw_commnad, rx_length * sizeof(unsigned char));

            modbus_decode(raw_commnad, rx_length);
            free(raw_commnad);
        }
    }

    close(uart0_filestream);
}