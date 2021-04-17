#ifndef PROJECT_2_SERVER_H
#define PROJECT_2_SERVER_H

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "Program.h"
#include "Logger.h"

using namespace std;

class Server
{

private:
    int serverSocket;
    struct sockaddr_in serverAddress;
    unsigned short serverPort;

    bool open = false;

    thread *serverThread;
    bool keepExecution;

    void execute(void (*handleFunction)(void *, int), void *programPtr) const
    {

        int clientSocket;
        struct sockaddr_in clientAddress;
        unsigned int clientLength;

        while (keepExecution)
        {

            clientLength = sizeof(clientAddress);
            clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLength);

            if (clientSocket < 0)
            {
                Logger::logToScreen("Server failed to accept\n");
                continue;
            }

            string addressString = string(inet_ntoa(clientAddress.sin_addr));
            Logger::logToScreen("Client Connection:" + addressString + " \n");

            (*handleFunction)(programPtr, clientSocket);

            close(clientSocket);
        }
    }

public:
    Server(unsigned short port)
    {

        serverPort = port;

        serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddress.sin_port = htons(serverPort);
    }

    ~Server()
    {
        this->stop();
    }

    void start(void (*handleFunction)(void *, int), void *programPtr)
    {

        if (open)
            return;

        if (serverSocket < 0)
        {
            Logger::logToScreen("Error opening server socket");
        }

        if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
        {
            Logger::logToScreen("Error binding server address");
        }

        if (listen(serverSocket, 10) < 0)
        {
            Logger::logToScreen("Error on listening");
        }

        keepExecution = true;
        serverThread = new thread(&Server::execute, this, handleFunction, programPtr);
        open = true;
    }

    void stop()
    {

        if (!open)
            return;

        keepExecution = false;
        serverSocket = close(serverSocket);
        serverThread->join();
        delete serverThread;

        open = false;
    }

    int getServerPort()
    {
        // if the port was randmolly allocated we need to find its
        // value
        if (serverPort == 0)
        {
            socklen_t addrlen = sizeof(serverAddress);
            getsockname(serverSocket, (struct sockaddr *)&serverAddress, &addrlen);

            serverPort = ntohs(serverAddress.sin_port);
        }

        return serverPort;
    }
};

#endif //PROJECT_2_SERVER_H
