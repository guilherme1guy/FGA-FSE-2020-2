#ifndef PROJECT_2_ALARMWATCHDOG_H
#define PROJECT_2_ALARMWATCHDOG_H

#include <gpio/GPIOConnection.h>
#include <connection/Client.h>
#include "Constants.h"
#include <wiringPi.h>
#include <map>

using namespace std;

class AlarmWatchdog
{

private:
    GPIOConnection *connection;
    Client *client;

    static map<int, AlarmWatchdog *> handlers;

    // hated this solution, but WiringPI gives no information
    // about the activated pin on the interruption
    // so each interruption needed to be maped to its own
    // function
    static void _handlePresence1Interrupt()
    {
        internalHandle(Constants::PRESENSE_SENSOR_1);
    }

    static void _handlePresence2Interrupt()
    {
        internalHandle(Constants::PRESENSE_SENSOR_2);
    }

    static void _handleOpening1Interrupt()
    {
        internalHandle(Constants::OPENING_SENSOR_1);
    }

    static void _handleOpening2Interrupt()
    {
        internalHandle(Constants::OPENING_SENSOR_2);
    }

    static void _handleOpening3Interrupt()
    {
        internalHandle(Constants::OPENING_SENSOR_3);
    }

    static void _handleOpening4Interrupt()
    {
        internalHandle(Constants::OPENING_SENSOR_4);
    }

    static void internalHandle(int pin)
    {
        // recover AlarmWatchdog from the handle map
        // and calls its handle function
        handlers[pin]->handle();
    }

    void handle()
    {
        // the pin this watchdog watches has changed state

        // TODO: execute calculations and checks

        Message m = MessageCreator::alarmAlertMessage(this->connection->getPin());

        int rType;
        // keep sending until server ACK
        do
        {

            Message r = client->sendMessage(m.encode());
            rType = r.type;
        } while (rType != Constants::ACK);
    }

public:
    AlarmWatchdog(GPIOConnection *connection, Client *c)
    {

        map<int, void (*)()> functions = {
            {Constants::PRESENSE_SENSOR_1, &_handlePresence1Interrupt},
            {Constants::PRESENSE_SENSOR_2, &_handlePresence2Interrupt},
            {Constants::OPENING_SENSOR_1, &_handleOpening1Interrupt},
            {Constants::OPENING_SENSOR_2, &_handleOpening2Interrupt},
            {Constants::OPENING_SENSOR_3, &_handleOpening3Interrupt},
            {Constants::OPENING_SENSOR_4, &_handleOpening4Interrupt}};

        this->connection = connection;
        this->client = c;

        int pin = connection->getPin();

        handlers[pin] = this;
        wiringPiISR(pin, INT_EDGE_BOTH, functions[pin]);
    }

    ~AlarmWatchdog()
    {

        int pin = this->connection->getPin();

        // connection will be deleted on ClientProgram
        this->connection = nullptr;
        delete client;

        if (handlers.find(pin) != handlers.end())
        {
            // remove self reference from handler map
            handlers.erase(pin);
        }
    }
};

// allocate static member
map<int, AlarmWatchdog *> AlarmWatchdog::handlers;

#endif // PROJECT_2_ALARMWATCHDOG_H
