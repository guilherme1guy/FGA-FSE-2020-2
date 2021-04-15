#ifndef PROJECT_2_LOGGER_HPP
#define PROJECT_2_LOGGER_HPP

#include <string>
#include <queue>
#include <list>
#include <thread>
#include <mutex>
#include <sstream>
#include <fstream>

using namespace std;

class Logger final {

private:
    queue<string> file_buffer;
    list<string> log_lines;

    static const int MAX_LOG_LINES = 25;

    static Logger* instance_;

    Logger(){

        for(int i = 0; i < MAX_LOG_LINES; i++){
            this->log_lines.emplace_back("\n");
        }

        Logger::file_buffer_lock = new mutex();
        Logger::log_lines_lock = new mutex();

        this->writer_thread = new thread(&Logger::writer_execute, this);
    }

    thread *writer_thread;
    void writer_execute() const {

        while(execute){
            flush_file_buffer();

            this_thread::sleep_for(chrono::seconds(1));
        }
    }

    bool execute = true;


    static string get_prefix()
    {
        stringstream s_prefix;

        s_prefix << "[";
        s_prefix << get_formated_date();
        s_prefix << "] ";

        return s_prefix.str();
    }

    static string get_formated_date()
    {

        auto now = chrono::system_clock::now();
        auto c_now = chrono::system_clock::to_time_t(now);

        string time_str = string(ctime(&c_now));
        time_str.replace(time_str.find('\n'), 1, "");

        return time_str;
    }


    static Logger &get_instance() {
        static auto &&logger = Logger();
        return (logger);
    }


protected:
    mutex* file_buffer_lock;
    mutex* log_lines_lock;

public:
    // src: https://github.com/hnrck/singleton_example
    ~Logger() {

        this->execute = false;
        this->writer_thread->join();

        delete Logger::file_buffer_lock;
        delete Logger::log_lines_lock;

    }


    Logger(const Logger &) = delete;
    void operator=(const Logger &) = delete;
    Logger(Logger &&) noexcept = default;
    Logger &operator=(Logger &&) noexcept = default;

    static void log_to_screen(const string& log_text)
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

    static void log_to_file(const string& text)
    {
        get_instance().file_buffer_lock->lock();
        get_instance().file_buffer.push(text);
        get_instance().file_buffer_lock->unlock();
    }

    static void flush_file_buffer() {

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

    static void end_logger()
    {
        flush_file_buffer();
    }


    static vector<string> get_log_lines(){

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


};

#endif //PROJECT_2_LOGGER_HPP
