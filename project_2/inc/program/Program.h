#ifndef PROJECT_2_PROGRAM_H
#define PROJECT_2_PROGRAM_H

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "curses.h"

#include "BMEManager.h"
#include "GPIOConnection.h"
#include "log/Logger.h"

#include "connection/Message.h"

#include "connection/Client.h"
#include "connection/Server.h"

using namespace std;

class Program
{

    bool execute = true;

protected:
    // server used to receive messages
    Server *server;

    virtual string _handleMessage(Message message) = 0;

public:
    virtual void loop()
    {

        Logger::logToScreen("Running...");

        while (execute)
        {
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    }

    virtual ~Program()
    {
        execute = false;

        Logger::logToScreen("Stopping server...");
        server->stop();
        delete server;

        Logger::logToScreen("Program destroyed");
    }

    static void handleMessage(void *ptr, int clientSocket)
    {
        Program *p = (Program *)ptr;

        Logger::logToScreen("Receiving message");

        // receive incoming message
        char messageBuffer[16];
        int receivedSize = recv(clientSocket, messageBuffer, 16, 0);

        if (receivedSize < 0)
        {
            Logger::logToScreen("Error reading message\n");
            return;
        }

        // process message
        string answer = p->_handleMessage(Message::decode(string(messageBuffer), clientSocket));

        // send answer
        while (send(clientSocket, answer.c_str(), receivedSize, 0) < 0)
        {
            printf("Error sending answer\n");
        }

        close(clientSocket);
    }
};

#endif //PROJECT_2_PROGRAM_H
