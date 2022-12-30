#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "hist.h"
#include "hist_stru.h"
#include "predict.h"
#include "comb.h"
#include "maybe.h"

#define DO_TRACE false
#define TRACE_TO stdout
#define TRACE_DO(do_this) { if(DO_TRACE) do_this; }

MAYBE_TYPE(double)
MAYBE_FAIL(double)

MAYBE(double) predict0(const T *sample, const Hist *h);

double estimate_probability_mass(const T *sample, const Hist *h)
{

//    assert(false && "not yet implemented.");
    // determin average mass of all points around this evidence
    double est_prob = 0.0;
    size_t ndims = LEN(sample->pred.evidence);
    size_t d = 0;
    //size_t sphere[ndims]; // hyper sphere 2d-> 3^2-1 Points, 3d->3^3-1, nd->3^n-1
    static const size_t nvals = 3; // -1, 0, 1
    double vals[nvals] = {-1.0,0.0,1.0}; // nn 3d stands for up, stay, down or left, stay, right, or what alse this means in heigher dimension fields
    //for(int i=0; i<ndims; i++)
    //{
    //    sphere[i] = 0;
    //}
    size_t iSphere = 0;
    size_t iVals = 0;
    bool ready = false;
    bool count_round_finished = false;
    size_t cnt = 1;
    #define DBGIT  { printf("%zu: ", cnt); for(int i=0; i<ndims; i++) { printf("%-1.0f ", vals[sphere[i]]); } printf("\n"); cnt++;}
    double cnt_pred = 0.0;
    double sum_pred = 0.0;
    T vicinity_sample = {0};
    double dist = 1;
    double divisor = 0; //denominator
    double dividend = 0; //numerator
    do {
        TRACE_DO(printf("dist: %.1f\n", dist));
        // TODO
        // 1. generate new vicinity sample
        // 2. predict value
        // 3. if prediction avaliable ... sum prediction and count prediction
        CombinationsIteratorState *s; 
        double step;
        for(s = init_combinations_iterator(ndims,2); 
            !is_ready(s); 
            s = next_combination(s))
        {
            int *ifactors = val_vec(s);
            for(int d=0; d<ndims; d++)
            {
                assert(dist < h->cur);
                step = h->step.pred.evidence[d];
                int multiplier = val_vec(s)[d] - 1; // 
                vicinity_sample.pred.evidence[d] = sample->pred.evidence[d] + dist*(double)multiplier*step;
                TRACE_DO(
                    printf("(%d|%.1f),",
                        multiplier,
                        vicinity_sample.pred.evidence[d]
                    )
                ); //) val_vec(s)[d]);
            }
            MAYBE(double) maybe_prediction = predict0(&vicinity_sample, h);
            TRACE_DO(printf(" ===> "));
            if(IS_NOTHING2(maybe_prediction))
            {
                TRACE_DO(printf("---"));
            }
            else
            {
                ready = true; // one measurement is enough?
                divisor += 1.0;
                dividend += MAYBE_VALUE_ACCESS(maybe_prediction);
                TRACE_DO(printf("%.1f", MAYBE_VALUE_ACCESS(maybe_prediction)));
            }
            TRACE_DO(printf("\n"));
        }
        free_combinations_iterator(s);
        if(!ready)
        {
            divisor = 0;
            dividend = 0;
            dist += 1; // search in bigger vicinity
        }
        TRACE_DO(printf("\n"));
    } while(!ready);
    est_prob = dividend/divisor;
    TRACE_DO(printf("vicinity estimate: %.1f\n", est_prob));
    return est_prob;
}

MAYBE(double) predict0(const T *sample, const Hist *h)
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
    if(divisor == 0.0) // no probability mass / measures for this bucket
    {
        INIT_NOTHING(double, res_nothing)
        return res_nothing;
    }
    else
    {
        double estimate = dividend/divisor;
        INIT_SOME(double, res_some, estimate)
        return res_some;
    }
}

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
    if(divisor == 0.0) // no probability mass / measures for this bucket
    {
        return estimate_probability_mass(sample, h);
    }
    else
    {
        double estimate = dividend/divisor;
        return estimate;
    }
}