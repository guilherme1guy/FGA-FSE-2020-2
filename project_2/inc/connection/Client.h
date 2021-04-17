#ifndef PROJECT_2_CLIENT_H
#define PROJECT_2_CLIENT_H

#include "Constants.h"
#include "Message.h"
#include "MessageCreator.h"
#include "log/Logger.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

class Client
{

private:
    struct sockaddr_in serverAddress;
    unsigned short serverPort;

    int doConnection()
    {
        int clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (clientSocket < 0)
        {
            Logger::logToScreen("socket error\n");
            return -1;
        }

        if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
        {
            Logger::logToScreen("connect error\n");
            return -1;
        }

        return clientSocket;
    }

    Message getResponse(int clientSocket)
    {

        // receive response
        int totalReceivedBytes = 0;
        int receivedBytes = 0;
        char buffer[16];

        if ((receivedBytes = recv(clientSocket, buffer, 16 - 1, 0)) <= 0)
        {
            Logger::logToScreen("Error receiving data\n");
            return MessageCreator::errorMessage();
        }

        totalReceivedBytes += receivedBytes;
        buffer[receivedBytes] = '\0';

        string response = string(buffer);
        return Message::decode(response, clientSocket);
    }

public:
    Client(string ip, unsigned short port)
    {

        serverPort = port;

        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());
        serverAddress.sin_port = htons(serverPort);
    }

    Message sendMessage(string message)
    {
        // establish connection
        int clientSocket = doConnection();

        if (clientSocket == -1)
        {
            return MessageCreator::errorMessage();
        }

        // send message
        int messageSize = message.size();
        if (send(clientSocket, message.c_str(), messageSize, 0) != messageSize)
        {
            Logger::logToScreen("Error sending message: sent bytes and message size mismatch \n");
            return MessageCreator::errorMessage();
        }

        Message response = getResponse(clientSocket);

        close(clientSocket);

        return response;
    }
};

#endif // PROJECT_2_CLIENT_H
