#ifndef CL_TEST_INTERNAL_COMPARISONS_H
#define CL_TEST_INTERNAL_COMPARISONS_H

#include <stdio.h>
#include <string.h>

#define CLTEST_FAIL_TEST()           \
    do {                             \
        clt_set_fail_line(__LINE__); \
        goto tear_down;              \
    } while (false);
#define CLTEST_EXPECT_TRUE(statement)                      \
    do {                                                   \
        if ((statement) != true) {                         \
            clt_set_error_output(#statement " was false"); \
            CLTEST_FAIL_TEST();                            \
        }                                                  \
    } while (false);
#define CLTEST_EXPECT_FALSE(statement)                     \
    do {                                                   \
        if ((statement) != true) {                         \
            clt_set_error_output(#statement " was false"); \
            CLTEST_FAIL_TEST();                            \
        }                                                  \
    } while (false);
#define CLTEST_EXPECT_EQ_NUM(x1, x2)                                         \
    do {                                                                     \
        if (x1 != x2) {                                                      \
            char buffer[512];                                                \
            snprintf(buffer, sizeof buffer, "%s = %lld, %s = %lld", #x1, x1, \
                     #x2, x2);                                               \
            clt_set_error_output(buffer);                                    \
            CLTEST_FAIL_TEST();                                              \
        }                                                                    \
    } while (false);
#define CLTEST_EXPECT_EQ_STR(s1, s2)                                          \
    do {                                                                      \
        if (strcmp(s1, s2) != 0) {                                            \
            char buffer[512];                                                 \
            snprintf(buffer, sizeof buffer, "%s = %s, %s = %s", #s1, s1, #s2, \
                     s2);                                                     \
            clt_set_error_output(buffer);                                     \
            CLTEST_FAIL_TEST();                                               \
        }                                                                     \
    } while (false);

#define CLTEST_EXPECT_NOT_EQ_NUM(x1, x2)                                     \
    do {                                                                     \
        if (x1 == x2) {                                                      \
            char buffer[512];                                                \
            snprintf(buffer, sizeof buffer, "%s = %lld, %s = %lld", #x1, x1, \
                     #x2, x2);                                               \
            clt_set_error_output(buffer);                                    \
            CLTEST_FAIL_TEST();                                              \
        }                                                                    \
    } while (false);
#define CLTEST_EXPECT_NOT_EQ_STR(s1, s2)                                      \
    do {                                                                      \
        if (strcmp(s1, s2) == 0) {                                            \
            char buffer[512];                                                 \
            snprintf(buffer, sizeof buffer, "%s = %s, %s = %s", #s1, s1, #s2, \
                     s2);                                                     \
            clt_set_error_output(buffer);                                     \
            CLTEST_FAIL_TEST();                                               \
        }                                                                     \
    } while (false);

#endif
