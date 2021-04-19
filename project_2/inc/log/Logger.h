#ifndef PROJECT_2_LOGGER_HPP
#define PROJECT_2_LOGGER_HPP

#include <fstream>
#include <iostream>
#include <list>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <memory>
#include <stdexcept>

#include "LogWriter.h"

using namespace std;

void *globalLogger = nullptr;

class Logger
{

private:
    LogWriter *loggerWriter;
    bool logDirectToScreen;
    queue<string> logQueue;

    Logger()
    {
        logDirectToScreen = true;
        loggerWriter = new LogWriter();
    }

    ~Logger()
    {
        logDirectToScreen = true;

        // dump logs not shown on scree
        while (!logQueue.empty())
        {
            auto log = logQueue.front();
            logToScreen(log);
            logQueue.pop();
        }

        delete loggerWriter;

        loggerWriter = nullptr;
        globalLogger = nullptr;
    }

protected:
    static string getPrefix()
    {
        // returns a string with the log message prefix:
        // [TIME]<space>

        stringstream s_prefix;

        s_prefix << "[";
        s_prefix << getFormattedDate();
        s_prefix << "] ";

        return s_prefix.str();
    }

public:
    static string getFormattedDate()
    {
        // returns current time as a string:
        // YYYY/MM/DD HH:mm:SS

        auto now = chrono::system_clock::now();
        auto c_now = chrono::system_clock::to_time_t(now);

        string time_str = string(ctime(&c_now));

        // remove \n char
        time_str.replace(time_str.find('\n'), 1, "");

        return time_str;
    }

    // SINGLETON PATTERN

    static Logger *getInstance()
    {

        if (globalLogger == nullptr)
        {
            globalLogger = new Logger();
        }

        return (Logger *)globalLogger;
    }

    static void setLogDirectToScreen(bool value)
    {
        Logger *l = getInstance();
        l->logDirectToScreen = value;
    }

    static queue<string> *getQueue()
    {
        return &getInstance()->logQueue;
    }

    static void logToScreen(string logText)
    {
        if (getInstance()->logDirectToScreen)
        {

            stringstream s;
            s << getPrefix();
            s << logText << '\n';

            cout << s.str();
        }
        else
        {
            getInstance()->logQueue.push(logText);
        }
    }

    static void logToFile(const string &text)
    {
        getInstance()->loggerWriter->log(text);
    }
};

#endif //PROJECT_2_LOGGER_HPP
