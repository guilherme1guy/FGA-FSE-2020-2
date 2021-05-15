#ifndef NVS_H
#define NVS_H

int32_t read_int_from_nvs(char *key);
void write_int_to_nvs(char *key, int32_t value);

char *read_str_from_nvs(char *key, int *len);
void write_str_to_nvs(char *key, char *value);

#endif
