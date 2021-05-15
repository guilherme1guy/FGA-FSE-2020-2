#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

char DATA_NAMESPACE[] = "data\0";

nvs_handle *get_partition_handle(nvs_open_mode_t mode)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    nvs_handle *partition_handle = calloc(1, sizeof(nvs_handle));

    esp_err_t res_nvs = nvs_open(&DATA_NAMESPACE[0], mode, partition_handle);

    if (res_nvs == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGE("NVS", "Mode: %d, Namespace: %s, not found", mode, &DATA_NAMESPACE[0]);
    }

    return partition_handle;
}

void release_handle(nvs_handle *h)
{
    nvs_close(*h);
    free(h);
}

int32_t read_int_from_nvs(char *key)
{

    printf("Reading key, type {\"%s\" : INT} from NVS\n", key);

    nvs_handle *handle = get_partition_handle(NVS_READONLY);

    int32_t value = -1;

    if (handle == NULL)
    {
        return value;
    }

    esp_err_t res = nvs_get_i32(*handle, key, &value);

    switch (res)
    {
    case ESP_OK:
        printf("Read value: %d\n", value);
        break;
    case ESP_ERR_NOT_FOUND:
        ESP_LOGE("NVS_R_INT", "Value not found");
        break;
    default:
        ESP_LOGE("NVS_R_INT", "Error accessing NVS: (%s)", esp_err_to_name(res));
        break;
    }

    release_handle(handle);

    return value;
}

char *read_str_from_nvs(char *key, int *len)
{
    printf("Reading key, type {\"%s\" : STR} from NVS\n", key);

    nvs_handle *handle = get_partition_handle(NVS_READONLY);

    if (handle == NULL)
    {
        return NULL;
    }

    size_t required_len = 0;

    esp_err_t res = nvs_get_str(*handle, key, NULL, &required_len);
    switch (res)
    {
    case ESP_OK:
        break;
    case ESP_ERR_NOT_FOUND:
        ESP_LOGE("NVS_R_STR", "Value not found");
        release_handle(handle);
        return NULL;
    default:
        ESP_LOGE("NVS_R_STR", "Error accessing NVS: (%s)", esp_err_to_name(res));
        release_handle(handle);
        return NULL;
    }

    char *value = malloc(required_len);
    memset(value, '\0', required_len);

    res = nvs_get_str(*handle, key, value, &required_len);
    switch (res)
    {
    case ESP_OK:
        printf("Read value: \"%s\"  (len: %d)\n", value, required_len);
        break;
    default:
        ESP_LOGE("NVS_R_STR", "Error accessing NVS: (%s)", esp_err_to_name(res));
        free(value);
        value = NULL;
        break;
    }

    release_handle(handle);

    if (len != NULL)
    {

        *len = required_len;
    }
    return value;
}

void write_int_to_nvs(char *key, int32_t value)
{
    printf("Writing {\"%s\" : \"%d\"} to NVS\n", key, value);
    nvs_handle *handle = get_partition_handle(NVS_READWRITE);

    if (handle == NULL)
    {
        return;
    }

    esp_err_t res = nvs_set_i32(*handle, key, value);
    if (res != ESP_OK)
    {
        ESP_LOGE("NVS_W", "Error writing int to NVS: (%s)", esp_err_to_name(res));
    }
    else
    {
        nvs_commit(*handle);
        printf("Writing done\n");
    }

    release_handle(handle);
}

void write_str_to_nvs(char *key, char *value)
{

    printf("Writing {\"%s\" : \"%s\"} to NVS\n", key, value);

    nvs_handle *handle = get_partition_handle(NVS_READWRITE);

    if (handle == NULL)
    {
        return;
    }

    esp_err_t res = nvs_set_str(*handle, key, value);
    if (res != ESP_OK)
    {
        ESP_LOGE("NVS_W", "Error writing str to NVS: (%s)", esp_err_to_name(res));
    }
    else
    {
        nvs_commit(*handle);
        printf("Writing done\n");
    }

    release_handle(handle);
}