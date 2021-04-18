#ifndef PROJECT_2_SERVERPROGRAM_H
#define PROJECT_2_SERVERPROGRAM_H

#include "../../connection/Message.h"
#include "../../connection/MessageCreator.h"
#include "../../i2c/BMEDataEncoder.h"
#include "../../log/Logger.h"
#include "../Program.h"
#include "StateStore.h"
#include "curses.h"
#include <signal.h>
#include <algorithm>
#include <set>
#include <tuple>

class ServerProgram : public Program
{

private:
    int maxHeight;
    int maxWidth;
    int windowWidth;
    int windowHeight;

    mutex *drawLock;

    bool updateWinSize()
    {
        int h, w;
        getmaxyx(stdscr, h, w);

        if (h != maxHeight || w != maxWidth)
        {
            maxHeight = h;
            maxWidth = w;
            windowWidth = maxWidth / 3;
            windowHeight = maxHeight;

            return true;
        }

        return false;
    }

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

    void drawInfoWindow(WINDOW *win) const
    {
        while (execute)
        {
            drawLock->lock();

            wmove(win, 0, 0);

            wprintw(win, "Clients: %d\n\n", clients.size());

            for (auto clientItr = clients.begin(); clientItr != clients.end(); ++clientItr)
            {
                auto clientInfo = clientItr->first;
                auto *store = clientItr->second;

                wprintw(
                    win,
                    "Client %s:%d\n\tTemperature: %f \n\tHumidity: %f",
                    get<0>(clientInfo),
                    get<1>(clientInfo),
                    store->getTemperature(),
                    store->getTemperature());
            }

            wrefresh(win);

            drawLock->unlock();
            this_thread::sleep_for(chrono::seconds(1));
        }
    }

    void drawMenuWindow(WINDOW *win) const
    {
        while (execute)
        {
            drawLock->lock();

            wmove(win, 0, 0);
            wrefresh(win);

            drawLock->unlock();
            this_thread::sleep_for(chrono::seconds(1));
        }
    }

    void drawLogWindow(WINDOW *win) const
    {

        Logger::setLogDirectToScreen(false);

        vector<string> recentLogs;
        for (int i = 0; i < 10; i++)
        {
            recentLogs.push_back("");
        }
        long unsigned int lastLogPosition = 0;

        while (execute)
        {
            drawLock->lock();

            wmove(win, 0, 0);

            auto *logQueue = Logger::getQueue();
            if (logQueue != nullptr)
            {
                wrefresh(win);

                while (!logQueue->empty())
                {

                    auto log = logQueue->front();
                    recentLogs[lastLogPosition] = "";
                    for (char c : log)
                    {
                        recentLogs[lastLogPosition] += c;
                    }
                    logQueue->pop();

                    lastLogPosition += 1;
                    if (lastLogPosition >= recentLogs.size())
                    {
                        lastLogPosition = 0;
                    }
                }

                for (long unsigned int i = 0; i < recentLogs.size(); i++)
                {
                    long unsigned int relativeIndex = lastLogPosition + i;
                    if (relativeIndex >= recentLogs.size())
                        relativeIndex -= recentLogs.size();

                    wprintw(win, "%s\n", recentLogs[relativeIndex].c_str());
                }

                Logger::logToScreen("test" + to_string(lastLogPosition));
            }

            drawLock->unlock();
            this_thread::sleep_for(chrono::seconds(1));
        }

        Logger::setLogDirectToScreen(true);
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
        Logger::logToScreen("Cleaning up ServerProgram");

        // delete remaining StateSores
        for (auto it = clients.cbegin(), next_it = it; it != clients.cend(); it = next_it)
        {
            ++next_it;
            auto *ptr = it->second;
            clients.erase(it);

            delete ptr;
        }

        Logger::logToScreen("ServerProgram destroyed");
    }

    void loop()
    {
        Logger::logToScreen("ServerProgram loop");
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);

        WINDOW *infoWin = newwin(windowHeight, windowWidth, 0, 0);
        WINDOW *menuWin = newwin(windowHeight, windowWidth, 0, windowWidth);
        WINDOW *logWin = newwin(windowHeight, windowWidth, 0, 2 * windowWidth);

        drawLock = new mutex();

        thread infoWinThread = thread(&ServerProgram::drawInfoWindow, this, infoWin);
        thread menuWinThread = thread(&ServerProgram::drawMenuWindow, this, menuWin);
        thread logWinThread = thread(&ServerProgram::drawLogWindow, this, logWin);

        while (execute)
        {

            bool resized = updateWinSize();
            if (resized)
            {
                drawLock->lock();

                wresize(infoWin, windowHeight, windowWidth);
                mvwin(infoWin, 0, 0);

                wresize(menuWin, windowHeight, windowWidth);
                mvwin(menuWin, 0, windowWidth);

                wresize(logWin, windowHeight, windowWidth);
                mvwin(logWin, 0, 2 * windowWidth);

                drawLock->unlock();
            }

            this_thread::sleep_for(chrono::milliseconds(100));
        }

        infoWinThread.join();
        menuWinThread.join();
        logWinThread.join();

        delwin(infoWin);
        delwin(menuWin);
        delwin(logWin);

        delete drawLock;

        refresh();

        endwin();
        Logger::logToScreen("ServerProgram loop ended");

        safeStop = true;
    }
};

#endif // PROJECT_2_SERVERPROGRAM_H
