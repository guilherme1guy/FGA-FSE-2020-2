#include <csignal>

#include "Program.hpp"

using namespace std;

Program *p;

static void quit(int sig){

    printf("Finishing program...\n");

    if (p != nullptr){
        p->quit();

        delete p;
    }

    exit(sig);
}

int main(int argc, const char *argv[])
{

    signal(SIGINT, quit);
      
    p = new Program();
    p->run();

    quit(0);
   
    return 0;
}