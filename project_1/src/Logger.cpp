#include <iostream>
#include <chrono>
#include <ctime>
#include <memory>
#include <string>
#include <stdexcept>

#include "Logger.h"

using namespace std;


string Logger::get_prefix()
{
    string prefix = "[ " + get_formated_date() + " ] --- ";
    return prefix;
}

string Logger::get_formated_date()
{

    auto now = chrono::system_clock::now();
    auto c_now = chrono::system_clock::to_time_t(now);

    string time_str = string(ctime(&c_now));
    time_str.replace(time_str.find('\n'), 1, "");

    return time_str;
}

Logger &Logger::get_instance() {
  static auto &&logger = Logger();
  return (logger);
}

void Logger::log_to_screen(string log_text)
{

    cout << get_prefix() + log_text + '\n';
}

void Logger::log_to_file(string text)
{
    get_instance().file_buffer.push(text);
}

void Logger::end_logger()
{

    // TODO: flush remaining file_buffer to file
   
}
