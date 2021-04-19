#ifndef PROJECT_2_SERVER_H
#define PROJECT_2_SERVER_H

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <errno.h>

#include "log/Logger.h"
#include "program/Program.h"

using namespace std;

class Server
{

private:
    int serverSocket;
    struct sockaddr_in serverAddress;
    unsigned short serverPort;

    thread serverThread;
    mutex serverStateLock;
    bool open = false;

    void execute(void (*handleFunction)(void *, int), void *programPtr) const
    {
        Logger::logToScreen("Server executing and listening");

        while (!this->open)
        {
            this_thread::yield();
        }

        while (this->open)
        {
            struct sockaddr_in clientAddress;

            unsigned int clientLength = sizeof(clientAddress);
            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLength);

            if (clientSocket < 0)
            {
                if (this->open)
                {
                    Logger::logToScreen("Server failed to accept\n");
                }
                else
                {

                    Logger::logToScreen("Server closing\n");
                }
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
    }

    ~Server()
    {
        this->stop();
    }

    void start(void (*handleFunction)(void *, int), void *programPtr)
    {

        serverStateLock.lock();

        if (open)
        {
            serverStateLock.unlock();
            return;
        }

        serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddress.sin_port = htons(serverPort);

        if (serverSocket < 0)
        {
            Logger::logToScreen("Error opening server socket");
            cerr << "Error opening socket";
            exit(-1);
        }

        if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
        {
            Logger::logToScreen("Error binding server address");
            cerr << "Error binding server address";
            exit(-1);
        }

        if (listen(serverSocket, 10) < 0)
        {
            Logger::logToScreen("Error on listening");
            cerr << "Error on listening";
            exit(-1);
        }

        Logger::logToScreen("Server socket created");

        serverThread = thread(&Server::execute, this, handleFunction, programPtr);
        open = true;

        serverStateLock.unlock();
    }

    void stop()
    {

        serverStateLock.lock();

        Logger::logToScreen("Stopping server...");

        if (!open)
        {
            serverStateLock.unlock();
            return;
        }

        open = false;

        if (shutdown(serverSocket, SHUT_RDWR) < 0)
        {
            perror(strerror(errno));
        }

        Logger::logToScreen("Closed server socket");

        serverThread.join();

        Logger::logToScreen("Destroyed server thread");

        Logger::logToScreen("Server stoped");
        serverStateLock.unlock();
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
