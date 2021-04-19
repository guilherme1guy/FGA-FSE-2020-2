#ifndef PROJECT_2_SERVERPROGRAM_H
#define PROJECT_2_SERVERPROGRAM_H

#include "../../connection/Message.h"
#include "../../connection/MessageCreator.h"
#include "../../i2c/BMEDataEncoder.h"
#include "../../gpio/AlarmDataEncoder.h"
#include "../../log/Logger.h"
#include "../Program.h"
#include "StateStore.h"
#include "curses.h"
#include <signal.h>
#include <algorithm>
#include <set>
#include <sstream>
#include <tuple>
#include <unistd.h>

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

                stringstream states;

                for (int device : Constants::getOutputDevices())
                {
                    states << "\t\t" << Constants::getDeviceName(device) << ": ";

                    if (store->getDeviceState(device) == GPIOConnection::ON_VALUE)
                    {
                        states << "ON";
                    }
                    else
                    {
                        states << "OFF";
                    }

                    states << "\n\
                    ";
                }

                wprintw(
                    win,
                    "Client %s:%d\n\
                    \tTemperature: %0.2fºC \n\
                    \tHumidity: %0.2f\n\
                    \tAlarm: %s\n\
                    \tStates: \n\
                    %s",
                    get<0>(clientInfo).c_str(),
                    get<1>(clientInfo),
                    store->getTemperature(),
                    store->getTemperature(),
                    store->getAlarmEnabled() ? "ON" : "OFF",
                    states.str().c_str());
            }

            wrefresh(win);
            wclear(win);

            drawLock->unlock();
            this_thread::sleep_for(chrono::seconds(1));
        }
    }

    void drawMenuWindow(WINDOW *win, ServerProgram *p) const
    {
        tuple<string, int> selected_client = make_tuple("", -1);
        bool hasSelectedClient = false;

        const int MAIN_MENU = 0;
        const int SELECT_CLIENT = 1;
        const int CHANGE_STATE = 2;
        const int CHANGE_ALARM_STATE = 4;
        const int QUIT_MENU = 5;

        int currentMenuMode = MAIN_MENU;

        while (p->execute)
        {
            drawLock->lock();

            wmove(win, 0, 0);

            // get next char from input buffer
            // returns ERR if empty
            char c = getch();
            string announcement = "";

            wprintw(
                win,
                "\t------------------\n\t%s\n\t------------------\n",
                announcement.c_str());

            if (currentMenuMode == MAIN_MENU)
            {

                if (!hasSelectedClient)
                {
                    wprintw(
                        win,
                        "No Client Selected\
                        \n\nOptions:\
                        \t\nq - quit\
                        \t\n1 - Select Client");

                    if (c == '1')
                    {
                        currentMenuMode = SELECT_CLIENT;
                    }
                    else if (c == 'q')
                    {
                        currentMenuMode = QUIT_MENU;
                    }
                }
                else
                {
                    wprintw(
                        win,
                        "Client: %s:%d\n\t\
                        Options:\
                        \t\nq - quit\
                        \t\n1 - Change Client\
                        \n\t2 - Change state\
                        \n\t3 - Change alarm state",
                        get<0>(selected_client).c_str(),
                        get<1>(selected_client));

                    if (c == '1')
                    {
                        currentMenuMode = SELECT_CLIENT;
                    }
                    else if (c == '2')
                    {
                        currentMenuMode = CHANGE_STATE;
                    }
                    else if (c == '3')
                    {
                        currentMenuMode = CHANGE_ALARM_STATE;
                    }
                    else if (c == 'q')
                    {
                        currentMenuMode = QUIT_MENU;
                    }
                }
            }
            else if (currentMenuMode == SELECT_CLIENT)
            {

                stringstream clientsText;
                int clientNumber = 1;
                vector<tuple<string, int>> clientVector;

                for (auto clientItr = clients.begin(); clientItr != clients.end(); ++clientItr)
                {
                    auto clientInfo = clientItr->first;

                    clientVector.push_back(clientInfo);

                    clientsText
                        << "\n\t"
                        << (char)('0' + clientNumber) << " - " << get<0>(clientInfo)
                        << ":" << to_string(get<1>(clientInfo)) << "\
                        ";

                    clientNumber++;
                }

                if (clientNumber == 1)
                {
                    clientsText << "\n\n\tNo clients connected";
                }

                wprintw(
                    win,
                    "Select a client\
                    \n0 - Go Back:\
                    %s",
                    clientsText.str().c_str());

                if (c == '0')
                {
                    currentMenuMode = MAIN_MENU;
                }
                else if (clientNumber > 1)
                {
                    for (char option = '0' + clientNumber; option > '0'; option--)
                    {
                        if (c == option)
                        {
                            selected_client = clientVector[option - '0' - 1];
                            hasSelectedClient = true;
                            currentMenuMode = MAIN_MENU;

                            Logger::logToFile(
                                Logger::getFormattedDate() +
                                ", selectClient, " +
                                get<0>(selected_client) + ":" + to_string(get<1>(selected_client)));
                        }
                    }
                }
            }
            else if (currentMenuMode == CHANGE_STATE)
            {
                auto *storePtr = clients.at(selected_client);

                stringstream devicesText;
                int deviceNumber = 1;
                vector<int> devicesVector;

                for (auto device : Constants::getOutputDevices())
                {
                    auto deviceName = Constants::getDeviceName(device);

                    devicesVector.push_back(device);

                    devicesText
                        << "\n\t"
                        << (char)('0' + deviceNumber) << " - " << deviceName
                        << " (" << storePtr->getDeviceStateString(device) << ")"
                        << "\
                        ";

                    deviceNumber++;
                }

                wprintw(
                    win,
                    "Select a device to change state:\
                    \n0 - Go Back:\
                    %s",
                    devicesText.str().c_str());

                if (c == '0')
                {
                    currentMenuMode = MAIN_MENU;
                }
                else
                {
                    for (char option = '0' + deviceNumber; option > '0'; option--)
                    {
                        if (c == option)
                        {
                            currentMenuMode = MAIN_MENU;
                            auto selected_device = devicesVector[option - '0' - 1];

                            auto m = MessageCreator::changeStateMessage(selected_device);
                            Client c = Client(get<0>(selected_client), get<1>(selected_client));

                            auto r = c.sendMessage(m.encode());

                            if (r.type == Constants::ACK)
                            {
                                storePtr->setDeviceState(
                                    selected_device,
                                    storePtr->invertState(storePtr->getDeviceState(selected_device)));
                                announcement = "Success";
                            }
                            else if (r.type == Constants::ERROR)
                            {
                                announcement =
                                    "There was an error changing " +
                                    Constants::getDeviceName(selected_device) +
                                    " State";
                            }

                            Logger::logToFile(
                                Logger::getFormattedDate() +
                                ", changeState, " +
                                get<0>(selected_client) + ":" + to_string(get<1>(selected_client)) +
                                ", " + Constants::getDeviceName(selected_device) +
                                ", " + storePtr->getDeviceStateString(selected_device) +
                                (r.type == Constants::ACK
                                     ? ", OK"
                                     : ", Error"));
                        }
                    }
                }
            }
            else if (currentMenuMode == CHANGE_ALARM_STATE)
            {
                auto *storePtr = clients.at(selected_client);
                storePtr->setAlarmEnabled(!storePtr->getAlarmEnabled());

                announcement = "Set alarm to " + storePtr->getAlarmEnabled() ? "ON" : "OFF";
                currentMenuMode = MAIN_MENU;

                Logger::logToFile(
                    Logger::getFormattedDate() +
                    ", alarmWatch, " +
                    get<0>(selected_client) + ":" + to_string(get<1>(selected_client)) +
                    ", " + (storePtr->getAlarmEnabled() ? "ON" : "OFF"));
            }
            else if (currentMenuMode == QUIT_MENU)
            {
                p->execute = false;
                Logger::logToScreen("Quitting...");
            }

            wrefresh(win);
            wclear(win);

            drawLock->unlock();
            this_thread::sleep_for(chrono::milliseconds(250));
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
            }

            wrefresh(win);
            wclear(win);

            drawLock->unlock();
            this_thread::sleep_for(chrono::milliseconds(250));
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
        auto alertData = AlarmDataEncoder::decodeAlarm(m.data);
        int clientPort = get<0>(alertData);
        int activatedSensor = get<1>(alertData);

        string sourceAddress = m.getSourceAddress();

        auto client = make_tuple(sourceAddress, clientPort);

        auto activateAlarm = clients[client]->getAlarmEnabled();

        if (activateAlarm)
        {
            Logger::logToScreen("Sensor " + Constants::getSensorLocation(activatedSensor) + " Activated!");

            char *cwd = getcwd(NULL, 0);

            stringstream alarmCommand;
            alarmCommand << "aplay " << cwd << "/src/resource/beep.mp3 -q";
            system(alarmCommand.str().c_str());

            Logger::logToScreen(alarmCommand.str());

            free(cwd);

            Logger::logToFile(Logger::getFormattedDate() + ", alarmON, " + Constants::getSensorLocation(activatedSensor));
        }

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
        Logger::setLogDirectToScreen(false);

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

    void createThread()
    {
        Logger::logToScreen("Starting Program thread");
        execute = true;

        if (loopThread == nullptr)
        {
            loopThread = new thread(&ServerProgram::loop, this);
        }
    }

    void loop()
    {
        Logger::logToScreen("ServerProgram loop");
        initscr();
        cbreak();
        noecho();
        nodelay(stdscr, TRUE);
        keypad(stdscr, TRUE);

        WINDOW *infoWin = newwin(windowHeight, windowWidth, 0, 0);
        WINDOW *menuWin = newwin(windowHeight, windowWidth, 0, windowWidth);
        WINDOW *logWin = newwin(windowHeight, windowWidth, 0, 2 * windowWidth);

        drawLock = new mutex();

        thread infoWinThread = thread(&ServerProgram::drawInfoWindow, this, infoWin);
        thread menuWinThread = thread(&ServerProgram::drawMenuWindow, this, menuWin, this);
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
