#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "modbus.hpp"
#include "BMEManager.hpp"
#include "PotentiometerManager.hpp"
#include "LCDManager.hpp"
#include "Logger.h"

using namespace std;

auto potentiometer = PotentiometerManager();
auto bme_manager = BMEManager();

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
    printf("7 - C1/C2 Read Potentiometer\n");
    printf("8 - Read BME\n");

    int option;
    printf("Select option:");
    cin >> option;

    ModbusMessage *message = nullptr;

    // variables used inside switch statement
    int i_number;
    float f_number;
    string str;

    tuple<float, float, float> bme_data;

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
    case 7:
        printf(
            "Potentiometer: TI %f .... TP %f\n",
            potentiometer.get_internal_temperature(),
            potentiometer.get_potentiometer_temperature());
        break;
    case 8:
        bme_data = bme_manager.get_data();
        printf("BME: T %f // P %f // H %f", get<0>(bme_data), get<1>(bme_data), get<2>(bme_data));
        break;
    default:
        printf("Invalid option!\n");
        break;
    }

    if (message != nullptr)
    {
        auto* response = ModbusMessage::send(message);
        
        if (response != nullptr){

            void* value = ModbusMessage::decode(response);
            // discard value (for now)
            free(value);
            delete response; 
        }

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