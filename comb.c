#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "comb.h"

void combinations(int buckets, int range)
{
    assert(buckets > 0);
    int vec[buckets];
    memset(&vec, 0, sizeof(int)*buckets);
    int i = buckets-1;
    bool ready = false;
    bool generate = true;
    while(!ready)
    {
       if(generate) 
       {
         for(int d=0; d<buckets-1; d++)
         {
            printf("%d,", vec[d]);
         }
        printf("%d\n", vec[buckets-1]);
       }
       vec[i]++;
       if(vec[i] > range) 
       {
        generate = false;
        i--;
        if(i < 0)
        {
            ready = true;
        }
        else
        {
            for(int k=i+1; k<buckets; k++)
            {
                vec[k] = 0;
            }
        }
       }
       else 
       {
        i = buckets-1;
        generate = true;
       }
    }
}

struct CombinationsIteratorState
{
    int buckets;
    int range;
    int i;
    bool ready;
    bool generate;
    int *vec;  // last generated combination of dimension of bucks
};
CombinationsIteratorState *init_combinations_iterator(int bucks, int range)
{
    int *vec = malloc(sizeof(int)*bucks);
    memset(vec, 0, sizeof(int)*bucks);
    CombinationsIteratorState *s = malloc(sizeof(struct CombinationsIteratorState)); 
    *s = (struct CombinationsIteratorState){bucks,range,0,false, false, vec}; 
    return s;
}
void free_combinations_iterator(CombinationsIteratorState *s)
{
    free(s->vec);
    s->vec = NULL;
    free(s);
    s = NULL;
}
bool is_ready(CombinationsIteratorState *s)
{
    return s->ready;
}
int *val_vec(CombinationsIteratorState *s)
{
    return s->vec;
}

CombinationsIteratorState *next_combination(CombinationsIteratorState *s)
{
    assert(s->buckets > 0);
    while(!s->ready)
    {
       if(s->generate) 
       {
         s->generate = false;
         return s;
       }
       s->vec[s->i]++;
       s->generate = true;
       if(s->vec[s->i] > s->range) 
       {
        s->generate = false;
        s->i--;
        if(s->i < 0)
        {
            s->ready = true;
        }
        else
        {
            for(int k=s->i+1; k<s->buckets; k++)
            {
                s->vec[k] = 0;
            }
        }
       }
       else 
       {
        s->i = s->buckets-1;
        s->generate = true;
       }
    }
    return s;
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
