%{
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include <ctype.h>
    #include <assert.h>
    #include "string_buffer_view.h"
    #include "hist.h"
    #include "hist_stru.h"
    #include "predict.h"
    #define INITIAL_STRING_BUFFER_SIZE 1

    Hist *h = NULL;

    typedef enum {EMPTY,NUMBER,STRING,LINE_END,CELL_END} TTokenType;
    TStringBuffer sb;
    int row_num;
    int col_num;
    int skip_rows = 0;
    int relevant_cols[] = {1, 12, 13};
    void register_month(void);
    void register_temp(void);
    void register_pressure(void);
    struct {
        int col;
        void (*lambda)(void);
    } register_map[] = {{1,register_month}, {12,register_pressure}, {13,register_temp}};
    union 
    {
        double      valNum;
        TStringView valStr;
    } yylval;
    T prev = {0};
    T cur = {0};
    void (*register_val)(TTokenType type) = NULL;


    struct {
        struct {
            T sample;
            double prediction;
        } *arr; 
        size_t space;
        size_t next_free;
    } validation;

    void cleanup()
    {
        clear_string_buffer(&sb);
    }

    void yyerror(char *s) {
        fprintf(stdout, "YYERR: %s\n", s);
        cleanup();
        exit(1);
    }

    bool is_relevant_col(int c)
    {
        // TODO slow, better use bitfield
        bool res = false;
        for(int i=0; !res && i<LEN(relevant_cols); i++)
        {
            res |= (relevant_cols[i] == c);
        }
        return res;
    }

    void print_token(TTokenType type)
    {
        switch(type)
        {
            case STRING:
                printf("(%d,%d) -> %s\n", row_num, col_num, get_string(&yylval.valStr));
            break;
            case NUMBER:
                printf("(%d,%d) -> %.4f\n", row_num, col_num, yylval.valNum);
            break;
            default: assert(NULL && "Unexpeted type.");
        }
    }


    void register_month(void)
    {
        cur.by_name.month = fmod(trunc(yylval.valNum / 100), 100);
        //printf("Month (%d,%d) -> %.4f %.1f\n", row_num, col_num, yylval.valNum, cur.by_name.month);
    }
    void register_temp(void)
    {
        cur.by_name.temp = yylval.valNum;
        //printf("Temp (%d,%d) -> %.4f\n", row_num, col_num, cur.by_name.temp);
    }
    void register_pressure(void)
    {
        cur.by_name.pressure = yylval.valNum;
        //printf("Pressure (%d,%d) -> %.4f\n", row_num, col_num, cur.by_name.pressure);
    }

    void register_train_hist(TTokenType type)
    {
        if(type == LINE_END)
        {
            if(!eql(&(T){0}, &prev))
            {
                prev.by_name.temp_next = cur.by_name.temp;
                //T_printf(stdout, &prev);
                //fprintf(stdout, "\n");
                hist_add_sample(h, &prev);
            }
            prev = cur;
            cur = (T){0};
        }
        if(row_num >= skip_rows && is_relevant_col(col_num))
        {
            //print_token(type);
            void (*reg_fn)(void) = NULL;
            for(int i=0; !reg_fn &&i<LEN(register_map); i++)
            {
                if(register_map[i].col == col_num)
                {
                    reg_fn = register_map[i].lambda;
                }
            }
            if(reg_fn)
            {
                reg_fn();
            }
        }
    }


    void validation_append(T *sample, double prediction)
    {
        if(validation.next_free >= validation.space)
        {
            if(validation.space == 0)
            {
                assert(validation.arr == NULL);
                // TODO avoid manually computing size
                validation.arr = malloc(sizeof(T)+sizeof(double));
                validation.space = 1;
            }
            else
            {
                assert(validation.arr);
                validation.space *= 2;
                validation.arr = realloc(validation.arr, validation.space*sizeof(validation.arr[0]));
            }
        }
        validation.arr[validation.next_free].sample = *sample;
        validation.arr[validation.next_free].prediction = prediction;
        validation.next_free += 1;
    }

    void register_validation(TTokenType type)
    {
        if(type == LINE_END)
        {
            if(!eql(&(T){0}, &prev))
            {
                prev.by_name.temp_next = cur.by_name.temp;
                double prediction = predict(&prev, h);
                validation_append(&prev, prediction);
            }
            prev = cur;
            cur = (T){0};
        }
        if(row_num >= skip_rows && is_relevant_col(col_num))
        {
            void (*reg_fn)(void) = NULL;
            for(int i=0; !reg_fn &&i<LEN(register_map); i++)
            {
                if(register_map[i].col == col_num)
                {
                    reg_fn = register_map[i].lambda;
                }
            }
            if(reg_fn)
            {
                reg_fn();
            }
        }
    }
%}

%option noyywrap

%%
[ \t]+       ;  /* ignore whitespace */

[+-]?[0-9]+(\.[0-9]+)?([eE][0-9]+)? {
/* -- NUM -- */
    yylval.valNum = atof(yytext);
    register_val(NUMBER);
    //return NUMBER;
}

\"([^"]|\"\")*\" {
/* -- STRING with quotes -- */
	/* yyleng is precomputed strlen(yytext) */
    size_t n_spaces = 0;
    //while(yytext[n_spaces]!='"') n_spaces++;
    size_t i=0, n = yyleng-n_spaces;
    char s[n];
    char *sp;

    if (n == 2) // only 2 quotes
    {
        //return EMPTY;
    }

    memset(s,0, n*sizeof(char));
    sp = s;

	/* copy yytext, changing "" to " */
    for (i = 1 /*skip 0="*/; i < n-1; i++)
    {
        *sp++ = yytext[n_spaces+i];
        if (yytext[n_spaces + i] == '"')
            i++; /* skip second one */
    }
    TStringView sw = append_string_buffer(&sb,s);
    yylval.valStr = sw;
    register_val(STRING);
    //return STRING;
}


[^=";\r\n \t]+[^";\r\n]* { 
/* -- STRING w/o quotes -- */
    if(strlen(yytext)==0)
    {
        //return EMPTY;
    }
    double d = -1;
    int r = sscanf(yytext, "%lf", &d);
    //printf("scanfres %d %lf\n", r, d);
    // scan rules are ambiguous, distinguish NUM here
    // TODO find better idea using lex rules if possible
    if(r)
    {
        yylval.valNum = d;
        register_val(NUMBER);
        //return NUMBER;
    }
    else
    {
        TStringView sw = append_string_buffer(&sb,yytext);
        yylval.valStr = sw;
        register_val(STRING);
        //return STRING;
    }
}

\n|\r\n   { 
/* -- Line End -- */
    row_num++;
    col_num = 0;
    register_val(LINE_END);
    //assert(row_num < 9);
    //return LINE_END; 
}

\; { 
/* -- Cell End -- */
    col_num++;
    //return CELL_END; 
}

. {
    yyerror("Unknown character");
}

%%

//static int yywrap(void) {
//    return 1;
//}

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
    fprintf(stdout, "--------------------\n");
    fprintf(stdout, "--------------------\n");
    return 0;
}