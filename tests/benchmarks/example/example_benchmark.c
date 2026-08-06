#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "clbench/clbench.h"

#define WIDTH 200
#define HEIGHT 60
#define FRAMES 10000

BENCH_F(ExampleBench1) {
    BenchmarkClock clock = start_clock(FRAMES);
    for (int frame = 0; frame < FRAMES; frame++) {
        printf("123\n");
    }
    stop_clock(&clock);
    return get_metrics(clock, CLB_TIMEPREFIX_MILLI);
}

BENCH_F(ExampleBench2) {
    BenchmarkClock clock = start_clock(FRAMES);
    for (int frame = 0; frame < FRAMES; frame++) {
        printf("123\n");
        printf("123\n");
    }
    stop_clock(&clock);
    return get_metrics(clock, CLB_TIMEPREFIX_MILLI);
}
