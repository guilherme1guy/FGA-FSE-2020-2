#ifndef PROJECT_2_BMEDATAENCODER_H
#define PROJECT_2_BMEDATAENCODER_H

#include <string>
#include <sstream>
#include <tuple>
#include <cmath>
#include "Constants.h"

using namespace std;

class BMEDataEncoder
{
public:
    static string encodeTemperatureHumidity(int clientPort, float temperature, float humidity)
    {
        stringstream stream;

        int i_temperature = (int)(round(temperature * 100));
        int i_humidity = (int)(round(humidity * 100));

        stream << to_string(clientPort) << Constants::SEPARATOR
               << to_string(i_temperature) << Constants::SEPARATOR
               << to_string(i_humidity);

        return stream.str();
    }

    static tuple<int, float, float> decodeTemperatureHumidity(string data)
    {
        stringstream clientPortStream;
        stringstream temperatureStream;
        stringstream humidityStream;

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
                temperatureStream << c;
            }
            else if (separatorsFound == 2)
            {
                humidityStream << c;
            }
        }

        int clientPort = atoi(clientPortStream.str().c_str());
        float temperature = atoi(temperatureStream.str().c_str()) / 100.0;
        float humidity = atoi(humidityStream.str().c_str()) / 100.0;

        return make_tuple(clientPort, temperature, humidity);
    }
};

#endif // PROJECT_2_BMEDATAENCODER_H
