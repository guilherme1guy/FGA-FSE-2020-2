#include <iostream>
#include <chrono>
#include <ctime>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>

#include "Logger.h"

using namespace std;


Logger::Logger(){

    for(int i = 0; i < MAX_LOG_LINES; i++){
        this->log_lines.emplace_back("\n");
    }

    Logger::file_buffer_lock = new mutex();
    Logger::log_lines_lock = new mutex();

    this->writer_thread = new thread(&Logger::writer_execute, this);
}

Logger::~Logger() {

    this->execute = false;
    this->writer_thread->join();

    delete Logger::file_buffer_lock;
    delete Logger::log_lines_lock;

}

void Logger::writer_execute() const {

    while(execute){
        flush_file_buffer();

        this_thread::sleep_for(chrono::seconds(1));
    }
}


string Logger::get_prefix()
{
    stringstream s_prefix;

    s_prefix << "[";
    s_prefix << get_formated_date();
    s_prefix << "] ";

    return s_prefix.str();
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

void Logger::log_to_screen(const string& log_text)
{
    stringstream s;
    s << get_prefix();
    s << log_text << '\n';

    get_instance().log_lines_lock->lock();

    // insert new log at the end
    auto *v = &(get_instance().log_lines);
    v->push_back(s.str());

    // remove old log from the beginning to keep size
    while (v->size() > Logger::MAX_LOG_LINES){
        v->pop_front();
    }

    get_instance().log_lines_lock->unlock();
}

void Logger::log_to_file(const string& text)
{
    get_instance().file_buffer_lock->lock();
    get_instance().file_buffer.push(text);
    get_instance().file_buffer_lock->unlock();
}


void Logger::flush_file_buffer() {

    ofstream file;
    file.open("log.csv", ios::app);

    get_instance().file_buffer_lock->lock();

    while (!get_instance().file_buffer.empty()){
        string line = get_instance().file_buffer.front();
        get_instance().file_buffer.pop();

        file << line;
    }

    get_instance().file_buffer_lock->unlock();

}

void Logger::end_logger()
{
    flush_file_buffer();
}

vector<string> Logger::get_log_lines(){

    // a lock and a copy is needed, since a thread can update
    // the logs on screen while the list is being written on
    // the screen
    get_instance().log_lines_lock->lock();

    // copy lines
    vector<string> logs;
    logs.reserve(MAX_LOG_LINES + 1);

    for (const auto& log : get_instance().log_lines){
        logs.push_back(log);
    }

    get_instance().log_lines_lock->unlock();

    // return copied lines
    return logs;
}


