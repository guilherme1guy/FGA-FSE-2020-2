#ifndef PROJECT_2_SERVERPROGRAM_H
#define PROJECT_2_SERVERPROGRAM_H

#include "../../connection/Message.h"
#include "../../connection/MessageCreator.h"
#include "../../i2c/BMEDataEncoder.h"
#include "../../log/Logger.h"
#include "../Program.h"
#include "StateStore.h"
#include <algorithm>
#include <set>
#include <tuple>

class ServerProgram : public Program
{

private:
    map<tuple<string, int>, StateStore *> clients;

    void removeClientIfPresent(string sourceAddress, int clientPort)
    {
        auto t = make_tuple(sourceAddress, clientPort);
        if (clients.find(t) != clients.end())
        {
            auto *stateStorePtr = clients[t];

            // remove client
            clients.erase(t);

            // delete StateStore
            delete stateStorePtr;
        }
    }

protected:
    string doIdentify(Message message)
    {
        string sourceAddress = message.getSourceAddress();
        int port = atoi(message.data.c_str());

        // if client is reconnecting, remove previous StateStore
        removeClientIfPresent(sourceAddress, port);

        clients[make_tuple(sourceAddress, port)] = new StateStore();

        Logger::logToScreen("Client " + sourceAddress + ":" + to_string(port) + " connected");

        return MessageCreator::ackMessage().encode();
    }

    string doDisconnect(Message message)
    {
        string sourceAddress = message.getSourceAddress();
        int port = atoi(message.data.c_str());

        removeClientIfPresent(sourceAddress, port);

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

    string doUpdateTemperatureHumidity(Message m)
    {
        auto values = BMEDataEncoder::decodeTemperatureHumidity(m.data);

        int clientPort = get<0>(values);
        string sourceAddress = m.getSourceAddress();
        auto t = make_tuple(sourceAddress, clientPort);

        if (clients.find(t) == clients.end())
        {
            Logger::logToScreen(
                "Error: Client " + sourceAddress + ":" + to_string(clientPort) +
                " tried to update before identifying");

            return MessageCreator::errorMessage().encode();
        }

        clients[t]->setTemperature(get<1>(values));
        clients[t]->setHumidity(get<2>(values));

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

    ~ServerProgram()
    {
        // delete remaining StateSores
        for (auto it = clients.cbegin(), next_it = it; it != clients.cend(); it = next_it)
        {
            ++next_it;
            auto *ptr = it->second;
            clients.erase(it);

            delete ptr;
        }
    }
};

#endif // PROJECT_2_SERVERPROGRAM_H
