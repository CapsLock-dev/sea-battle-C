#include "cltest/cltest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct TestCase {
    const char* name;
    int (*func)();
    TestCase* next;
};

typedef struct {
    TestCase* head;
    TestCase* tail;
} CaseList;

static CaseList* case_list = NULL;

void clt_new_test(const char* name, int (*func)()) {
    TestCase* new_case = malloc(sizeof(TestCase));
    if (new_case == NULL) return;
    new_case->func = func;
    new_case->name = name;
    new_case->next = NULL;

    if (case_list == NULL) {
        case_list = malloc(sizeof(CaseList));
        case_list->head = new_case;
        case_list->tail = new_case;
    } else {
        case_list->tail->next = new_case;
        case_list->tail = new_case;
    }
}

static char g_error_message[512] = {0};
static int g_fail_line = 0;

void clt_set_error_output(const char* str) {
    strncpy(g_error_message, str, sizeof(g_error_message) - 1);
    g_error_message[sizeof(g_error_message) - 1] = '\0';
}

void clt_set_fail_line(int line) { g_fail_line = line; }

static int run_case(TestCase* test) {
    test->func();
    if (g_fail_line != 0) {
        printf("\033[1;31mTest %s failed at line %d. %s \033[0m\n", test->name,
               g_fail_line, g_error_message);
        clt_set_fail_line(0);
        return 1;
    } else {
        printf("\033[1;32mTest %s passed\033[0m\n", test->name);
        return 0;
    }
}

int run_all_tests() {
    TestCase* curr = case_list->head;
    int fails = 0;
    while (curr != NULL) {
        fails += run_case(curr);
        curr = curr->next;
    }
    return fails;
}

void list_all_tests() {
    TestCase* curr = case_list->head;
    while (curr != NULL) {
        printf("%s\n", curr->name);
        curr = curr->next;
    }
}

int run_specifit_test(char* name) {
    TestCase* curr = case_list->head;
    while (curr != NULL) {
        if (strcmp(name, curr->name) == 0) {
            return run_case(curr);
        }
        curr = curr->next;
    }
    printf("Test not found\n");
    return 1;
}

int main(int argc, char** argv) {
    if (case_list == NULL) return 0;
    int code = 0;
    if (argc == 2 && strcmp(argv[1], "--list") == 0) {
        list_all_tests();
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--all") == 0) {
        code = run_all_tests() > 0 ? 1 : 0;
    } else if (argc == 2) {
        code = run_specifit_test(argv[1]);
    } else {
        code = run_all_tests() > 0 ? 1 : 0;
    }
    TestCase* curr = case_list->head;
    while (curr != NULL) {
        TestCase* next = curr->next;
        free(curr);
        curr = next;
    }
    return code;
}
