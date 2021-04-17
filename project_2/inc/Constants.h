//
// Created by gguy on 16/04/2021.
//

#ifndef PROJECT_2_CONSTANTS_H
#define PROJECT_2_CONSTANTS_H

using namespace std;

class Constants
{

public:
  static const char ERROR = 'e';
  static const char ACK = 'a';
  static const char IDENTIFY = 'i';
  static const char DISCONNECT = 'd';
  static const char ALARM_ALERT = '!';

  // Values also represent Output GPIO pin
  static const int GPIO_LAMP_1 = 17;
  static const int GPIO_LAMP_2 = 18;
  static const int GPIO_LAMP_3 = 27;
  static const int GPIO_LAMP_4 = 22;
  static const int GPIO_AC_1 = 13;
  static const int GPIO_AC_2 = 19;

  // Values also represent Input GPIO pin
  static const int PRESENSE_SENSOR_1 = 25;
  static const int PRESENSE_SENSOR_2 = 26;
  static const int OPENING_SENSOR_1 = 05;
  static const int OPENING_SENSOR_2 = 06;
  static const int OPENING_SENSOR_3 = 12;
  static const int OPENING_SENSOR_4 = 16;
  static const int OPENING_SENSOR_5 = 20;
  static const int OPENING_SENSOR_6 = 21;

  static string getSensorLocation(int sensorID)
  {
    switch (sensorID)
    {
    case PRESENSE_SENSOR_1:
      return "Living Room";
    case PRESENSE_SENSOR_2:
      return "Kitchen";
    case OPENING_SENSOR_1:
      return "Kitchen Door";
    case OPENING_SENSOR_2:
      return "Kitchen Window";
    case OPENING_SENSOR_3:
      return "Living Room Door";
    case OPENING_SENSOR_4:
      return "Living Room Window";
    case OPENING_SENSOR_5:
      return "Room 01 Window";
    case OPENING_SENSOR_6:
      return "Room 02 Window";
    default:
      break;
    }
    return "Invalid Location";
  }
};

#endif // PROJECT_2_CONSTANTS_H
