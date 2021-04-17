#ifndef PROJECT_2_MESSAGECREATOR_H
#define PROJECT_2_MESSAGECREATOR_H

#include "Message.h"
#include "Constants.h"

using namespace std;

class MessageCreator
{

public:
    static Message ackMessage()
    {
        return Message(Constants::ACK, "");
    }

    static Message errorMessage()
    {
        return Message(Constants::ERROR, "");
    }

    static Message identifyClientMessage(int clientPort)
    {
        return Message(Constants::IDENTIFY, to_string(clientPort));
    }

    static Message disconnectMessage(int clientPort)
    {
        return Message(Constants::DISCONNECT, to_string(clientPort));
    }
};

#endif // PROJECT_2_MESSAGECREATOR_H
