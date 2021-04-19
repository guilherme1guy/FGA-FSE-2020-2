#ifndef PROJECT_2_MESSAGECREATOR_H
#define PROJECT_2_MESSAGECREATOR_H

#include "Message.h"
#include "Constants.h"
#include "i2c/BMEDataEncoder.h"
#include "gpio/AlarmDataEncoder.h"

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

    static Message alarmAlertMessage(int sensorID, int clientPort)
    {
        return Message(Constants::ALARM_ALERT, AlarmDataEncoder::encodeAlarm(clientPort, sensorID));
    }

    static Message updateMessage(int clientPort, float temperature, float humidity)
    {
        return Message(
            Constants::UPDATE,
            BMEDataEncoder::encodeTemperatureHumidity(clientPort, temperature, humidity));
    }

    static Message changeStateMessage(int device)
    {
        return Message(Constants::CHANGESTATE, to_string(device));
    }
};

#endif // PROJECT_2_MESSAGECREATOR_H
