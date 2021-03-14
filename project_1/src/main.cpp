#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "modbus.hpp"

using namespace std;

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
    cin >> option;

    ModbusMessage *message = nullptr;

    // variables used inside switch statement
    int i_number;
    float f_number;
    string str;

    switch (option)
    {
    case 0:
        printf("Exiting...\n");
        return -1;
    case 1:
        message = ModbusMessage::create_request_int();
        break;
    case 2:
        message = ModbusMessage::create_request_float();
        break;
    case 3:
        message = ModbusMessage::create_request_string();
        break;
    case 4:
        printf("Insert Integer: ");
        cin >> i_number;
        message = ModbusMessage::create_send_int(i_number);
        break;
    case 5:
        printf("Insert Float: ");
        cin >> f_number;
        message = ModbusMessage::create_send_float(f_number);
        break;
    case 6:
        printf("Insert String: ");
        //clear input buffer
        while ((getchar()) != '\n');
        cin >> str;
        message = ModbusMessage::create_send_string(str);
        break;
    default:
        printf("Invalid option!\n");
        break;
    }

    if (message != nullptr)
    {
        ModbusMessage::send(message);
        delete message; //also frees data
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