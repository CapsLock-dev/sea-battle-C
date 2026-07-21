#ifndef CLTEST_CLTEST_H
#define CLTEST_CLTEST_H

#include "cltest/cltest-comparisons.h"

typedef struct TestCase TestCase;
void clt_new_test(const char* name, int (*func)());
void clt_set_error_output(const char* str);
void clt_set_fail_line(int line);

#define EXPECT_TRUE(statement) CLTEST_EXPECT_TRUE(statement)
#define EXPECT_FALSE(statement) CLTEST_EXPECT_FALSE(statement)
#define EXPECT_EQ_NUM(x1, x2) CLTEST_EXPECT_EQ_NUM((long long)x1,(long long)x2)
#define EXPECT_EQ_STR(str1, str2) CLTEST_EXPECT_EQ_STR(str1,str2)
#define EXPECT_NEQ_NUM(x1, x2) CLTEST_EXPECT_NOT_EQ_NUM((long long)x1,(long long)x2)
#define EXPECT_NEQ_STR(str1, str2) CLTEST_EXPECT_NOT_EQ_STR(str1,str2)

#define TEAR_DOWN(...) do {tear_down: __VA_ARGS__; return 0;} while(false);

#define TEST_F(name) int name(); \
void clt_new_test_helper_##name() __attribute__ ((constructor));\
void clt_new_test_helper_##name() { \
    clt_new_test(#name, name);\
} \
int name()

#endif
