#include "clbench/clbench.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct BenchCase BenchCase;

struct BenchCase {
    const char* name;
    Metrics (*func)();
    BenchCase* next;
    Metrics result;
};

typedef struct {
    BenchCase* head;
    BenchCase* tail;
} BenchList;

static BenchList* g_bench_list;

BenchmarkClock start_clock(unsigned int iterations) {
    return (BenchmarkClock){.iterations = iterations, .start = clock()};
}
void stop_clock(BenchmarkClock* bench_clock) {
    clock_t stop = clock();
    long double total = stop - bench_clock->start;
    bench_clock->total_time = total;
}
Metrics get_metrics(BenchmarkClock clock, TimePrefix prefix) {
    long double prefix_convert = 0;
    switch (prefix) {
        case CLB_TIMEPREFIX_NANO:
            prefix_convert = 1e9;
            break;
        case CLB_TIMEPREFIX_MICRO:
            prefix_convert = 1e6;
            break;
        case CLB_TIMEPREFIX_MILLI:
            prefix_convert = 1e3;
            break;
    }
    Metrics metrics = {
        .total_time = clock.total_time * prefix_convert / CLOCKS_PER_SEC,
        .avg_time = clock.total_time / clock.iterations,
        .iterations = clock.iterations,
        .prefix = prefix,
    };
    return metrics;
}

void clb_new_bench(const char* name, Metrics (*func)()) {
    BenchCase* new_case = malloc(sizeof(BenchCase));
    if (new_case == NULL) return;
    new_case->func = func;
    new_case->name = name;
    new_case->next = NULL;

    if (g_bench_list == NULL) {
        g_bench_list = malloc(sizeof(BenchList));
        g_bench_list->head = new_case;
        g_bench_list->tail = new_case;
    } else {
        g_bench_list->tail->next = new_case;
        g_bench_list->tail = new_case;
    }
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    BenchCase* curr = g_bench_list->head;
    while (curr != NULL) {
        Metrics res = curr->func();
        curr->result = res;
        BenchCase* next = curr->next;
        curr = next;
    }
    curr = g_bench_list->head;
    printf("BENCHMARK RESULTS: \n");
    int i = 1;
    while (curr != NULL) {
        Metrics metrics = curr->result;
        char* prefix = 0;
        switch (metrics.prefix) {
            case CLB_TIMEPREFIX_NANO:
                prefix = "ns";
                break;
            case CLB_TIMEPREFIX_MICRO:
                prefix = "mcs";
                break;
            case CLB_TIMEPREFIX_MILLI:
                prefix = "ms";
                break;
        }
        printf(
            "Benchmark %d) %s iterations=%u avg_time=%Lf%s total_time=%Lf%s\n",
            i++, curr->name, metrics.iterations, metrics.avg_time, prefix,
            metrics.total_time, prefix);
        BenchCase* next = curr->next;
        free(curr);
        curr = next;
    }
    return 0;
}
