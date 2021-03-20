#ifndef PROJECT_1_LOGGER_HPP
#define PROJECT_1_LOGGER_HPP

#include <string>
#include <queue>
#include <list>
#include <thread>
#include <mutex>

using namespace std;

class Logger final {

private:
    queue<string> file_buffer;
    list<string> log_lines;

    static const int MAX_LOG_LINES = 25;

    static Logger* instance_;

    Logger();

    thread *writer_thread;
    void writer_execute();
    bool execute = true;

    static string get_prefix();
    static string get_formated_date();

    static Logger &get_instance();

protected:
    static mutex file_buffer_lock;

public:
    // src: https://github.com/hnrck/singleton_example
    ~Logger();

    Logger(const Logger &) = delete;
    void operator=(const Logger &) = delete;
    Logger(Logger &&) noexcept = default;
    Logger &operator=(Logger &&) noexcept = default;

    static void log_to_screen(string log_text);
    static void log_to_file(string text);

    static void flush_file_buffer();
    static void end_logger();

    static list<string> get_log_lines();

};

#endif //PROJECT_1_LOGGER_HPP
