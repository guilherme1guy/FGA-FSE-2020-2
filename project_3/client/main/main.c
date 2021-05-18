#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#include "cJSON.h"

#include "wifi.h"
#include "mqtt.h"
#include "nvs.h"
#include "dht11.h"
#include "gpio.h"

xSemaphoreHandle wifiSemaphore;
xSemaphoreHandle MQTTSemaphore;

static uint8_t mac_id[6];
char mac_str[13];
char device_topic[45];

#define LED_GPIO GPIO_NUM_2
#define BUTTON_GPIO GPIO_NUM_0
#define DHT_GPIO GPIO_NUM_4

// lists used to generate io_info for server registration
const int inputs[] = {BUTTON_GPIO, DHT_GPIO};
const int outputs[] = {LED_GPIO};

bool registered = false;
char *device_location = NULL;

cJSON *create_base_json()
{

    cJSON *msg = cJSON_CreateObject();
    cJSON *sender = cJSON_CreateString(mac_str);
    cJSON_AddItemToObject(msg, "sender", sender);

    return msg;
}

char *create_registration_message()
{
    cJSON *msg = create_base_json();

    cJSON *is_dht_present = cJSON_CreateBool(true);

    // io_info
    cJSON *io_info = cJSON_CreateObject();

    cJSON *input_info = cJSON_CreateIntArray(inputs, sizeof(inputs) / sizeof(inputs[0]));
    cJSON *output_info = cJSON_CreateIntArray(outputs, sizeof(outputs) / sizeof(outputs[0]));

    cJSON_AddItemToObject(io_info, "input", input_info);
    cJSON_AddItemToObject(io_info, "output", output_info);
    // io_info end

    // add to base message
    cJSON_AddItemToObject(msg, "is_dht_present", is_dht_present);
    cJSON_AddItemToObject(msg, "io_info", io_info);

    // needs to be freed later
    char *msg_str = cJSON_Print(msg);

    // free json obj
    cJSON_Delete(msg);

    return msg_str;
}

void send_sensor_value_with_id_mqtt(char *type_string, int value, int gpio)
{
    char base_topic[] = "fse2020/160123186/\0";

    int total_chars = strlen(&base_topic[0]) +
                      strlen(type_string) + 1 +
                      strlen(device_location) + 1;

    char *topic = malloc(total_chars * sizeof(char));

    strcpy(topic, &base_topic[0]);
    strcat(topic, device_location);
    strcat(topic, "/");
    strcat(topic, type_string);

    cJSON *msg = create_base_json();
    cJSON *value_obj = cJSON_CreateNumber((double)value);

    cJSON_AddItemToObject(msg, "value", value_obj);

    if (gpio != -1)
    {
        cJSON *gpio_obj = cJSON_CreateNumber((double)gpio);

        cJSON_AddItemToObject(msg, "gpio", gpio_obj);
    }

    char *msg_str = cJSON_Print(msg);

    mqtt_send_message(topic, msg_str);

    cJSON_Delete(msg);
    free(msg_str);
    free(topic);
}

void send_sensor_value_mqtt(char *type_string, int value)
{
    send_sensor_value_with_id_mqtt(type_string, value, -1);
}

void handle_mqtt_event(char *data, int len)
{

    cJSON *msg = cJSON_ParseWithLength(data, len);
    cJSON *sender = cJSON_GetObjectItemCaseSensitive(msg, "sender");

    if (strcmp(sender->valuestring, mac_str) == 0)
    {
        // ignore self messages
        printf("Ignoring self event\n");
        cJSON_Delete(msg);
        return;
    }

    printf("Handling received event\n");

    cJSON *location = cJSON_GetObjectItem(msg, "location");
    cJSON *unregister = cJSON_GetObjectItemCaseSensitive(msg, "unregister");

    if (location != NULL)
    {
        printf("Received location from server\n");

        char *location_str = location->valuestring;

        printf(
            "Comparing local to received location: \"%s\" and \"%s\"\n",
            device_location,
            location_str);

        if (strcmp(location_str, device_location) != 0)
        {
            printf("Replacing local location with server location...\n");
            write_str_to_nvs("location", location_str);
            write_int_to_nvs("registered", 1);

            char *non_volatile_location_str = calloc(strlen(location_str), sizeof(char));
            strcpy(non_volatile_location_str, location_str);

            char *old_location = device_location;
            device_location = non_volatile_location_str;

            free(old_location);
        }

        printf("Device is registered\n");
        registered = true;
    }
    else if (unregister != NULL)
    {
        printf("Received an unregister request\n");

        // avoiding writing location to nvs, unless needed

        write_int_to_nvs("registered", 0);
        registered = false;
    }

    cJSON_Delete(msg);
}

void connectWifiTask(void *params)
{
    while (true)
    {
        if (xSemaphoreTake(wifiSemaphore, portMAX_DELAY))
        {
            mqtt_start();
        }
    }
}

void serverComunicationTask(void *params)
{
    if (xSemaphoreTake(MQTTSemaphore, portMAX_DELAY))
    {
        while (true)
        {
            mqtt_subscribe_topic(device_topic);

            if (registered)
            {
                printf("Sending sensor data to server\n");

                struct dht11_reading currentData = DHT11_read();

                send_sensor_value_mqtt("temperatura", currentData.temperature);
                send_sensor_value_mqtt("umidade", currentData.humidity);
                send_sensor_value_with_id_mqtt("estado", currentData.status, DHT_GPIO);
            }
            else
            {
                printf("Sending registration message to server\n");

                char *registration_message = create_registration_message();

                mqtt_send_message(
                    device_topic,
                    registration_message);

                free(registration_message);
            }

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
}

void app_main(void)
{

    // init nvs
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    printf("Reading initial data from NVS\n");

    // check for values on nvs
    int i_registered = read_int_from_nvs("registered");

    if (i_registered == -1)
    {
        // NVS has no values
        printf("'registered' not in NVS, setting as false\n");
        write_int_to_nvs("registered", false);
    }
    else
    {
        registered = (i_registered == 1 ? true : false);
        printf(
            "NVS data shows that device is%s registered\n",
            (registered ? "" : " NOT"));
    }

    // read_str automatically allocates device_location (if no errors happen)
    int len_device_location;
    char *read_str = read_str_from_nvs("location", &len_device_location);

    if (read_str == NULL)
    {
        printf("'location' not in NVS\n");
        printf("Creating default data for 'location' on NVS");
        device_location = malloc(2 * sizeof(char));
        device_location[0] = ' ';
        device_location[1] = '\0';

        write_str_to_nvs("location", device_location);
    }
    else
    {
        printf("'location' was read with size %d\n", len_device_location);
        device_location = read_str;
    }

    // create semaphores
    wifiSemaphore = xSemaphoreCreateBinary();
    MQTTSemaphore = xSemaphoreCreateBinary();

    // get mac address
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(&mac_id[0]));
    sprintf(mac_str, "%02x%02x%02x%02x%02x%02x", mac_id[0], mac_id[1], mac_id[2], mac_id[3], mac_id[4], mac_id[5]);
    printf("MAC: %s\n", mac_str);

    // get default device topic
    sprintf(device_topic, "fse2020/160123186/dispositivos/%s", mac_str);

    // connect to wifi
    wifi_init_sta();
    printf("Wifi started\n");

    printf(
        "Starting device tasks with status:\n\tRegistered: %d\n\tLocation: \"%s\"(len: %d)\n\tDeice Mac: %s\n",
        registered,
        device_location,
        strlen(device_location),
        mac_str);

    // create tasks
    xTaskCreate(&connectWifiTask, "MQTT Connection", 4096, NULL, 1, NULL);
    xTaskCreate(&serverComunicationTask, "Broker Communication", 4096, NULL, 1, NULL);

    // start DHT11
    DHT11_init(DHT_GPIO);

    // start gpio
    init_gpio_handler();

    // register inputs
    printf("Registering input GPIO#%d\n", BUTTON_GPIO);
    setup_input_pin(BUTTON_GPIO);
    // GPIO_4/DHT_GPIO is not registered here, since DHT will handle it

    // register outputs
    for (int i = 0; i < sizeof(outputs) / sizeof(outputs[0]); i++)
    {
        printf("Registering output GPIO#%d\n", outputs[i]);
        setup_output_pin(outputs[i]);
    }
}
