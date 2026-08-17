#include "tree/tree.h"

#include <assert.h>
#include <stdio.h>
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

TreeEC tree_free(Tree* t) {
    if (t == NULL) return TREE_EC_IsNull;
    if (t->head == NULL) {
        free(t);
        return TREE_EC_Ok;
    }
    size_t stack_size = 25;
    Node** stack = malloc(sizeof(Node*) * stack_size);
    if (stack == NULL) return TREE_EC_AllocationError;
    size_t top = 1;
    stack[0] = t->head;
    while (top != 0) {
        Node* curr = stack[--top];
        for (size_t i = 0; i < curr->curr_entry_count; ++i) {
            envelope_free(curr->entries[i].key);
            envelope_free(curr->entries[i].value);
        }
        for (size_t i = 0; i < TREE_MAX_CHILDREN_COUNT; ++i) {
            if (curr->children[i] != NULL) {
                if (top + 1 >= stack_size) {
                    stack_size *= 2;
                    stack = realloc(stack, sizeof(Node*) * stack_size);
                }
                stack[top++] = curr->children[i];
            }
        }
        free(curr);
    }
    free(stack);
    free(t);
    return TREE_EC_Ok;
}

static Node* node_init() {
    Node* n = calloc(1, sizeof(Node));
    if (n == NULL) return NULL;
    return n;
}

static void node_free(Node* n) {
    for (size_t i = 0; i < n->curr_entry_count; ++i) {
        envelope_free(n->entries[i].key);
        envelope_free(n->entries[i].value);
    }
    free(n);
}

static TreeEC node_add_entry(Tree* t, Node* n, DataEnvelope* key,
                             DataEnvelope* value) {
    if (n->curr_entry_count >= TREE_MAX_ENTRY_COUNT)
        return TREE_EC_EntryOverflow;
    size_t count = n->curr_entry_count;
    size_t i = 0;
    for (; i < count; ++i) {
        Entry cur = n->entries[i];
        CompareResult cmp = t->key_type->cmp(key, cur.key);
        if (cmp == CMP_LESS) break;
        if (cmp == CMP_EQUAL) return TREE_EC_KeyExists;
        if (cmp == CMP_WRONG) return TREE_EC_CompareFail;
    }
    DataEnvelope* key_copy = envelope_copy(key);
    if (key_copy == NULL) return TREE_EC_AllocationError;
    DataEnvelope* value_copy = envelope_copy(value);
    if (value_copy == NULL) {
        envelope_free(key_copy);
        return TREE_EC_AllocationError;
    }
    if (i < count) {
        memmove(&n->entries[i + 1], &n->entries[i],
                (count - i) * sizeof(Entry));
    }
    n->entries[i].key = key_copy;
    n->entries[i].value = value_copy;
    ++n->curr_entry_count;
    return TREE_EC_Ok;
}

TreeEC tree_insert(Tree* t, DataEnvelope* key, DataEnvelope* value) {
    if (t == NULL || key == NULL || value == NULL) return TREE_EC_IsNull;
    if (t->key_type != key->type || t->value_type != value->type)
        return TREE_EC_IncorrectType;
    if (t->head == NULL) {
        Node* n = node_init();
        if (n == NULL) return TREE_EC_AllocationError;
        if (node_add_entry(t, n, key, value) != TREE_EC_Ok) {
            node_free(n);
            return TREE_EC_AllocationError;
        }
        t->head = n;
        return TREE_EC_Ok;
    }
    Node* parent = NULL;
    Node* curr = t->head;
    if (curr->curr_entry_count == TREE_MAX_ENTRY_COUNT) {
        Node* right = node_init();
        if (right == NULL) return TREE_EC_AllocationError;
        Node* new_root = node_init();
        if (new_root == NULL) {
            node_free(right);
            return TREE_EC_AllocationError;
        }

        new_root->entries[0] = curr->entries[1];
        curr->entries[1] = (Entry){.value = NULL, .key = NULL};
        new_root->children[0] = curr;
        new_root->children[1] = right;
        new_root->curr_entry_count = 1;
        t->head = new_root;

        right->entries[0] = curr->entries[2];
        curr->entries[2] = (Entry){.value = NULL, .key = NULL};

        right->children[0] = curr->children[2];
        right->children[1] = curr->children[3];
        curr->children[2] = NULL;
        curr->children[3] = NULL;

        right->curr_entry_count = 1;
        curr->curr_entry_count = 1;

        parent = t->head;
        CompareResult cmp_res = t->key_type->cmp(key, new_root->entries[0].key);
        switch (cmp_res) {
            case CMP_EQUAL:
                return TREE_EC_KeyExists;
            case CMP_WRONG:
                return TREE_EC_CompareFail;
            case CMP_MORE:
                curr = new_root->children[1];
                break;
            case CMP_LESS:
                break;
        }
    }
    while (curr != NULL) {
        if (curr->curr_entry_count == TREE_MAX_ENTRY_COUNT) {
            Entry mid = curr->entries[1];
            curr->entries[1] = (Entry){.key = NULL, .value = NULL};

            Node* right = node_init();
            if (right == NULL) return TREE_EC_AllocationError;

            right->entries[0] = curr->entries[2];
            curr->entries[2] = (Entry){.key = NULL, .value = NULL};
            right->curr_entry_count = 1;
            right->children[0] = curr->children[2];
            right->children[1] = curr->children[3];
            curr->children[2] = NULL;
            curr->children[3] = NULL;
            curr->curr_entry_count = 1;

            TreeEC ec = node_add_entry(t, parent, mid.key, mid.value);
            if (ec != TREE_EC_Ok) {
                node_free(right);
                return ec;
            }
            size_t index = 0;
            for (size_t i = 0; i < parent->curr_entry_count; ++i) {
                if (t->key_type->cmp(parent->entries[i].key, mid.key) ==
                    CMP_EQUAL)
                    index = i;
            }
            int entry_count = (int)parent->curr_entry_count - 1;
            for (int i = entry_count; i > (int)index; --i) {
                parent->children[i + 1] = parent->children[i];
            }
            parent->children[index] = curr;
            parent->children[index + 1] = right;
            CompareResult cmp_res = t->key_type->cmp(key, mid.key);
            if (cmp_res == CMP_LESS) {
                curr = parent->children[index];
            } else {
                curr = parent->children[index + 1];
            }
        }
        size_t i = 0;
        if (curr->children[0] == NULL) {
            return node_add_entry(t, curr, key, value);
        }
        CompareResult cmp_res = CMP_WRONG;
        for (; i < curr->curr_entry_count; ++i) {
            Entry e = curr->entries[i];
            cmp_res = t->key_type->cmp(key, e.key);
            switch (cmp_res) {
                case CMP_EQUAL:
                    return TREE_EC_KeyExists;
                case CMP_WRONG:
                    return TREE_EC_CompareFail;
                case CMP_LESS:
                    goto exit_loop;
                case CMP_MORE:
                    break;
            }
        }
    exit_loop:
        parent = curr;
        curr = curr->children[i];
    }

    return TREE_EC_UnexpectedError;
}

static void node_remove_entry(Node* n, size_t index) {
    if (n == NULL) return;
    for (size_t i = index; i + 1 < n->curr_entry_count; ++i) {
        n->entries[i] = n->entries[i + 1];
    }
    n->entries[n->curr_entry_count - 1] = (Entry){.key = NULL, .value = NULL};
    n->curr_entry_count -= 1;
}

static void merge_children(Node* parent, size_t index) {
    if (parent == NULL) return;
    Node* main = parent->children[index];
    Node* target = parent->children[index + 1];

    main->entries[main->curr_entry_count] = parent->entries[index];
    for (size_t i = index; i + 1 < parent->curr_entry_count; ++i) {
        parent->entries[i] = parent->entries[i + 1];
    }
    parent->entries[parent->curr_entry_count - 1] =
        (Entry){.key = NULL, .value = NULL};
    for (size_t i = 0; i < target->curr_entry_count; ++i) {
        main->entries[main->curr_entry_count + 1 + i] = target->entries[i];
    }
    if (target->children[0] != NULL) {
        for (size_t i = 0; i <= target->curr_entry_count; ++i) {
            main->children[main->curr_entry_count + 1 + i] =
                target->children[i];
        }
    }
    main->curr_entry_count += 1 + target->curr_entry_count;
    free(target);
    parent->entries[parent->curr_entry_count - 1] =
        (Entry){.key = NULL, .value = NULL};
    for (size_t i = index + 1; i < parent->curr_entry_count; ++i) {
        parent->children[i] = parent->children[i + 1];
    }
    parent->children[parent->curr_entry_count] = NULL;
    parent->curr_entry_count -= 1;
}

static void fix_weak(Node* parent, size_t index) {
    if (parent == NULL || parent->children[index] == NULL ||
        parent->children[index]->curr_entry_count >= 2)
        return;
    Node* child = parent->children[index];
    if (index > 0 && parent->children[index - 1]->curr_entry_count >= 2) {
        Node* left = parent->children[index - 1];
        for (size_t i = child->curr_entry_count; i > 0; --i) {
            child->entries[i] = child->entries[i - 1];
        }
        child->entries[0] = parent->entries[index - 1];
        if (child->children[0] != NULL) {
            for (size_t i = child->curr_entry_count + 1; i > 0; --i) {
                child->children[i] = child->children[i - 1];
            }
            child->children[0] = left->children[left->curr_entry_count];
            left->children[left->curr_entry_count] = NULL;
        }
        parent->entries[index - 1] = left->entries[left->curr_entry_count - 1];
        left->entries[left->curr_entry_count - 1] =
            (Entry){.key = NULL, .value = NULL};
        ++child->curr_entry_count;
        --left->curr_entry_count;
    } else if (index < parent->curr_entry_count &&
               parent->children[index + 1]->curr_entry_count >= 2) {
        Node* right = parent->children[index + 1];
        child->entries[child->curr_entry_count] = parent->entries[index];
        if (child->children[0] != NULL) {
            child->children[child->curr_entry_count + 1] = right->children[0];
        }
        ++child->curr_entry_count;

        parent->entries[index] = right->entries[0];
        for (size_t i = 0; i + 1 < right->curr_entry_count; ++i) {
            right->entries[i] = right->entries[i + 1];
        }
        right->entries[right->curr_entry_count - 1] =
            (Entry){.key = NULL, .value = NULL};
        if (child->children[0] != NULL) {
            for (size_t i = 0; i < right->curr_entry_count; ++i) {
                right->children[i] = right->children[i + 1];
            }
            right->children[right->curr_entry_count] = NULL;
        }
        --right->curr_entry_count;
    } else if (index > 0) {
        merge_children(parent, index - 1);
    } else {
        merge_children(parent, index);
    }
}

static Entry get_predecessor(Node* n) {
    while (n->children[0] != NULL) {
        size_t index = n->curr_entry_count;
        if (n->children[index]->curr_entry_count < 2) {
            fix_weak(n, index);
            index = n->curr_entry_count;
        }
        n = n->children[index];
    }
    Entry e = n->entries[n->curr_entry_count - 1];
    n->entries[n->curr_entry_count - 1] = (Entry){.key = NULL, .value = NULL};
    n->curr_entry_count -= 1;
    return e;
}

static Entry get_successor(Node* n) {
    while (n->children[0] != NULL) {
        size_t index = 0;
        if (n->children[index]->curr_entry_count < 2) {
            fix_weak(n, index);
        }
        n = n->children[index];
    }
    Entry e = n->entries[0];
    node_remove_entry(n, 0);
    return e;
}

TreeEC tree_delete(Tree* t, DataEnvelope* key) {
    if (t == NULL || key == NULL) return TREE_EC_IsNull;
    if (t->head == NULL) return TREE_EC_KeyDoesntExists;
    Node* curr = t->head;
    while (curr != NULL) {
    continue_loop:
        size_t i = 0;
        CompareResult cmp_res = CMP_WRONG;
        for (; i < curr->curr_entry_count; ++i) {
            Entry e = curr->entries[i];
            cmp_res = t->key_type->cmp(key, e.key);
            switch (cmp_res) {
                case CMP_EQUAL:
                    if (curr->children[0] == NULL) {
                        envelope_free(curr->entries[i].key);
                        envelope_free(curr->entries[i].value);
                        node_remove_entry(curr, i);
                        if (curr == t->head && curr->curr_entry_count == 0) {
                            free(curr);
                            t->head = NULL;
                        }
                        return TREE_EC_Ok;
                    } else {
                        Node* left = curr->children[i];
                        Node* right = curr->children[i + 1];
                        if (left->curr_entry_count >= 2) {
                            Entry pred = get_predecessor(left);
                            envelope_free(curr->entries[i].key);
                            envelope_free(curr->entries[i].value);
                            curr->entries[i].key = pred.key;
                            curr->entries[i].value = pred.value;
                            return TREE_EC_Ok;
                        }
                        if (right->curr_entry_count >= 2) {
                            Entry succ = get_successor(right);
                            envelope_free(curr->entries[i].key);
                            envelope_free(curr->entries[i].value);
                            curr->entries[i].key = succ.key;
                            curr->entries[i].value = succ.value;
                            return TREE_EC_Ok;
                        }
                        merge_children(curr, i);
                        if (curr == t->head && curr->curr_entry_count == 0) {
                            Node* child = curr->children[0];
                            free(curr);
                            t->head = child;
                            curr = child;
                        } else {
                            curr = curr->children[i];
                        }
                    }
                    goto continue_loop;
                case CMP_WRONG:
                    return TREE_EC_CompareFail;
                case CMP_LESS:
                    goto exit_loop;
                case CMP_MORE:
                    break;
            }
        }
    exit_loop:
        if (curr->children[0] == NULL) return TREE_EC_KeyDoesntExists;
        if (curr->children[i]->curr_entry_count < 2) {
            fix_weak(curr, i);
            if (curr == t->head && curr->curr_entry_count == 0) {
                Node* child = curr->children[0];
                free(curr);
                t->head = child;
                curr = child;
            }
            continue;
        }

        curr = curr->children[i];
    }
    return TREE_EC_KeyDoesntExists;
}

TreeEC tree_find(Tree* t, DataEnvelope* key, DataEnvelope** out_value) {
    if (t == NULL || key == NULL) return TREE_EC_IsNull;

    Node* curr = t->head;
    while (curr != NULL) {
        size_t i = 0;
        CompareResult cmp_res = CMP_WRONG;
        for (; i < curr->curr_entry_count; ++i) {
            Entry e = curr->entries[i];
            cmp_res = t->key_type->cmp(key, e.key);
            switch (cmp_res) {
                case CMP_EQUAL:
                    *out_value = e.value;
                    return TREE_EC_Ok;
                case CMP_WRONG:
                    return TREE_EC_CompareFail;
                case CMP_LESS:
                    goto exit_loop;
                case CMP_MORE:
                    break;
            }
        }
    exit_loop:
        curr = curr->children[i];
    }

    return TREE_EC_KeyDoesntExists;
}
