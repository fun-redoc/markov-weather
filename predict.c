
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "hist.h"
#include "hist_stru.h"
#include "predict.h"

double predict(const T *sample, const Hist *h)
{
    T repr;
    normalize_to_bucket(&repr, sample, h);
    double divisor = 0; //denominator
    double dividend = 0; //numerator
    for(int i=0; i<h->cur; i++)
    {
        bool matches = true;
        for(int j=0; matches && j<LEN(repr.pred.evidence); j++)
        {
            matches &= (repr.pred.evidence[j] == h->xs[i].pred.evidence[j]);
        }
        if(matches)
        {
            divisor += h->cs[i];
            dividend += h->cs[i] * h->xs[i].pred.proposition;
        }
    }
    //assert(divisor!=0.0);
    double estimate = dividend/divisor;
    return estimate;
}