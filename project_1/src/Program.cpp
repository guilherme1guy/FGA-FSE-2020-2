#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#include "Program.hpp"

#include "TemperatureController.hpp"

using namespace std;

Program::Program() {
    this->potentiometer = new PotentiometerManager();
    this->bme_manager = new BMEManager();
    //lcd_manager = new LCDManager();
}

Program::~Program(){
    delete this->potentiometer;
    delete this->bme_manager;
    // delete this->lcd_manager;
}

int Program::menu()
{

    cout << "\n\n\n-------------------------------\n";
    cout << "0 - Exit\n";
    cout << "1 - A1 Request Int\n";
    cout << "2 - A2 Request Float\n";
    cout << "3 - A3 Request String\n";
    cout << "4 - B1 Send Int\n";
    cout << "5 - B2 Send Float\n";
    cout << "6 - B3 Send String\n";
    cout << "7 - C1/C2 Read Potentiometer\n";
    cout << "8 - Read BME\n";

    int option;
    printf("Select option:");
    cin >> option;

    ModbusMessage *message = nullptr;

    // variables used inside switch statement
    int i_number;
    float f_number;
    string str;

    tuple<float, float, float> bme_data;

    string line1;
    string line2;

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
            potentiometer->get_internal_temperature(),
            potentiometer->get_potentiometer_temperature());
        break;
    case 8:
        bme_data = bme_manager->get_data();
        printf("BME: T %f // P %f // H %f", get<0>(bme_data), get<1>(bme_data), get<2>(bme_data));
        break;
    case 9:
        cout << "Write line 1\n >";
        cin >> line1;
        cout << "Write line 2\n >";
        cin >> line2;

        //lcd_manager->write_on_screen(line1, line2);
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

void Program::run(){

    int menu_result = 0;
    while (menu_result == 0)
    {
        menu_result = menu();
    }

}