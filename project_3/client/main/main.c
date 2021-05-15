#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"

#include "cJSON.h"

#include "wifi.h"
#include "mqtt.h"

#include "dht11.h"

xSemaphoreHandle wifiSemaphore;
xSemaphoreHandle MQTTSemaphore;

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
    char messageBuffer[50];

    if (xSemaphoreTake(MQTTSemaphore, portMAX_DELAY))
    {
        while (true)
        {
            struct dht11_reading currentData = DHT11_read();

            printf("Temperature is %d \n", currentData.temperature);
            printf("Humidity is %d\n", currentData.humidity);
            printf("Status code is %d\n", currentData.status);

            sprintf(messageBuffer, "temperature: %d", currentData.temperature);
            mqtt_send_message("sensors/temperature", messageBuffer);

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

    // create semaphores
    wifiSemaphore = xSemaphoreCreateBinary();
    MQTTSemaphore = xSemaphoreCreateBinary();

    // connect to wifi
    wifi_init_sta();

    // create tasks
    xTaskCreate(&connectWifiTask, "MQTT Connection", 4096, NULL, 1, NULL);
    xTaskCreate(&serverComunicationTask, "Broker Communication", 4096, NULL, 1, NULL);

    // start DHT11
    DHT11_init(GPIO_NUM_4);
}
