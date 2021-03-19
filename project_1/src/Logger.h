#ifndef PROJECT_1_LOGGER_HPP
#define PROJECT_1_LOGGER_HPP

#include <string>
#include <queue>

using namespace std;

class Logger final {

private:
    queue<string> file_buffer;

    static Logger* instance_;

    Logger() = default;

    static string get_prefix();
    static string get_formated_date();

    static Logger &get_instance();

public:
    // src: https://github.com/hnrck/singleton_example
    ~Logger() = default;

    Logger(const Logger &) = delete;
    void operator=(const Logger &) = delete;
    Logger(Logger &&) noexcept = default;
    Logger &operator=(Logger &&) noexcept = default;

    static void log_to_screen(string log_text);
    static void log_to_file(string text);

    static void flush_file_buffer();
    static void end_logger();

};

#endif //PROJECT_1_LOGGER_HPP
