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
// define the value below to verify all data arrays used for transfering
// arbitrary data
// #define VALIDATE_TESTDATA

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

    char* tests[num_tests];
    for (int i = 0; i < num_tests; i++) {
        tests[i] = malloc(get_bitsize(i) * sizeof(char));
        for (int j = 0; j < get_bitsize(i); j++) {
            tests[i][j] = 'a';
        }
    }
#ifdef VALIDATE_TESTDATA
    // validation for data arrays
    for (int i = 0; i < num_tests; i++) {
        if (tests[0][0] != 'a') {
            printf("fatal error with filling arrays. aborting\n");
            exit(-1);
        }
        for (int j = 0; j <= get_bitsize(i); j++) {
            if (tests[i][j] != 'a') {
                printf("Array %d filled until pos. %d\n", i + 1, j);
            }
        }
    }
#endif

    // create a pipe while disabling blocking, so data does not need to be read
    // from the pipe to add new data
    pipe(test_pipe);

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

            char* buffer = malloc(16777216 * sizeof(char) * 2);

            // start single measurement
            start = getrdtsc();

            // write array to pipe
            write(test_pipe[1], tests[i], get_bitsize(i) * sizeof(char));
            read(test_pipe[0], buffer, get_bitsize(i) * sizeof(char) * 2);

            full_data += test_size;

            // stop measurement
            stop = getrdtsc();

            // save measurement
            ticks[j] = stop - start;
            free(buffer);
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
            "(for %d measurements) for %d Bytes (%.2f MiB/s)\n",
            ticks_min, ticks_max, (double)ticks_all / (MEASUREMENTS - 2.0),
            MEASUREMENTS, test_size,
            ((double)test_size * MEASUREMENTS) / (1024.0 * 1024.0 * time_diff));
    }
    printf("Full data written on this Benchmark: %0.3lfGiB\n",
           full_data / 1024.0 / 1024.0 / 1024.0);
}