#ifndef GPIO_H
#define GPIO_H

void init_gpio_handler();
void setup_input_pin(int pin);
void setup_output_pin(int pin);
void write_to_output_pin(int pin, int value);

#endif
