#ifndef PROJECT_2_CLIENTPROGRAM_H
#define PROJECT_2_CLIENTPROGRAM_H

#include "Program.h"
#include "GPIOConnection.h"
#include "BMEManager.h"
#include "Message.h"
#include "MessageCreator.h"

class ClientProgram : public Program
{
    // Program control when running in client mode

private:
    string masterIP;
    int masterPort;

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

    void alarmAlert(int sensorID)
    {

        Message m = MessageCreator::alarmAlertMessage(sensorID);

        int rType;
        // keep sending until server ACK
        do
        {

            Message r = getClient().sendMessage(m.encode());
            rType = r.type;
        } while (rType != Constants::ACK);
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
    }

    void loop()
    {
        identifyOnServer();
        Logger::logToScreen("Successfully identified...");

        // TODO: Initialize GPIO devices

        // TODO: Setup alarm watchdog

        while (1)
        {
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
};

#endif // PROJECT_2_CLIENTPROGRAM_H
