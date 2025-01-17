#include <thread>
#include <tuple>
#include <chrono>
#include <ctime>
#include <future>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cstdio>

#include "TemperatureController.hpp"
#include "LCDManager.hpp"
#include "Logger.h"
#include "GPIOManager.h"

using namespace std;

void TemperatureController::update_lcd()
{

    char buffer[50] = {0};

    sprintf(&(buffer[0]), "TI %.2f TE %.2f", this->get_internal_temperature(), this->get_external_temperature());
    auto line1 = string(buffer);

    sprintf(&(buffer[0]), "TR %.2f", this->get_reference_temperature());
    auto line2 = string(buffer);

    this->lcd->write_on_screen(line1, line2);
}

void TemperatureController::compute_pid()
{
    auto now = chrono::high_resolution_clock::now();
    auto delta = chrono::duration_cast<chrono::duration<double>>(now - last_time);
    float delta_time = delta.count();

    float error = this->reference_temperature - this->internal_temperature;
    this->error_sum = this->clamp(this->error_sum + error * delta_time, -100, 100);  //integral
    auto delta_error = this->clamp((error - last_error) / delta_time, -100, 100);    // derivative

    // compute pid
    auto pid = this->kp * error + this->ki * error_sum + this->kd * delta_error;
    this->temperature_adjustment = this->clamp(pid, -100.0, 100.0);

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

float TemperatureController::clamp(float value, float min_v, float max_V){

    return max(min_v, min(max_V, value));
}

tuple<int, int> TemperatureController::get_activation_values(){
    // returns (resistor %, fan %) based on pid result

    int resistor = 0, fan = 0;

    // if pid is positive it means that the system needs to heat up
    if (this->temperature_adjustment > 0){
        // turn on the resistor to heat up
        // the resistor has a range of 0% to 100%
        resistor = (int) round(this->clamp(abs(this->temperature_adjustment), 0, 100));
    }else{
        // turn on the fan to cool down
        // the fan has a range of 40% to 100%
        fan =  (int) round(this->clamp(abs(this->temperature_adjustment), 40, 100));
    }

    return make_tuple(resistor, fan);
}

TemperatureController::TemperatureController()
{

    this->potentiometer = new PotentiometerManager();
    this->bme = new BMEManager();
    this->lcd = new LCDManager();

    this->reference_source = REFERENCE_FROM_SENSOR;

    // populate initial data
    this->update_data();
    this->last_time = chrono::high_resolution_clock::now();
}

TemperatureController::~TemperatureController() {
    delete this->potentiometer;
    delete this->bme;
    delete this->lcd;
}

void TemperatureController::execute_temperature_control(){

    Logger::log_to_screen("Starting temperature control");

    // each iteration represents a second
    // every 1 second:
        // - update the lcd
        // - run pid
    // every 2 seconds:
        // - save csv with: datetime, internal temperature, external temperature, user temperature, resistor %, fan %)
    
    auto* gpio = new GPIOManager();

    auto last_csv_save = chrono::high_resolution_clock::now();
    while (this->execute)
    {
        Logger::log_to_screen("Temperature control iteration");

        update_data();
        update_lcd();
        compute_pid();

        auto activation_percentages = this->get_activation_values();
        
        gpio->set_value(GPIOManager::GPIO_RESISTOR_PIN, get<0>(activation_percentages));
        gpio->set_value(GPIOManager::GPIO_FAN_PIN, get<1>(activation_percentages));

        auto now = chrono::high_resolution_clock::now();
        auto time_delta = chrono::duration_cast<chrono::duration<double>>(now - last_csv_save);
        if (time_delta.count() >= 2.0){
            // save csv every 2 seconds
            last_csv_save = now;
            stringstream csv_log;
            
            std::time_t last_time_c = std::chrono::system_clock::to_time_t(this->last_time);
            
            csv_log << "\"";
            csv_log << last_time_c;
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

        }
       
        std::this_thread::sleep_for(chrono::milliseconds(1000));
    }

    delete gpio;
    
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

    delete this->temperature_control_thread;

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


