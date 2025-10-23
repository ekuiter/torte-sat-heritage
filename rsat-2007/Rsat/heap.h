#include "structures.h"

void init_heap(heap* h,CNF_VAR_INDEX n);
my_var getMin(heap* h);
void insert(heap* h,my_var v);
void increase(heap* h,int v);
void update(heap* h,int v);
void undo(heap* h,my_var v);
my_var select_next_var();
my_var dequeue(implication_queue* q);
void rescaleVariableActivity();
void bumpLiteralActivity(my_lit l);
void bumpClauseActivity(my_clause* c);
void free_heap(heap* h);
//void shuffle_var_order(cnf_manager*);
