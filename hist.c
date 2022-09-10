#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "comb.h"
#include "hist.h"
#include "hist_stru.h"

double gauss(double sigma, double mu, double x)
{
    // gauss density function
    return (1/(sqrt(2*PI*sigma*sigma)))*exp(-((x-mu)*(x-mu)/(2*sigma*sigma)));
}

T *zero()
{
    assert(sizeof(struct T) == sizeof(double)*hist_dims);
    T *z = malloc(sizeof(T));
    for(int i=0; i<hist_dims; i++)
    {
        z->dims[i] = 0;
    }
    return z;
}
bool smaller_or_equal(const T* v1, const T* v2)
{
   assert(v2 && v2 && LEN(v1->dims) == LEN(v2->dims));
   bool res = true;
   for(int i=0; res && i<LEN(v1->dims); i++)
   {
    res &= (v1->dims[i] <= v2->dims[i]);
   }
   return res;
}
bool eql(const T* v1, const T* v2)
{
   assert(v1);
   assert(v2);
   assert(LEN(v1->dims) == LEN(v2->dims));
   bool res = true;
   for(int i=0; res && i<LEN(v1->dims); i++)
   {
    res &= (v1->dims[i] == v2->dims[i]);
   }
   return res;
}
double sqr_dist(const T* v1, const T* v2)
{
    assert(v1 && v2 && LEN(v1->dims) == LEN(v2->dims));
    double res = 0;
    for(int i=0; i<LEN(v1->dims); i++)
    {
        double d = v1->dims[i] - v2->dims[i];
        res += d*d;
    }
    return res;
}

double add(double a, double b) {
    return a + b;
}
void sum(T *s, const T* v1, const T* v2)
{
    appl2(s, v1, v2, add);
}
void T_printf(FILE *f, const T *v)
{
    fprintf(f, "(m=%.0f, p=%5.1f,t=%6.1f => t1=%6.1f)",v->by_name.month, v->by_name.pressure, v->by_name.temp, v->by_name.temp_next);
}

double subtract(double a, double b)
{
    return a - b;
}
void delta(T *d, const T* v1, const T* v2)
{
    appl2(d, v1, v2, subtract);
}
void divide(T *res, const T *v, const int parts)
{
    assert(res && v && (parts > 0) && LEN(res->dims) == LEN(v->dims));
    for(int i=0; i<LEN(v->dims); i++)
    {
        res->dims[i] = v->dims[i]/parts;
    }
}
double ddiv(double a, double b)
{
    return a/b;
}
void divide2(T *res, const T *v, const T *m)
{
    appl2(res, v, m, ddiv);
}
void modul(T *res, const T *v, const T *m)
{
    appl2(res, v, m, fmod);
}
void appl1(T *res, const T *v, double (*f)(double))
{
    assert(res);
    assert(v);
    assert(f);
    assert(LEN(v->dims) == LEN(res->dims));
    for(int i=0; i<LEN(v->dims); i++)
    {
        res->dims[i] = f(v->dims[i]);
    }
}
void appl2(T *res, const T *v, const T *m, double (*f)(double,double))
{
    assert(res);
    assert(v);
    assert(m);
    assert(f);
    assert(LEN(v->dims) == LEN(m->dims));
    assert(LEN(m->dims) == LEN(res->dims));
    for(int i=0; i<LEN(v->dims); i++)
    {
        res->dims[i] = f(v->dims[i], m->dims[i]);
    }
}
Hist *hist_create(const T *bucks, const T* left, const T* right)
{
    // n: number of buckets in each direction
    assert(smaller_or_equal(left,right));
    Hist *res = malloc(sizeof(Hist));
    res->upper_bound = *right;
    res->lower_bound = *left;
    T d;
    delta(&d, right, left);
    appl2(&(res->step), &d, bucks, ddiv);
    res->bucks = *bucks;
    res->cur = 0;
    res->size = 1; //bucks; // small initial value, than realloc
    res->xs = malloc(sizeof(T)*res->size);
    res->cs = malloc(sizeof(*(res->cs))*res->size);
    memset(res->cs,(*(res->cs))-(*(res->cs)),res->size); // fill with 0 value regardless of type
    return res;
}
void hist_free(Hist *h)
{
    free(h->xs);
    free(h->cs);
    free(h);
    h=NULL;
}
double mul(double a, double b)
{
    return a*b;
}
void hist_print(FILE *f, const Hist *h)
{
    assert(h);
    for(size_t i=0; i < h->cur; i++)
    {
        T repr;
        appl2(&repr, &h->xs[i], &h->step, mul);
        appl2(&repr, &repr, &h->lower_bound, add);
        //T_printf(f, &h->xs[i]);
        T_printf(f, &repr);
        fprintf(f, "|");
        for(size_t j = 0; j < h->cs[i]; j++)
        {
            fprintf(f, "=");
        }
        fprintf(f, "\n");
    }
}
double ModifiedTrunc(double v) {
    return v < 0 ? floor(v) : trunc(v);
}
double div_and_tunc(double v, double m)
{
    return ModifiedTrunc(v/m);
}
double min(double a, double b)
{
    return a>b?b:a;
}
double max(double a, double b)
{
    return a>b?a:b;
}

void normalize_to_bucket(T *normalized, const T *original, const Hist *h){
   delta(normalized, original, &h->lower_bound);
   appl2(normalized, normalized, &h->step, div_and_tunc);
}
void hist_add_sample(Hist *h, const T *t)
{
    assert(h);
    assert(h->size > 0 && h->cs && h->xs);
    //assert(!smaller(t, &h->lower_bound));
    // 1. find representative value
    // .  under lower bound map to -1
    // .  over upper bound map to number of buckets
    // repr = map_bounds(trunc((v - lower) / step))
    T repr;
    //delta(&repr, t, &h->lower_bound);
    //appl2(&repr, &repr, &h->step, div_and_tunc);
    normalize_to_bucket(&repr, t, h);
    T min_bound;
    T max_bound;
    for(int i=0; i<LEN(min_bound.dims); i++)
    {
        min_bound.dims[i] = -1;
        max_bound.dims[i] = h->bucks.dims[i];
    }
    appl2(&repr, &repr, &min_bound, max);
    appl2(&repr, &repr, &max_bound, min);
    // 2. find position of the representative value in array
    #define IDX_NOT_DEFINED -1
    int idx_repr = IDX_NOT_DEFINED;
    for(int i=0; idx_repr == -1 && i<h->cur; i++)
    {
        if(eql(&(h->xs[i]), &repr))
        {
            idx_repr = i;
            break;
        }
    }
    if(idx_repr == IDX_NOT_DEFINED)
    {
        if(h->cur >= h->size)
        {
            assert((int)(65534/2) > (h->size + 1));
            h->size *= 2;
            h->xs = realloc(h->xs, sizeof(T)*h->size);
            h->cs = realloc(h->cs, sizeof(*(h->cs))*h->size);
        }
        idx_repr = h->cur;
        h->xs[idx_repr] = repr;
        h->cs[idx_repr] = 0;
        h->cur += 1;
    }
    h->cs[idx_repr] += 1;
    // 2a. if no position found append new representative value
    // TODO
    // 3. increase the counter of the representative value
    // TODO
}
void hist_nomalise_probs(Hist *h)
{
    assert(NULL && "uninmplemented");
}
double density(const Hist *h, const T *v)
{
    assert(NULL && "uninmplemented");
}

int main_hist_test(void)
//int main(void)
{
    Hist *h = hist_create(&(T){12, 10,70,70}, &(T){1, 800,-30,-30}, &(T){12, 1200,+40, +40});
    hist_add_sample(h, &(T){3, 1100, 15,  -19});
    hist_add_sample(h, &(T){3, 1101, 15,  -18});
    hist_add_sample(h, &(T){3, 1301, 15,  51});
    hist_add_sample(h, &(T){3, 799,  15, -29});
    hist_add_sample(h, &(T){3, 700,  15, -40});
    hist_add_sample(h, &(T){3, 700,  15, -50});
    hist_add_sample(h, &(T){3, 800,  15, -30});
    hist_add_sample(h, &(T){3, 800,  15, -31});
    hist_add_sample(h, &(T){3, 1200, 15,  +40});
    hist_add_sample(h, &(T){3, 1000, 20,  25});
    hist_print(stdout, h);
    fprintf(stdout, "--------------------\n");
    for(int i=0; i<h->cur; i++)
    {
        //appl2(&repr, &h->xs[i], &h->step, mul);
        //appl2(&repr, &repr, &h->lower_bound, add);
        double measured_normalized = h->xs[i].by_name.temp_next;
        double measured = measured_normalized*(h->step.by_name.temp_next) + h->lower_bound.by_name.temp_next;
        fprintf(stdout, "%d: %3.1f <- %3.1f\n", i, measured_normalized, measured);
    }
    return 0;
}

int main_test_combinations_operator(void)
{
    int buckets = 2;
    CombinationsIteratorState *s; 
    for(s = init_combinations_iterator(buckets,3); 
        !is_ready(s); 
        s = next_combination(s))
    {
        for(int d=0; d<buckets-1; d++)
        {
           printf("%d,", val_vec(s)[d]);
        }
        printf("%d\n", val_vec(s)[buckets-1]);
    }
    free_combinations_iterator(s);
    return 0;
}