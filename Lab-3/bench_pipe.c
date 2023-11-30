#include <err.h>
#include <errno.h>
#include <limits.h>
#include <linux/fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "bench_utils.h"
#define SLEEP_TIME 1

// (0-based) will return the number of bytes to test with in each iteration.
// in every iteration the number should quadruple
int get_bitsize(int test_iteration) {
    int ret_val = 64;
    for (int i = 3; i <= test_iteration + 2; i++) {
        ret_val *= 4;
    }
    return ret_val;
}

int main(void) {
    const int num_tests = 10;
    int test_pipe[2];

    pipe2(test_pipe, O_NONBLOCK);
    fcntl(test_pipe[0], F_SETPIPE_SZ, 16777216);
    fcntl(test_pipe[1], F_SETPIPE_SZ, 16777216);

    long test = (long)fcntl(test_pipe[0], F_GETPIPE_SZ);
    if (test < 16777216) {
        printf("programm needs to be run with higher privileges (sudo)\n");
        printf(
            "otherwise the buffer-size of the pipe cant be increased so "
            "easily\n");
        return -1;
    }

    int* ticks;
    ticks = malloc(MEASUREMENTS * sizeof(int));
    if (NULL == ticks) ERROR("malloc", ENOMEM);
    memset(ticks, 0, MEASUREMENTS * sizeof(int));

    for (int i = 0; i < num_tests; i++) {
        int test_size = get_bitsize(i);
        int ticks_min = INT_MAX;
        int ticks_max = INT_MIN;
        long long ticks_all = 0;
        struct timeval tv_start;
        struct timeval tv_stop;
        double time_diff;

        int dump;

        gettimeofday(&tv_start, NULL);

        for (int j = 0; j < MEASUREMENTS; j++) {
            unsigned long long start, stop;
            char* testdata = malloc(test_size * sizeof(char));

            for (int j = 0; j < test_size; j++) {
                testdata[j] = 'a';
            }
            start = getrdtsc();

            write(test_pipe[1], testdata, sizeof(testdata));

            stop = getrdtsc();
            free(testdata);
            ticks[j] = stop - start;
        }
        gettimeofday(&tv_stop, NULL);

        // copied from bench_mmap.c
        for (int j = 0; j < MEASUREMENTS; j++) {
            if (ticks_min > ticks[j]) ticks_min = ticks[j];
            if (ticks_max < ticks[j]) ticks_max = ticks[j];
            ticks_all += ticks[j];
        }
        ticks_all -= ticks_min;
        ticks_all -= ticks_max;

        time_diff =
            ((tv_stop.tv_sec - tv_start.tv_sec) +
             ((tv_stop.tv_usec - tv_start.tv_usec) / (1000.0 * 1000.0)));

        // copied print format from bench_mmap.c to have a consisten output over
        // all benchmarks
        printf(
            "time: min:%d max:%d Ticks Avg without min/max:%f Ticks "
            "(for %d measurements) for %d Bytes (%.2f MB/s)\n",
            ticks_min, ticks_max, (double)ticks_all / (MEASUREMENTS - 2.0),
            MEASUREMENTS, test_size,
            ((double)test_size * MEASUREMENTS) / (1024.0 * 1024.0 * time_diff));
    }
}