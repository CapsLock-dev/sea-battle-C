#include "datatype/datatype.h"
#include <stdlib.h>
#include <string.h>

const DataType* datatype_create(unsigned int id, 
                                CompareResult (*cmp)(const DataEnvelope*, const DataEnvelope*), 
                                size_t (*hash)(const DataEnvelope*), 
                                size_t size) 
{
    DataType* dt = malloc(sizeof(DataType));
    if (dt == NULL) return NULL;
    dt->id = id;
    dt->cmp = *cmp;
    dt->hash = *hash;
    dt->size = size;

    return dt;
}

void datatype_free(const DataType* dt) {
    free((DataType*)dt);
}

DataEnvelope* envelope_create(const DataType* type, void* data) {
    DataEnvelope* de = malloc(sizeof(DataEnvelope));
    if (de == NULL) return NULL;
    de->data = malloc(type->size);
    if (de->data == NULL) {free(de); return NULL;}
    memcpy(de->data, data, type->size);
    de->type = type;
    de->full_copy = true;
    return de;
}

void envelope_free(DataEnvelope* envelope) {
    free(envelope->data);
    free(envelope);
}

DataEnvelope* envelope_copy(DataEnvelope* data) {
    if (!data->full_copy) return data;
    DataEnvelope* env = malloc(sizeof(DataEnvelope));
    if (env == NULL) return NULL;
    env->data = malloc(data->type->size);
    if (env->data == NULL) {free(env); return NULL;}
    env->type = data->type;
    memcpy(env->data, data->data, data->type->size);
    return env;
}
