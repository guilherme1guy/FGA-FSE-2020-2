#include <stdlib.h>
#include <string.h>

unsigned char* endian_swap(unsigned char* bytes, int size){

    unsigned char* result = calloc(size, sizeof(unsigned char));
    
    for (int i = 0; i < size; i++){
        result[i] = bytes[size - i - 1];
    }

    return result;
}