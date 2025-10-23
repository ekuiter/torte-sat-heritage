#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "types.h"
#include "flags.h"
#include "structures.h"
#include "globals.h"
#include "heap.h"

#include <assert.h>  //must be defined after flags.h for NDEBUG to work

extern clock_t start_t;
extern my_manager* my_m;

void print_watched_list(my_lit l)
{
  int index = watched_index(l);
  my_clause** list = my_m->watched[index];
  int size = my_m->watched_size[index];
  
  printf("\nWatched list of literal [%d]\n",lit_index(l));
  for(int i=0;i<size;i++){
    printf("%d) index:%d :",i,list[i]->index);
    print_clause(list[i]);		
  }
}

void free_cnf_manager()
{
  my_clause** clauses = my_m->original_clauses;
  int size = my_m->cur_cc;
  /* free original clauses */
  for(int j=0; j<size; j++) {
    my_clause* clause = clauses[j];
    free(clause->lits);
    free(clause);
  }
  
  free(my_m->original_clauses); /* this takes care of freeing individual clause structures */

  clauses = my_m->learned_clauses;
  size = my_m->cur_cdc_count;
  /* free original clauses */
  for(int j=0; j<size; j++) {
    my_clause* clause = clauses[j];
    if(clause!=NULL){
      free(clause->lits);
      free(clause);
    }
  }
  
  free(my_m->learned_clauses); /* this takes care of freeing individual clause structures */

  size = my_m->vc*2;
  my_clause*** watched = my_m->watched;
  for(int i=0;i<size;i++){
    my_clause** w = watched[i];
    free(w);
  }

  free(my_m->watched);
  free(my_m->watched_len);
  free(my_m->watched_size);

  free(my_m->cdc);
  free(my_m->assign);
  free(my_m->decision_lit);
  free(my_m->score);
  free(my_m->reason);
  free(my_m->status);
  free(my_m->level);
  if(my_m->seen!=NULL){
    free(my_m->seen);
  }

  free(my_m->saved);

  
  free(my_m->stack);
  free(my_m->save);
  
  free_heap(my_m->var_order_heap);
  free(my_m->var_order_heap);
    
  free(my_m->learned_clause_scores);
  
  free_heap(my_m->imp);
  free(my_m->imp);

  free(my_m);
}//end function free_cnf_manager

/*
 * Assumes that the first two literals in the clause are the ones
 * being watched.
 */
void declare_watched_literals(my_clause* c)
{
  void add_watched_clause(my_lit,my_clause*);

  if(c->size<2){
    printf("Attempting to declare watched list on a unit clause:");
    print_clause(c);
  }
  
  my_lit* lits = c->lits;
  
  add_watched_clause(lits[0],c);
  add_watched_clause(lits[1],c);    
}

/*
 * Initialize watched literals for original set of clauses.
 */
void initialize_watched_literals()
{
  my_clause*** watched = my_m->watched;
  int* watched_len = my_m->watched_len;
  int size = 2*my_m->vc;

  //initialize watched_clauses counts to 0, vectors to NULL
  //for each variable
  for(int i=0; i<size; i++) {	      
    watched[i] = (my_clause**)calloc(1,sizeof(my_clause*));
    watched_len[i] = 1;
  }//end for i
  
  my_clause** clauses = my_m->original_clauses;
  size = my_m->cc;
  //the two watched literals per clause are the first two in the list of literals
  //for each clause
  for(int j=0; j<size; j++) {
    my_clause* c = clauses[j];
    int csize = c->size;
    declare_watched_literals(c);
  }//end for j

}

void print_stats()
{
  rprintf("CNF stats: (%d vars, %d clauses)\n",my_m->vc,my_m->cc);
  rprintf("Decisions: %d\n",my_m->decisions);
  rprintf("Conflicts: %d\n",my_m->conflicts);
  //rprintf("Max decision level: %d\n",my_m->max_decision_level);

  //rprintf("Final decision lit size:%d (%.2f %%)\n",my_m->decision_lit_size,(double)my_m->decision_lit_size*100/my_m->vc);

  //rprintf("Final stack/save sizes: %d(%.2f %%)/%d(%.2f %%)\n",my_m->stack_size,(double)my_m->stack_size*100/my_m->vc,my_m->save_size,(double)my_m->save_size*100/my_m->vc);

  //rprintf("Final cdc size: %d (%.2f %%)\n",my_m->cdc_size,(double)my_m->cdc_size*100/my_m->vc);
  
}//end function print_stats

void print_clause(my_clause* c)
{
  if(c==NULL){printf("(NULL)\n"); return;}
  
  int size = c->size;
  my_lit* lits = c->lits;
  my_lit* status = my_m->status;
  int* level = my_m->level;
  double* score = my_m->score;
  printf("(");
  for(int i=0;i<size;i++){
    my_var v = var(lits[i]);
    printf("%d%s(%d), ",lit_index(lits[i]),(status[v]==0?"":(status[v]==lits[i]?"s":"r")),level[v]);
  }//end for i
  printf(")\n\n");
}

//Great for debugging
void print_location()
{
  printf("Dec=%d,Conf=%d\n",my_m->decisions,my_m->conflicts);
}
