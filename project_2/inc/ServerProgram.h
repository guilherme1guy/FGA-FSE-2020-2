#ifndef PROJECT_2_SERVERPROGRAM_H
#define PROJECT_2_SERVERPROGRAM_H

#include "Program.h"
#include "Logger.h"
#include "Message.h"
#include "MessageCreator.h"
#include <algorithm>
#include <set>
#include <tuple>

class ServerProgram : public Program
{

private:
    set<tuple<string, int>> clients;

protected:
    string doIdentify(Message message)
    {
        string sourceAddress = message.getSourceAddress();
        int port = atoi(message.data.c_str());

        clients.insert(make_tuple(sourceAddress, port));

        Logger::logToScreen("Client " + sourceAddress + ":" + to_string(port) + " connected");

        return MessageCreator::ackMessage().encode();
    }

    string doDisconnect(Message message)
    {
        string sourceAddress = message.getSourceAddress();
        int port = atoi(message.data.c_str());

        auto t = make_tuple(sourceAddress, port);
        if (clients.find(t) != clients.end())
        {
            clients.erase(t);
        }

        Logger::logToScreen("Client " + sourceAddress + ":" + message.data + " disconnected");

        return MessageCreator::ackMessage().encode();
    }

    string doAlarmAlert(Message m)
    {

        int activatedSensor = atoi(m.data.c_str());

        Logger::logToScreen("Sensor " + Constants::getSensorLocation(activatedSensor) + " Activated!");

        // TODO: check if alarms are enabled and play mp3

        return MessageCreator::ackMessage().encode();
    }

    string _handleMessage(Message message)
    {

        switch (message.type)
        {
        case Constants::IDENTIFY:
            return doIdentify(message);
        case Constants::DISCONNECT:
            return doDisconnect(message);
        case Constants::ALARM_ALERT:
            return doAlarmAlert(message);
        default:
            break;
        }

        return MessageCreator::ackMessage().encode();
    }

public:
    ServerProgram(int inboundPort)
    {

        server = new Server(inboundPort);
        server->start(Program::handleMessage, (void *)this);
    }
};

#endif // PROJECT_2_SERVERPROGRAM_H
