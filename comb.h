#ifndef __COMB_H__
#define __COMB_H__

void combinations(int buckets, int range);

typedef struct CombinationsIteratorState CombinationsIteratorState;
CombinationsIteratorState *init_combinations_iterator(int bucks, int range);
void free_combinations_iterator(CombinationsIteratorState *s);
bool is_ready(CombinationsIteratorState *s);
int *val_vec(CombinationsIteratorState *s);
CombinationsIteratorState *next_combination(CombinationsIteratorState *s);

#endif