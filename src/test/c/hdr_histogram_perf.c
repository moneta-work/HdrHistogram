#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include <stdio.h>
#include <hdr_histogram.h>
#include <locale.h>
#include <time.h>


#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>

void hdr_gettime(struct timespec* ts)
{
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts->tv_sec = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;
}


#elif __LINUX__

void hdr_gettime(struct timespec* t)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, t);
}

#else

#warning "Platform not supported\n"

#endif


struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec) < 0)
    {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec-start.tv_nsec;
    }
    else
    {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

int main(int argc, char **argv)
{
    struct hdr_histogram* histogram;
    int64_t max_value = 24 * 60 * 60 * 1000000L;
    int result = hdr_alloc(max_value, 4, &histogram);
    if (result != 0)
    {
        fprintf(stderr, "Failed to allocate histogram: %d\n", result);
        return -1;
    }

    struct timespec t0;
    struct timespec t1;
    setlocale(LC_NUMERIC, "");
    int64_t iterations = 400000000;

    for (int i = 0; i < 100; i++)
    {
        hdr_gettime(&t0);
        for (int64_t j = 1; j < iterations; j++)
        {
            hdr_record_value(histogram, j);
        }
        hdr_gettime(&t1);


        struct timespec taken = diff(t0, t1);
        double time_taken = taken.tv_sec + taken.tv_nsec / 1000000000.0;
        double ops_sec = (iterations - 1) / time_taken;

        printf("%s - %d, ops/sec: %'.2f\n", "Iteration", i + 1, ops_sec);
    }

    return 0;
}
