#ifndef CL_DATATYPE_DATATYPE_H
#define CL_DATATYPE_DATATYPE_H
#include <stddef.h>

typedef enum {
    CMP_WRONG,
    CMP_EQUAL,
    CMP_MORE,
    CMP_LESS
} CompareResult;

typedef struct DataEnvelope DataEnvelope;

typedef struct{
    unsigned int id;
    CompareResult (*cmp)(const DataEnvelope*, const DataEnvelope*);
    size_t (*hash)(const DataEnvelope*);
    size_t size;
} DataType;

struct DataEnvelope {
    const DataType* type;
    void* data;
    bool full_copy; // if true envelope_copy will perform deep copy of data, if false will do nothing
};

const DataType* datatype_create(unsigned int id, 
                                CompareResult (*cmp)(const DataEnvelope*, const DataEnvelope*), 
                                size_t (*hash)(const DataEnvelope*), 
                                size_t size);
void datatype_free(const DataType* dt);

DataEnvelope* envelope_create(const DataType* type, void* data);
void envelope_free(DataEnvelope* envelope);
DataEnvelope* envelope_copy(DataEnvelope* data);

#endif
