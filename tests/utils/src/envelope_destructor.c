#include "test-utils/envelope_destructor.h"

#include <stdlib.h>

typedef struct EnvelopeContainer EnvelopeContainer;

struct EnvelopeContainer {
    DataEnvelope* envelope;
    EnvelopeContainer* next;
};

typedef struct {
    EnvelopeContainer* head;
    EnvelopeContainer* tail;
} EnvelopeList;

static EnvelopeList list;

DataEnvelope* s_envelope_init(DataEnvelope* data) {
    EnvelopeContainer* cont = malloc(sizeof(EnvelopeContainer));
    if (cont == NULL) return NULL;
    cont->envelope = data;
    cont->next = NULL;
    if (list.tail != NULL) {
        list.tail->next = cont;
        list.tail = cont;
    } else {
        list.head = list.tail = cont;
    }
    return data;
}

void s_envelope_free_all() {
    EnvelopeContainer* current = list.head;
    while (current) {
        EnvelopeContainer* next = current->next;
        envelope_free(current->envelope);
        free(current);
        current = next;
    }
    list.head = NULL;
    list.tail = NULL;
}
