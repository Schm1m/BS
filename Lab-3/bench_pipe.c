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
#include <time.h>
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
    long long full_data = 0;
    const int num_tests = 10;  // 10th test will be 16MB
    int test_pipe[2];

    // create a pipe while disabling blocking, so data does not need to be read
    // from the pipe to add new data
    pipe2(test_pipe, O_NONBLOCK);

    // increase pipe buffer to 16MB
    fcntl(test_pipe[1], F_SETPIPE_SZ, 16777216);

    // check if run with higher privileges. needed for increased pipe buffer
    long test = (long)fcntl(test_pipe[0], F_GETPIPE_SZ);
    if (test < 16777216) {
        printf("programm needs to be run with higher privileges (sudo)\n");
        printf(
            "otherwise the buffer-size of the pipe cant be increased so "
            "easily\n");
        return -1;
    }

    // copied from bench_mmap.c
    // allocate and initiate space in memory to store measurements
    int* ticks;
    ticks = malloc(MEASUREMENTS * sizeof(int));
    if (NULL == ticks) ERROR("malloc", ENOMEM);
    memset(ticks, 0, MEASUREMENTS * sizeof(int));

    time_t t_start = time(NULL);

    for (int i = 0; i < num_tests; i++) {
        int test_size = get_bitsize(i);
        int ticks_min = INT_MAX;
        int ticks_max = INT_MIN;
        long long ticks_all = 0;
        struct timeval tv_start;
        struct timeval tv_stop;
        double time_diff;

        gettimeofday(&tv_start, NULL);

        for (int j = 0; j < MEASUREMENTS; j++) {
            unsigned long long start, stop;

            // create array, allocate memory and fill with 'a' to create
            // arbitrary data to test with
            char* testdata = malloc(test_size * sizeof(char));
            for (int j = 0; j < test_size; j++) {
                testdata[j] = 'a';
            }

            // start single measurement
            start = getrdtsc();

            // write array to pipe
            write(test_pipe[1], testdata, sizeof(testdata));
            full_data += test_size;

            // stop measurement
            stop = getrdtsc();

            // free data array. otherwise huge memory leak
            free(testdata);

            // save measurement
            ticks[j] = stop - start;
        }
        gettimeofday(&tv_stop, NULL);

        // copied from bench_mmap.c
        // tbh no idea yet what it does, but it works. will update comments if i
        // understood it
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
    time_t t_end = time(NULL);
    double diff = difftime(t_end, t_start);
    printf("Benchmark took %lf minutes\n", diff/60);
    printf("Full data written on this Benchmark: %0.3lfGB\n", full_data/1000.0/1000.0/1000.0);
}