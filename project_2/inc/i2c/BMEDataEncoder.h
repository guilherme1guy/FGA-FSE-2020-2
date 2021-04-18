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
    static string encodeTemperatureHumidity(float temperature, float humidity)
    {
        stringstream stream;

        int i_temperature = (int)(round(temperature * 100));
        int i_humidity = (int)(round(humidity * 100));

        stream << to_string(i_temperature) << Constants::SEPARATOR << to_string(i_humidity);

        return stream.str();
    }

    static tuple<float, float> decodeTemperatureHumidity(string data)
    {
        stringstream temperatureStream;
        stringstream humidityStream;

        bool foundSeparator = false;
        for (auto c : data)
        {
            if (c == Constants::SEPARATOR)
            {
                foundSeparator = true;
            }
            else if (foundSeparator)
            {
                humidityStream << c;
            }
            else
            {
                temperatureStream << c;
            }
        }

        float temperature = atoi(temperatureStream.str().c_str()) / 100.0;
        float humidity = atoi(humidityStream.str().c_str()) / 100.0;

        return make_tuple(temperature, humidity);
    }
};

#endif // PROJECT_2_BMEDATAENCODER_H
