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

    void identifyOnServer()
    {

        Message m = MessageCreator::identifyClientMessage(this->server->getServerPort());

        Client *c = new Client(masterIP, masterPort);
        Message r = c->sendMessage(m.encode());

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

        Client *c = new Client(masterIP, masterPort);
        Message r = c->sendMessage(m.encode());
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
    }
};

#endif // PROJECT_2_CLIENTPROGRAM_H
