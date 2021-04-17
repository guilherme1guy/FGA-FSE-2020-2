#include <csignal>
#include <string>

#include "Program.h"
#include "ServerProgram.h"
#include "ClientProgram.h"

using namespace std;

Program *p;

static void quit(int sig)
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

    p->loop();

    quit(0);

    return 0;
}