#
#include <time.h>


long get_now_milli() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (long)ts.tv_sec * 1000L + (ts.tv_nsec / 1000000);
};
