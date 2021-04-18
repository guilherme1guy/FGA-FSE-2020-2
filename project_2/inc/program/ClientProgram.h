#ifndef PROJECT_2_CLIENTPROGRAM_H
#define PROJECT_2_CLIENTPROGRAM_H

#include "Program.h"
#include "connection/Message.h"
#include "connection/MessageCreator.h"
#include "gpio/GPIOConnection.h"
#include "i2c/BMEManager.h"
#include <map>
#include <vector>
#include <thread>
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

    void updateTemperatureAndHumidity(ClientProgram *t) const
    {

        BMEManager bme;

        while (execute)
        {
            auto now = chrono::system_clock::now();

            Logger::logToScreen("Sending update to master server...");

            auto i2cValues = bme.getData();

            Message m = MessageCreator::updateMessage(get<0>(i2cValues), get<2>(i2cValues));
            t->getClient().sendMessage(m.encode());

            auto s = chrono::duration_cast<chrono::seconds>(now.time_since_epoch());
            auto next_wake = chrono::system_clock::time_point(++s);
            this_thread::sleep_until(next_wake);
        }
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

        // every 1 second the master server should be updated about temperature and humidity
        thread updateThread = thread(
            &ClientProgram::updateTemperatureAndHumidity, this, this);

        while (execute)
        {
            // wait for program end
            this_thread::sleep_for(chrono::seconds(1));
        }

        // execute will be false at this point
        // so we can wait for thread join
        updateThread.join();
    }
};

#endif // PROJECT_2_CLIENTPROGRAM_H
