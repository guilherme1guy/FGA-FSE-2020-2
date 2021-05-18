#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"

xQueueHandle interruption_queue;

extern void send_sensor_value_with_id_mqtt(char *type_string, int value, int gpio);

static void IRAM_ATTR gpio_isr_handler(void *args)
{
    int pin = (int)args;
    xQueueSendFromISR(interruption_queue, &pin, NULL);
}

void handle_gpio_input_interrupt(void *params)
{
    int pin;

    while (true)
    {
        if (xQueueReceive(interruption_queue, &pin, portMAX_DELAY))
        {
            // De-bouncing
            int state = gpio_get_level(pin);

            gpio_isr_handler_remove(pin);

            printf("GPIO#%d state: %d\n", pin, state);
            send_sensor_value_with_id_mqtt("estado", state, pin);

            // Enable interruption again
            vTaskDelay(50 / portTICK_PERIOD_MS);
            gpio_isr_handler_add(pin, gpio_isr_handler, (void *)pin);
        }
    }
}

void setup_input_pin(int pin)
{
    // setup pin as input
    gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    // setup Pulldown resistor for pin
    gpio_pulldown_en(pin);
    // disable pull-up resistor
    gpio_pullup_dis(pin);

    // setup interruption for pin
    gpio_set_intr_type(pin, GPIO_INTR_POSEDGE);
    gpio_isr_handler_add(pin, gpio_isr_handler, (void *)pin);
}

void setup_output_pin(int pin)
{
    gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

void write_to_output_pin(int pin, int value)
{
    gpio_set_level(pin, value);
}

void init_gpio_handler()
{
    interruption_queue = xQueueCreate(10, sizeof(int));
    xTaskCreate(handle_gpio_input_interrupt, "HandleGPIOInput", 2048, NULL, 1, NULL);
    gpio_install_isr_service(0);
}
