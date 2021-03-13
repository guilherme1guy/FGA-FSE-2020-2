#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <modbus.h>

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