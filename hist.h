#ifndef __HIST_H__
#define __HIST_H__
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#define PI 3.14159265358979323846
#define SQR(x) ((x)*(x))
#define LEN(a) (sizeof(a)/sizeof(a[0]))

double gauss(double sigma, double mu, double x);

typedef struct T T;
T *zero();
bool smaller_or_equal(const T* v1, const T* v2);
bool eql(const T* v1, const T* v2);
void sum(T *s, const T* v1, const T* v2);
void delta(T *d, const T* v1, const T* v2);
void divide(T *res, const T *v, const int parts);
void divide2(T *res, const T *v, const T *m);
void modul(T *res, const T *v, const T *m);
void appl2(T *res, const T *v, const T *m, double (*f)(double,double));
void appl1(T *res, const T *v, double (*f)(double));
double dist(const T* v1, const T* v2);
void T_printf(FILE *f, const T *v);

typedef struct Hist Hist;
Hist *hist_create(const T *n_buckets, const T* left, const T* right);
void hist_free(Hist *h);
void hist_print(FILE *f, const Hist *h);
void hist_add_sample(Hist *h, const T *t);
void normalize_to_bucket(T *normalized, const T *original, const Hist *h);
void hist_nomalise_to_probs(Hist *h);
double density(const Hist *h, const T *v);


#endif // __HIST_H__