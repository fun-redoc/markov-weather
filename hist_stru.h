#ifndef __HIST_STRU_H__
#define __HIST_STRU_H__
#include <stdlib.h>
#include "hist.h"

const static size_t hist_dims = 4;
struct T {
    union {
        struct {
            double month;
            double pressure;
            double temp;
            double temp_next;
        } by_name;
        struct {
            double evidence[hist_dims-1];
            double proposition;
        } pred;
        double dims[hist_dims];
    };
};
struct Hist {         // implemented as resizible array
    bool normalized;  // set false each tim cs ist added, set true when sum of all cs is 1
    T bucks;          // nuber of buckets is bucks^dims
    size_t size;      // current size of the arrays xs, cs
    size_t cur;       // index of the currently last free position in the arrays
    T step;
    T lower_bound;
    T upper_bound;
    T *xs;            // bucket right vals
    double *cs;       // bucket counts, when normalized density
};
#endif