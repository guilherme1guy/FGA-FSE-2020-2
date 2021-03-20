#include <iostream>
#include <chrono>
#include <ctime>
#include <memory>
#include <string>
#include <stdexcept>
#include <ncurses.h>

#include "Logger.h"

using namespace std;


Logger::Logger(){

    for(int i = 0; i < MAX_LOG_LINES; i++){
        this->log_lines.push_back("\n");
    }
}

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

    string s = get_prefix() + log_text + '\n';
    
    auto *v = &(get_instance().log_lines);
    v->push_back(s);

    while (v->size() > Logger::MAX_LOG_LINES){
        v->pop_front();
    }
}

void Logger::log_to_file(string text)
{
    get_instance().file_buffer.push(text);
}

void Logger::end_logger()
{

    // TODO: flush remaining file_buffer to file
   
}

list<string> Logger::get_log_lines(){
    return get_instance().log_lines;
}