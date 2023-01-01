#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "string_buffer_view.h"
#include "hist.h"
#include "hist_stru.h"
#include "predict.h"

#define DO_TRACE true
#define TRACE_TO stdout
#define TRACE_DO(do_this) { if(DO_TRACE) do_this; }

#define LEN(a) (sizeof(a)/sizeof(a[0]))
#define MAX(a,b) (a>b?a:b)

#define INITIAL_STRING_BUFFER_SIZE 1


typedef int (*Function_Main)(int, char**, int*) ;
int check_main(int argc, char* argv[], int *cur);
int forecast_main(int argc, char* argv[], int *cur);

typedef enum {
    check = 0,
    forecast  = 1
} Modes;
static struct {
    Modes mode;
    Function_Main lambda;
    char* argv;
} mode_map[] = {{check,check_main,"-check"},{forecast,forecast_main,"-forecast"}};

#ifdef UNUSED
static struct {
    Modes mode;
    union {
        struct {
            char *train_data_path;
            char *validation_data_path;
        } check;
        char* check_mode_args[2];
        struct {
            char *train_data_path;
        } forecast;
        char* forecast_mode_args[2];
    };
} mode;
#endif

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

int forecast_main(int argc, char *argv[], int *cur) 
{
    assert(cur != NULL);
    TRACE_DO(printf("forecast_main : %d %s\n", *cur, argv[*cur]));

    (*cur)++;
    assert(*cur < argc);
    char *train_file_path = argv[*cur];
    (*cur)++;
    assert(*cur < argc);
    int month;
    assert(sscanf(argv[*cur], "%d", &month) > 0);
    (*cur)++;
    assert(*cur < argc);
    double pressure_cur_day;
    assert(sscanf(argv[*cur], "%lf", &pressure_cur_day) > 0);
    (*cur)++;
    assert(*cur < argc);
    double temperature_cur_day;
    assert(sscanf(argv[*cur], "%lf", &temperature_cur_day) > 0);

    TRACE_DO(printf("today data - month:%d, preassure:%.1f, temprature:%.1f\n", month, pressure_cur_day, temperature_cur_day));

    T bucks = {12.0,10.0,70.0,70.0};
    h = hist_create(&bucks, &(T){01, 800,-30,-30}, &(T){12, 1200,+40, +40});
    init_string_buffer(&sb, INITIAL_STRING_BUFFER_SIZE);
    //yyin = fopen("data_train.csv", "r"); 
    yyin = fopen(train_file_path, "r"); 
    assert(yyin);
    row_num = 0;
    col_num = 0;
    register_val = register_train_hist;
    yylex();

    fprintf(stdout, "temprature tomorrow %3.1f\n",
                    predict(&(T){(double)month,pressure_cur_day,temperature_cur_day,0.0},h)*(h->step.by_name.temp_next) + h->lower_bound.by_name.temp_next);
    hist_free(h);
    return 0;
}

int check_main(int argc, char *argv[], int *cur) 
{
    assert(cur != NULL);
    TRACE_DO(printf("check_main : %d %s\n", *cur, argv[*cur]));

    (*cur)++;
    assert(*cur < argc);
    char *train_file_path = argv[*cur];
    (*cur)++;
    assert(*cur < argc);
    char *validation_file_path = argv[*cur];

    validation.space = 0;
    validation.next_free = 0;
    validation.arr = NULL;

    T bucks = {12.0,10.0,70.0,70.0};
    h = hist_create(&bucks, &(T){01, 800,-30,-30}, &(T){12, 1200,+40, +40});
    init_string_buffer(&sb, INITIAL_STRING_BUFFER_SIZE);
    //yyin = fopen("data_train.csv", "r"); 
    yyin = fopen(train_file_path, "r"); 
    assert(yyin);
    row_num = 0;
    col_num = 0;
    register_val = register_train_hist;
    yylex();
    fclose(yyin);
    //printf("rows=%d, cols=%d\n", row_num, col_num);
    //hist_print(stdout, h);

    //yyin = fopen("data_test.csv", "r"); 
    yyin = fopen(validation_file_path, "r"); 
    assert(yyin);
    row_num = 0;
    col_num = 0;
    register_val = register_validation;
    yylex();
    fclose(yyin);

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

    hist_free(h);
    free(validation.arr);
    validation.arr = NULL;
    validation.space = 0;
    validation.next_free = 0;
    TRACE_DO(fprintf(stdout, "--------------------\n"));
    TRACE_DO(fprintf(stdout, "--------------------\n"));
    return 0;
}

int main(int argc, char *argv[])
{
    if(argc<2)
    {
        fprintf(stderr, "ERROR: you have to provide args.\n");
    }

    int ret = -1;
    for(int a=1; a<argc; a++)
    {
        for(int i=0; i<LEN(mode_map); i++)
        {
            if(strcmp(mode_map[i].argv,argv[a]) == 0)
            {
                int local_ret = mode_map[i].lambda(argc, argv, &a);
                ret = MAX(ret, local_ret);
            }
        }
    }
    if(ret<0)
    {
        fprintf(stderr, "ERROR: invalid command line.\n");
        return 1;
    }
    return ret;
}
