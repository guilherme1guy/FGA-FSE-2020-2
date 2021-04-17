#ifndef PROJECT_2_CLIENTPROPERTIES_H
#define PROJECT_2_CLIENTPROPERTIES_H

#include <string>

using namespace std;

class ClientProperties
{

    int port;
    string ip;

    ClientProperties(string ip, int port)
    {
        this->ip = ip;
        this->port = port;
    }
};

#endif // PROJECT_2_CLIENTPROPERTIES_H
