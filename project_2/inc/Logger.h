#ifndef PROJECT_2_LOGGER_HPP
#define PROJECT_2_LOGGER_HPP

#include <iostream>
#include <string>
#include <queue>
#include <list>
#include <thread>
#include <mutex>
#include <sstream>
#include <fstream>
#include "LogWriter.h"

using namespace std;

class Logger final {

private:

    LogWriter* loggerWriter;

    // SINGLETON PATTERN
    static Logger* instance_;

    // private constructor
    Logger(){
        loggerWriter = new LogWriter();
    }

    // END OF SINGLETON PATTERN

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

public:

    ~Logger() {
        delete this->loggerWriter;
        instance_ = nullptr;
    }

    // SINGLETON PATTERN
    // reference: https://github.com/hnrck/singleton_example
    static Logger *getInstance() {

        if (instance_ == nullptr){
            instance_ = new Logger();
        }

        return instance_;
    }

    // Deleted copy constructor.
    // Only the getInstance class method is allowed to give a logger.
    Logger(const Logger &) = delete;
    // Deleted copy assignment.
    // Only the getInstance class method is allowed to give a logger.
    void operator=(const Logger &) = delete;
    // Default move constructor.
    Logger(Logger &&) noexcept = default;
    // Default move assignment.
    Logger &operator=(Logger &&) noexcept = default;
    // END OF SINGLETON PATTERN


    static void logToScreen(const string& log_text)
    {
        stringstream s;
        s << getPrefix();
        s << log_text << '\n';

        cout << s.str();
    }

    static void logToFile(const string& text)
    {
        getInstance()->loggerWriter->log(text);
    }
};

#endif //PROJECT_2_LOGGER_HPP
