#ifndef CLBENCH_CLBENCH_H
#define CLBENCH_CLBENCH_H
#include <time.h>

typedef enum {
    CLB_TIMEPREFIX_NANO,
    CLB_TIMEPREFIX_MICRO,
    CLB_TIMEPREFIX_MILLI
} TimePrefix;

typedef struct {
    unsigned int iterations;
    long double avg_time;
    long double total_time;
    TimePrefix prefix;
} Metrics;

typedef struct {
    long ticks;
    long double total_time;
    unsigned int iterations;
    clock_t start;
} BenchmarkClock;

BenchmarkClock start_clock(unsigned int iterations);
void stop_clock(BenchmarkClock* clock);
Metrics get_metrics(BenchmarkClock clock, TimePrefix prefix);
void clb_new_bench(const char* name, Metrics (*func)());

#define BENCH_F(name)                                                  \
    Metrics name();                                                    \
    void clb_new_bench_helper_##name() __attribute__((constructor));   \
    void clb_new_bench_helper_##name() { clb_new_bench(#name, name); } \
    Metrics name()

#endif
