#ifndef CL_TREE_TREE_H
#define CL_TREE_TREE_H

#include <stddef.h>

#include "datatype/datatype.h"
#include "tree/errors.h"

typedef struct Node Node;
#define TREE_MAX_ENTRY_COUNT 3
#define TREE_MAX_CHILDREN_COUNT 4

typedef struct {
    DataEnvelope* key;
    DataEnvelope* value;
} Entry;

struct Node {
    Entry entries[TREE_MAX_ENTRY_COUNT];
    Node* children[TREE_MAX_CHILDREN_COUNT];
    size_t curr_entry_count;
};

typedef struct {
    const DataType* key_type;
    const DataType* value_type;
    Node* head;
} Tree;

Tree* tree_init(const DataType* key_type, const DataType* value_type);
TreeEC tree_free(Tree* t);

TreeEC tree_insert(Tree* t, DataEnvelope* key, DataEnvelope* value);
TreeEC tree_delete(Tree* t, DataEnvelope* key);
TreeEC tree_find(Tree* t, DataEnvelope* key, DataEnvelope** out_value);

#endif
