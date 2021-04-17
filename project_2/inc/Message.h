#ifndef PROJECT_2_MESSAGE_H
#define PROJECT_2_MESSAGE_H

#include <string>

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

using namespace std;

class Message
{

private:
    struct sockaddr_storage sourceAddress;
    bool hasSourceAddress = false;

    void setSourceAddress(int socketID)
    {
        socklen_t length = sizeof(this->sourceAddress);
        getpeername(socketID, (struct sockaddr *)&this->sourceAddress, &length);

        hasSourceAddress = true;
    }

public:
    char type;
    string data;

    Message(char type, string data)
    {
        this->type = type;
        this->data = data;

        memset(&this->sourceAddress, 0, sizeof(this->sourceAddress));
    }

    string encode()
    {
        string m = type + data;
        return m;
    }

    static Message decode(string m, int socketID)
    {
        char type = m[0];
        string data = "";

        for (long unsigned int i = 1; i < m.size(); i++)
        {
            data += m[i];
        }

        Message message = Message(type, data);
        message.setSourceAddress(socketID);

        return message;
    }

    string getSourceAddress()
    {

        if (!hasSourceAddress)
            return "";

        char buffer[INET_ADDRSTRLEN];

        struct sockaddr_in *s = (struct sockaddr_in *)&this->sourceAddress;
        inet_ntop(AF_INET, &s->sin_addr, buffer, sizeof(buffer));

        return string(buffer);
    }
};

#endif // PROJECT_2_MESSAGE_H
