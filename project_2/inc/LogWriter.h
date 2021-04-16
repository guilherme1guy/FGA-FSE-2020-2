#ifndef PROJECT_2_LOGWRITER_H
#define PROJECT_2_LOGWRITER_H

#include <string>
#include <queue>
#include <list>
#include <thread>
#include <mutex>
#include <fstream>
#include <chrono>

using namespace  std;

class LogWriter {
    // Write specific logs to a file
    // Has an internal thread so writing is made automatically at defined time steps
    // Deals with thread allocation and deallocation (as well as cleaning its file buffer
    // before deletion)

private:

    queue<string> fileBuffer;
    mutex* fileBufferLock;


    thread *writerThread;

    // control variable for thread execution, when false it will break
    // the thread loop
    bool execute = true;


    void threadLoop(LogWriter* t) const {
        while(execute){
            t->write();
            this_thread::sleep_for(chrono::seconds(1));
        }
    }

    void write() {

        ofstream file;
        this->fileBufferLock->lock();

        file.open("log.csv", ios::app);

        while (!fileBuffer.empty()){
            string line = fileBuffer.front();
            fileBuffer.pop();

            file << line;
        }

        file.close();

        this->fileBufferLock->unlock();
    }

public:

    LogWriter(){
        fileBufferLock = new mutex();
        writerThread = new thread(&LogWriter::threadLoop, this, this);
    }

    ~LogWriter(){

        execute = false;
        writerThread->join();

        write();

    }

    void log(const string& text)
    {
        fileBufferLock->lock();
        fileBuffer.push(text);
        fileBufferLock->unlock();
    }

};

#endif //PROJECT_2_LOGWRITER_H
