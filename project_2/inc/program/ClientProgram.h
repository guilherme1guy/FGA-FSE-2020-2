#ifndef PROJECT_2_CLIENTPROGRAM_H
#define PROJECT_2_CLIENTPROGRAM_H

#include "Program.h"
#include "connection/Message.h"
#include "connection/MessageCreator.h"
#include "gpio/GPIOConnection.h"
#include "i2c/BMEManager.h"
#include <map>
#include <vector>
#include "gpio/AlarmWatchdog.h"

class ClientProgram : public Program
{
    // Program control when running in client mode

private:
    string masterIP;
    int masterPort;

    map<int, GPIOConnection *> inputGpioDevices;
    map<int, GPIOConnection *> outputGpioDevices;
    vector<AlarmWatchdog *> alarmWatchdogs;

    Client getClient()
    {
        return Client(masterIP, masterPort);
    }

    void identifyOnServer()
    {

        Message m = MessageCreator::identifyClientMessage(this->server->getServerPort());
        Message r = getClient().sendMessage(m.encode());

        if (r.type == Constants::ERROR)
        {
            Logger::logToScreen("Failed to Identify on server! Retrying in 3 seconds...");

            this_thread::sleep_for(chrono::seconds(3));

            identifyOnServer();
        }
    }

    void disconnectFromServer()
    {
        Message m = MessageCreator::disconnectMessage(this->server->getServerPort());
        getClient().sendMessage(m.encode()); // in this case the response is ignored
                                             // since we are, most likely, closing the client
    }

protected:
    string _handleMessage(Message message)
    {
        Logger::logToScreen("Client Handling message");

        return "";
    }

public:
    ClientProgram(string masterIP, int masterPort)
    {

        this->masterIP = masterIP;
        this->masterPort = masterPort;

        server = new Server(0);
        server->start(Program::handleMessage, (void *)this);
    }

    ~ClientProgram()
    {
        disconnectFromServer();

        auto it = alarmWatchdogs.begin();
        while (it != alarmWatchdogs.end())
        {
            auto *awPtr = *it;
            it = alarmWatchdogs.erase(it);
            delete awPtr;
        }

        // delete input gpio
        for (auto it = inputGpioDevices.cbegin(), next_it = it; it != inputGpioDevices.cend(); it = next_it)
        {
            ++next_it;
            GPIOConnection *connection = it->second;
            inputGpioDevices.erase(it);

            delete connection;
        }

        // delete output gpio
        for (auto it = outputGpioDevices.cbegin(), next_it = it; it != outputGpioDevices.cend(); it = next_it)
        {
            ++next_it;
            GPIOConnection *connection = it->second;
            outputGpioDevices.erase(it);

            delete connection;
        }
    }

    void loop()
    {
        identifyOnServer();
        Logger::logToScreen("Successfully identified...");

        vector<int> inputDevices = Constants::getInputDevices();
        vector<int> outputDevices = Constants::getOutputDevices();

        // initialize gpio devices
        for (int device : inputDevices)
        {
            inputGpioDevices[device] = new GPIOConnection(device, GPIOConnection::GPIO_INPUT);
        }

        for (int device : outputDevices)
        {
            outputGpioDevices[device] = new GPIOConnection(device, GPIOConnection::GPIO_OUTPUT);
        }

        // Setup alarm watchdog
        for (int device : inputDevices)
        {
            Client *c = new Client(masterIP, masterPort);
            AlarmWatchdog *alarmWatchdog = new AlarmWatchdog(inputGpioDevices[device], c);

            alarmWatchdogs.push_back(alarmWatchdog);
        }

        while (1)
        {
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
};

#endif // PROJECT_2_CLIENTPROGRAM_H
