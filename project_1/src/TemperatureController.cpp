#include <thread>
#include <tuple>
#include <chrono>
#include <ctime>
#include <future>
#include <cmath>
#include <iostream>
#include <sstream>

#include "TemperatureController.hpp"
#include "LCDManager.hpp"
#include "Logger.h"
#include "GPIOManager.h"

using namespace std;

void TemperatureController::update_lcd()
{
}

void TemperatureController::compute_pid()
{
    time_t now = time(nullptr);
    float delta_time = difftime(now, last_time);

    float error = this->reference_temperature - this->internal_temperature;
    this->error_sum += (error * delta_time);              //integral
    auto delta_error = (error - last_error) / delta_time; // derivative

    // compute pid
    this->temperature_adjustment = this->kp * error + this->ki * error_sum + this->kd * delta_error;

    // remember values for next run
    this->last_error = error;
    this->last_time = now;
}

void TemperatureController::update_data()
{


    auto f_it = async(&PotentiometerManager::get_internal_temperature, this->potentiometer);
    auto f_et = async(&PotentiometerManager::get_potentiometer_temperature, this->potentiometer);

    float rt;
    if (this->reference_source == REFERENCE_FROM_USER)
    {
        rt = user_defined_reference_temperature;
    }
    else
    {
        auto f_data = async(&BMEManager::get_data, this->bme);
        auto data = f_data.get();
        rt = get<0>(data);
    }

    this->internal_temperature = f_it.get();
    this->external_temperature = f_et.get();
    this->reference_temperature = rt;
}

int TemperatureController::clamp(int min, int max, int value){

    int final_value;

    if (value < min){
        final_value = min;
    }else if (value > max){
        final_value = max;
    }else{
        final_value = value;
    }

    return final_value;
}

tuple<int, int> TemperatureController::get_activation_values(){
    // returns (resistor %, fan %) based on pid result

    int resistor = 0, fan = 0;

    // if pid is negative it means that the system needs to heat up
    if (this->temperature_adjustment < 0){
        // turn on the resistor to heat up
        // the resistor has a range of 0% to 100%
        resistor = this->clamp(0, 100, (int) round(this->temperature_adjustment * -1));
    }else{
        // turn on the fan to cool down
        // the fan has a range of 40% to 100%
        resistor = this->clamp(40, 100, (int) round(this->temperature_adjustment * -1));
    }

    return make_tuple(resistor, fan);
}

TemperatureController::TemperatureController()
{

    this->potentiometer = new PotentiometerManager();
    this->bme = new BMEManager();

    this->reference_source = REFERENCE_FROM_SENSOR;

    // populate initial data
    this->update_data();
    this->last_time = time(nullptr);
}

void TemperatureController::execute_temperature_control(){

    Logger::log_to_screen("Starting temperature control");

    // each iteration represents a second
    // every 1 seconds:
        // - update the lcd
        // - run pid
    // every 2 seconds:
        // - save csv with: datetime, internal temperature, external temperature, user temperature, resistor %, fan %)
    
    bool save_csv = false;

    auto* gpio = new GPIOManager();

    while (this->execute)
    {
        Logger::log_to_screen("Temperature control iteration");

        update_data();
        compute_pid();

        auto activation_percentages = this->get_activation_values();
        
        gpio->set_value(GPIOManager::GPIO_RESISTOR_PIN, get<0>(activation_percentages));
        gpio->set_value(GPIOManager::GPIO_FAN_PIN, get<1>(activation_percentages));

        if (save_csv){
            save_csv = false;

            stringstream csv_log;

            csv_log << "\"";
            csv_log << this->last_time;
            csv_log << "\", ";
            csv_log << this->internal_temperature;
            csv_log << ", ";
            csv_log << this->external_temperature;
            csv_log << ", ";
            csv_log << this->reference_temperature;
            csv_log << ", ";
            csv_log << get<0>(activation_percentages);
            csv_log << ", ";
            csv_log << get<1>(activation_percentages);
            csv_log << "\n";

            Logger::log_to_file(csv_log.str());

        }else{
            save_csv = true;
        }
       
        std::this_thread::sleep_for(chrono::milliseconds(1000));
    }
    
}

void TemperatureController::start(){

    // ignore if already in execution
    if (this->execute) return;

    this->execute = true;
    this->temperature_control_thread = new thread(&TemperatureController::execute_temperature_control, this);

}

void TemperatureController::end(){

    this->execute = false;
    this->temperature_control_thread->join();

}

void TemperatureController::set_reference_temperature(float temperature)
{

    if (temperature >= 0)
    {
        this->user_defined_reference_temperature = temperature;
        this->reference_source = REFERENCE_FROM_USER;
    }
    else
    {
        this->reference_source = REFERENCE_FROM_SENSOR;
    }
}

float TemperatureController::get_internal_temperature() const {
    return internal_temperature;
}

float TemperatureController::get_external_temperature() const {
    return external_temperature;
}

float TemperatureController::get_reference_temperature() const {
    return reference_temperature;
}

float TemperatureController::get_temperature_adjustment() const {
    return temperature_adjustment;
}
