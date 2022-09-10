#ifndef __PREDICT_H__
#define __PREDICT_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "hist.h"

double predict(const T *sample, const Hist *h);

#endif