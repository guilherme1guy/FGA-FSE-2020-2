#ifndef PROJECT_2_ALARMDATAENCODER_H
#define PROJECT_2_ALARMDATAENCODER_H

#include <sstream>
#include <tuple>
#include "Constants.h"

class AlarmDataEncoder
{

public:
    static string encodeAlarm(int clientPort, int device)
    {
        stringstream stream;

        stream << to_string(clientPort) << Constants::SEPARATOR
               << to_string(device);

        return stream.str();
    }

    static tuple<int, int> decodeAlarm(string data)
    {
        stringstream clientPortStream;
        stringstream deviceStream;

        int separatorsFound = 0;
        for (auto c : data)
        {
            if (c == Constants::SEPARATOR)
            {
                separatorsFound += 1;
            }
            else if (separatorsFound == 0)
            {
                clientPortStream << c;
            }
            else if (separatorsFound == 1)
            {
                deviceStream << c;
            }
        }

        int clientPort = atoi(clientPortStream.str().c_str());
        int device = atoi(deviceStream.str().c_str());

        return make_tuple(clientPort, device);
    }
};

#endif // PROJECT_2_ALARMDATAENCODER_H
