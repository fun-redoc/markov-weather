#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "string_buffer_view.h"
#include "hist.h"
#include "hist_stru.h"
#include "predict.h"

#define DO_TRACE false
#define TRACE_TO stdout
#define TRACE_DO(do_this) { if(DO_TRACE) do_this; }

#define INITIAL_STRING_BUFFER_SIZE 1

extern Hist *h;
extern TStringBuffer sb;
extern FILE *yyin, *yyout;
extern int yylex (void);
extern int row_num;
extern int col_num;

enum TTokenType;
extern void (*register_val)(enum TTokenType type);
extern void register_train_hist(enum TTokenType type);
extern void register_validation(enum TTokenType type);

extern struct {
    struct {
        T sample;
        double prediction;
    } *arr; 
    size_t space;
    size_t next_free;
} validation;


int main(void)
{
    validation.space = 0;
    validation.next_free = 0;
    validation.arr = NULL;

    T bucks = {12.0,10.0,70.0,70.0};
    h = hist_create(&bucks, &(T){01, 800,-30,-30}, &(T){12, 1200,+40, +40});
    init_string_buffer(&sb, INITIAL_STRING_BUFFER_SIZE);
    yyin = fopen("data_train.csv", "r"); 
    assert(yyin);
    row_num = 0;
    col_num = 0;
    register_val = register_train_hist;
    yylex();
    //printf("rows=%d, cols=%d\n", row_num, col_num);
    //hist_print(stdout, h);

    yyin = fopen("data_test.csv", "r"); 
    assert(yyin);
    row_num = 0;
    col_num = 0;
    register_val = register_validation;
    yylex();

    // printout prediction
    int cnt_nan = 0;
    int cnt_valid = 0;
    double total_sqr_error = 0;
    for(int i=0; i<validation.next_free; i++)
    {
        //appl2(&repr, &h->xs[i], &h->step, mul);
        //appl2(&repr, &repr, &h->lower_bound, add);
        double measured = validation.arr[i].sample.by_name.temp_next;
        assert(!isnan(measured));
        double predicted_normalized = validation.arr[i].prediction;
        double predicted = predicted_normalized*(h->step.by_name.temp_next) + h->lower_bound.by_name.temp_next;
        if(isnan(predicted))
        {
            cnt_nan += 1;
        }
        else
        {
            cnt_valid += 1;
            assert(!isnan(predicted));
            total_sqr_error += sqrt((predicted - measured)*(predicted - measured));
        }
        //fprintf(stdout, "%d: %3.1f <- %3.1f\n", i, measured, predicted);
    }
    double avg_error = total_sqr_error/(double)cnt_valid;
    fprintf(stdout, "invalid:%d, valid:%d, avg error:%3.1f\n", cnt_nan, cnt_valid, avg_error);
    fprintf(stdout, "2022.09.21 - temprature tomorrow %3.1f\n", predict(&(T){9.0,1026.0,16.0,0.0},h)*(h->step.by_name.temp_next) + h->lower_bound.by_name.temp_next);

    hist_free(h);
    free(validation.arr);
    validation.arr = NULL;
    validation.space = 0;
    validation.next_free = 0;
    TRACE_DO(fprintf(stdout, "--------------------\n"));
    TRACE_DO(fprintf(stdout, "--------------------\n"));
    return 0;
}
