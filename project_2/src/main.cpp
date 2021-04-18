#include <csignal>
#include <string>
#include <mutex>

#include "program/Program.h"
#include "program/client/ClientProgram.h"
#include "program/server/ServerProgram.h"
#include "curses.h"

using namespace std;

Program *p;
mutex exit_lock;
int SIGINT_COUNT = 0;

static void quit(int sig)
{

    // this lock will never be release, because the program will end
    // after it is obtained
    bool hasLock = exit_lock.try_lock();

    if (sig == SIGINT)
        SIGINT_COUNT += 1;

    if (hasLock)
    {
        Logger::logToScreen("Finishing program...");

        if (p != nullptr)
        {
            Logger::logToScreen("Destroying program instance...");
            delete p;
            p = nullptr;
        }

        Logger::logToScreen("Gracious exit!");
        exit(sig);
    }
    else
    {
        Logger::logToScreen("An exit operation is already in progress, killing...");

        if (SIGINT_COUNT > 1)
        {
            exit(-1);
        }
    }
}

void invalid_command()
{
    cerr << "Invalid usage. Please use one of the following modes:\
        \nbin s <port>\t\t\t | executes in server mode, listening on<port>\
        \nbin c <host_ip> <host_port>\t | executes in client mode, connecting to<host_ip> : <host_port>\
        \n\t\t\t\t\t<host_ip> must be in the format : ''255.255.255.255''"
         << endl;

    quit(-1);
}

int main(int argc, const char *argv[])
{
    signal(SIGINT, quit);

    // command examples:
    // bin s 3000                  executes in server mode, listening on port 3000
    /// bin c 192.168.0.1 3000     executes in client mode, connecting to a server on 192.168.0.1:3000

    if (argc < 3)
    {
        invalid_command();
    }

    // agr[1] is mode
    string mode = string(argv[1]);

    // create correct Program object
    if (mode[0] == 's')
    {
        int port = atoi(argv[2]);
        p = new ServerProgram(port);
    }
    else if (mode[0] == 'c')
    {
        if (argc < 4)
        {
            invalid_command();
        }

        string ip = string(argv[2]);
        int port = atoi(argv[3]);

        p = new ClientProgram(ip, port);
    }
    else
    {
        invalid_command();
    }

    while (p)
    {
        this_thread::sleep_for(chrono::seconds(1));
    }

    quit(0);
}