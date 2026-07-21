#include "tree/tree.h"
#include <stdlib.h>
#include <string.h>

Tree* tree_init(const DataType* key_type, const DataType* value_type) {
    Tree* tree = malloc(sizeof(Tree));
    if (tree == NULL) return NULL;
    tree->key_type = key_type;
    tree->value_type = value_type;
    tree->head = NULL;

    return tree;
}

static Node* node_init() {
    Node* n = calloc(1, sizeof(Node));
    if (n == NULL) return NULL;
    return n;
}

static TreeEC node_add_entry(Tree* t, Node* n, DataEnvelope* key, DataEnvelope* value) {
    if (n->curr_entry_count >= TREE_ENTRY_COUNT) return TREE_EC_EntryOverflow;
    size_t count = n->curr_entry_count;
    size_t i = 0;
    for (;i<count; ++i) {
        Entry cur = n->entries[i];
        CompareResult cmp = t->key_type->cmp(key, cur.key);
        if (cmp == CMP_LESS) break;
        if (cmp == CMP_EQUAL) return TREE_EC_KeyExists;
        if (cmp == CMP_WRONG) return TREE_EC_CompareFail;
    }
    DataEnvelope* key_copy = envelope_copy(key);
    if (key_copy == NULL) return TREE_EC_AllocationError;
    DataEnvelope* value_copy = envelope_copy(value);
    if (value_copy == NULL) {envelope_free(key_copy) ;return TREE_EC_AllocationError;}
    if (i < count) {
        memmove(&n->entries[i+1], &n->entries[i], (count-i)*sizeof(Entry));
    }
    n->entries[i].key = key_copy;
    n->entries[i].value = value_copy;
    ++n->curr_entry_count;
    return TREE_EC_Ok;
}

TreeEC tree_free(Tree* t) {
    if (t == NULL) return TREE_EC_IsNull; 

    return TREE_EC_Ok;
}

static TreeEC split(Tree* t, Node* parent, Node* n, size_t node_index) {
    Entry mid = n->entries[1];
    Node* right = node_init();
    if (right == NULL) return TREE_EC_AllocationError;
    Node* left = node_init();
    if (left == NULL) {free(right); return TREE_EC_AllocationError;}

    right->entries[0] = n->entries[2];
    n->entries[2] = (Entry){.value=NULL, .key=NULL};
    right->curr_entry_count = 1;

    left->entries[0] = n->entries[0];
    n->entries[0] = (Entry){.value=NULL, .key=NULL};
    left->curr_entry_count = 1;

    if (n->children[0] != NULL) {
        right->children[0] = n->children[2];        
        right->children[1] = n->children[3];        
        n->children[2] = NULL;
        n->children[3] = NULL;
        left->children[0] = n->children[0];
        left->children[1] = n->children[1];
        n->children[0] = NULL;
        n->children[1] = NULL;
    }
    TreeEC ec = node_add_entry(t, parent, mid.key, mid.value);
    if (ec != TREE_EC_Ok) {free(left); free(right); return ec;}
    envelope_free(mid.key);
    envelope_free(mid.value);
    free(n);

    return TREE_EC_Ok;
}

TreeEC tree_insert(Tree* t, DataEnvelope* key, DataEnvelope* value) {
    if (t == NULL || key == NULL || value == NULL) return TREE_EC_IsNull; 
    if (t->key_type != key->type || t->value_type != value->type) return TREE_EC_IncorrectType;
    if (t->head == NULL) {
        Node* n = node_init();
        if (n == NULL) return TREE_EC_AllocationError;
        if (node_add_entry(t, n, key, value) != TREE_EC_Ok) {free(n); return TREE_EC_AllocationError;}
        t->head = n;
        return TREE_EC_Ok;
    }
    Node* curr = t->head;
    while(curr->children[0] != NULL) {
        size_t child_index = curr->curr_entry_count;
        for (size_t i=0; i<curr->curr_entry_count; ++i) {
            CompareResult cmp = t->key_type->cmp(key, curr->entries[i].key);
            if (cmp == CMP_EQUAL) return TREE_EC_KeyExists;
            if (cmp == CMP_WRONG) return TREE_EC_CompareFail;
            if (cmp == CMP_LESS) {
                child_index = i;
                break;
            }
        } 
        if (curr->children[child_index]->curr_entry_count == 3) {
            //split
        }
        curr = curr->children[child_index];
    }
    TreeEC ec = node_add_entry(t, curr, key, value);
    if (ec != TREE_EC_Ok) return ec;

    return TREE_EC_Ok;
}

TreeEC tree_delete(Tree* t, DataEnvelope* key) {
    if (t == NULL || key == NULL) return TREE_EC_IsNull; 

}

TreeEC tree_find(Tree* t, DataEnvelope* key, DataEnvelope** out_value) {
    if (t == NULL || key == NULL) return TREE_EC_IsNull; 

}
