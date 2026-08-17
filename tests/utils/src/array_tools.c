#include "test-utils/array_tools.h"

#include <stdlib.h>

void array_shuffle(void* array, size_t size, size_t element_size) {
    unsigned char* bytes = array;
    for (size_t i = 0; i < size; ++i) {
        size_t j = (size_t)rand() % (i + 1);
        for (size_t k = 0; k < element_size; ++k) {
            unsigned char byte_buffer = bytes[i * element_size + k];
            bytes[i * element_size + k] = bytes[j * element_size + k];
            bytes[j * element_size + k] = byte_buffer;
        }
    }
}
