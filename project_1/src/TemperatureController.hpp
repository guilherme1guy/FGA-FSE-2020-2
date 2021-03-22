#ifndef PROJECT_1_TEMPERATURECONTROLLER_HPP
#define PROJECT_1_TEMPERATURECONTROLLER_HPP

#include <chrono>
#include <ctime>
#include <thread>
#include <tuple>


#include "PotentiometerManager.hpp"
#include "BMEManager.hpp"
#include "LCDManager.hpp"
#include "GPIOManager.h"

using namespace std;

class TemperatureController{

private:
    
    // PID related attributes
    float internal_temperature; // input for pid
    float external_temperature;
    float reference_temperature; // target pid temperature
    
    // used for integral and derivative calculations
    float error_sum, last_error;

    // constants used for pid calculations
    float kp = 5.0, ki = 1.0, kd = 5.0;    

    // result of pid
    float temperature_adjustment;

    // last time pid was executed (used to get time delta)
    chrono::high_resolution_clock::time_point last_time;

    void compute_pid();
    void update_data();

    // end of PID part

    int reference_source; // get from user (static) or from potentiometer (every 1 second)
    float user_defined_reference_temperature;
    
    static const int REFERENCE_FROM_SENSOR = 1;
    static const int REFERENCE_FROM_USER = 2;

    PotentiometerManager* potentiometer;
    BMEManager* bme;
    LCDManager* lcd;

    void update_lcd();

    float clamp(float value, float min_v, float max_v);

    // returns (resistor %, fan %) based on pid result
    tuple<int, int> get_activation_values();

    bool execute = false;
    thread *temperature_control_thread;
    void execute_temperature_control();


public:

    float get_internal_temperature() const;
    float get_external_temperature() const;
    float get_reference_temperature() const;
    float get_temperature_adjustment() const;

    TemperatureController();
    ~TemperatureController();

    void start();
    void end();

    void set_reference_temperature(float temperature); // if set to -1, use potentiometer
};


#endif //PROJECT_1_TEMPERATURECONTROLLER_HPP
