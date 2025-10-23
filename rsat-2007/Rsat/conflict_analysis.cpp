#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "types.h"
#include "flags.h"
#include "structures.h"
#include "globals.h"
#include "heap.h"

#include <assert.h>  //must be defined after flags.h for NDEBUG to work

#define max(x,y) (x>y?x:y)

extern my_manager* my_m;
//int big_number = 10000000;
//int ccc = 83;

void double_stack_len()
{
  int old_len = my_m->stack_size;
  int new_len = old_len*2;
  my_lit* old_stack = my_m->stack;
  my_lit* new_stack = (my_lit*)calloc(new_len,sizeof(my_lit));
  
  for(int i=0;i<old_len;i++){
    new_stack[i] = old_stack[i];
  }

  free(old_stack);
  my_m->stack = new_stack;
  my_m->stack_size = new_len;
}

void double_save_len()
{
  int old_len = my_m->save_size;
  int new_len = old_len*2;
  int* old_save = my_m->save;
  int* new_save = (int*)calloc(new_len,sizeof(int));
  
  for(int i=0;i<old_len;i++){
    new_save[i] = old_save[i];
  }
  
  free(old_save);
  my_m->save = new_save;
  my_m->save_size = new_len;
}

void double_cdc_len()
{
  int old_len = my_m->cdc_size;
  int new_len = old_len*2;
  my_lit* old_cdc = my_m->cdc;
  my_lit* new_cdc = (my_lit*)calloc(new_len,sizeof(my_lit));
  
  for(int i=0;i<old_len;i++){
    new_cdc[i] = old_cdc[i];
  }

  free(old_cdc);
  my_m->cdc = new_cdc;
  my_m->cdc_size = new_len;  
}

inline BOOLEAN removable(my_lit l,unsigned int minl)
{
  char* seen = my_m->seen;
  int var_count = my_m->vc;
  my_lit* stack = my_m->stack;
  int stack_index = 0;
  int stack_size = my_m->stack_size;
  int* save = my_m->save;
  int save_size = my_m->save_size;
  int size = 0;
  my_clause** reasons = my_m->reason;
  int* levels = my_m->level;

  stack[stack_index++] = l;

  while(stack_index>0){
    stack_index--;
    my_lit cur = stack[stack_index];
    my_var cur_v = var(cur);

    my_clause* reason = reasons[cur_v];

    my_lit* lits = reason->lits;
    int rsize = reason->size;
    int begin = 1;

    //for each literal in the reason clause
    for(int i=begin;i<rsize;i++){     
      my_lit cur_lit = lits[i];
      my_var index = var(cur_lit);
      int level = levels[index];//cur_lit->rlevel;//cur_lit->slevel + cur_lit->rlevel;

      //if seen[index]==1, this literal involved in the conflict-->one more reason to skip l, we can ignore this current literal
      //if seen[index]==0, this literal is not in the conflict. But, perhaps, all its antecedent literals were in.

      //Assumption: cur_lit is already assigned a value in the current stack (before the conflict)
      if(!seen[index]&& level != 1){

	//assert(level>0);	
	//At this point, we know this literal is not directly in the conflict analysis.
	//Here we check if this guy's antecedent literals can all be involved in the conflict.
	//if this literal is a decision literal (the else), too bad. It does not even have any antecedent. Return false.
	//if (1<< (this literal's level)) & minl == 0, it means (guarantees) no literal of the level were present in the conflict.
	//this just implies that this current literal (cur_lit) can never satisfy the condition above, return false.
	//
	//otherwise, this literal MAY have all antecedent present in the conflict analysis, push it on the stack
	if( (reasons[index]!=NULL) && ((1<< (level)) & minl)){
	  seen[index] = 1;
	  if(stack_index>=stack_size){
	    double_stack_len();
	    stack_size = my_m->stack_size;
	    stack = my_m->stack;
	  }

	  if(size>=save_size){
	    double_save_len();
	    save_size = my_m->save_size;
	    save = my_m->save;
	  }

	  stack[stack_index++] = cur_lit;
	  save[size++] = index;
	}else{
	  //this literal, l, is NOT removable
	  for(int j=0;j<size;j++){
	    seen[save[j]] = 0;
	  }

	  return 0;
	}//end if else

      }//end if !seen
      
    }//end for i
  }

  return 1;
}

int derive_conflict_clause(my_clause* conf,int clevel,int* alevel)
{
  int num_lits_at_clevel = 0;
  my_lit next_lit = 0;
  my_lit* next_on_stack = my_m->assign_top;
  
  my_m->seen = (char*)calloc(my_m->vc+1,sizeof(char));
  char* seen = my_m->seen;

  int assertion_level = 1;
  my_m->assertion_level = 1;
  my_m->cdc_index = 0;
  //my_m->low_point = big_number;
  my_m->lits_at_clevel = 0;

  my_lit* cdc = my_m->cdc;
  int cdc_size = my_m->cdc_size;

  int* levels = my_m->level;
  my_clause** reasons = my_m->reason;

  cdc[my_m->cdc_index++] = 0;

  if(clevel<=1){
    //conflict is fatal      
    cdc[0] = 0;
    cdc[1] = 0;
    free(my_m->seen);
    my_m->seen = NULL;

    *alevel = 0;

    return 0;
  }

  //as long as we have more literals to expand
  do{

    if(conf==NULL){
      print_location();
      printf("Inside derive conflict clause [%d]\n",my_m->conflicts);
      printf("Reason of literal [%d] is NULL\n",lit_index(next_lit));
      printf("Decision literal of level %d is [%d], # lits at clevel = %d\n",clevel,lit_index(*(my_m->decision_lit[clevel])),num_lits_at_clevel);

      exit(0);
    }

    int size = conf->size;			
    my_lit* lits = conf->lits;
    int begin = (next_lit==0?0:1);
    
    for(int i=begin;i<size;i++){
    
      //inspect each literal and decide whether to
      //1) put it in the learned clause OR
      //2) expand on it
      my_lit cur_lit = lits[i];
      int index = var(cur_lit);
      int level = levels[index];

      if(!seen[index] && (level > 1) && cur_lit != next_lit){
	//if we have not seen this variable
	seen[index] = 1;
	bumpLiteralActivity(cur_lit);  //increase scores of those involve in the conflict
	
	if(level==clevel){
	  //this literal's value was set at clevel
	  //expand on it	  
	  num_lits_at_clevel++;

	}else{
	  //this literal's value was set before clevel
	  //add it to the learned clause	

	  if(my_m->cdc_index>=cdc_size){
	    double_cdc_len();
	    cdc_size = my_m->cdc_size;
	    cdc = my_m->cdc;
	  }
	  
	  cdc[my_m->cdc_index++] = cur_lit;
	  
	  
	  
	  if(level>my_m->assertion_level){
	    my_m->assertion_level = level;
	  }

	}//end if else
      }//end if seen
      
    }//end for i

    while(1){
      next_on_stack--;
      assert(next_on_stack>=my_m->assign);

      int index = var(*next_on_stack);

      if(seen[index]){
	break;
      }
    }//end while 1

    next_lit = *next_on_stack;      //next_lit is now the next literal in the stack that is involved in the conflict
    my_var next_var = var(next_lit);
    seen[next_var] = 0;  //mark next_lit not seen, because we exclude it from the learned clause

    num_lits_at_clevel--; //coz we just expanded on next_lit
    my_m->lits_at_clevel = num_lits_at_clevel;

    if(reasons[next_var]!=NULL){
      my_clause* reason;

      reason = reasons[next_var];

      //done setting reason for next_var

      conf = reason;

      if(reason->index>=0){
	//only bump the activity of those clauses learned
	bumpClauseActivity(reason);
      }

    }else{
      //we should be out of the loop next
      //do nothing here
      conf = NULL;
    }

  }while(num_lits_at_clevel>0);

  cdc[0] = neg(next_lit);
  seen[var(next_lit)] = 1;

  //
  //
  // End derivation
  //
  //

  int size = my_m->cdc_index;
  int i=size,j=size;

  unsigned int minl = 0;

  for (i = 1; i<size; i++){
    my_lit cur = cdc[i];
    int level = levels[var(cur)];
    minl |= 1 << (level & 31);       
  } 
  
  int start = 1;

  for (i = j = start; i < size; i++){
    my_lit cur = cdc[i];
    my_var cur_v = var(cur);
    if((reasons[cur_v]==NULL) || !removable(cur,minl)){
      
      int level = levels[cur_v];
      
      if(level>assertion_level){ 
	assertion_level = level; 
      }
      
      cdc[j++] = cdc[i];
    }
    
  }//end for i=j=X

  //Knot added 4/19/06
  *alevel = assertion_level;  

  int out_size = size-(i-j);

  free(seen);
  my_m->seen = NULL;

  //i-j is the number of literals in the conflict clause skipped by the minimization step
  return out_size; //return the size of the learned clause

}//end derive_conflict_clause

/* sort ints in place */
int int_cmp(const void *a, const void *b) { 
	
  int ai = *((int *)a);
  int bi = *((int *)b);

  if(ai < bi) return -1;
  else if(ai == bi) return 0;
  else return 1;
}
void sort_int(int *list,unsigned count)
{
  qsort((void *)list,(size_t)count,sizeof(int),int_cmp);
}

int var_score_vsids(struct var *);
extern struct var **vars;

void double_learned_clauses_array()
{
  int old_len = my_m->learned_clauses_array_len;
  int new_len = 2*old_len;
  my_clause** arr = (my_clause**)calloc(new_len,sizeof(my_clause*));
  my_clause** old = my_m->learned_clauses;

  double* new_score_arr = (double*)calloc(new_len,sizeof(double));
  double* old_score_arr = my_m->learned_clause_scores;

  for(int i=0;i<old_len;i++){
    arr[i] = old[i];
    new_score_arr[i] = old_score_arr[i];
  }

  free(old);
  free(old_score_arr);
  my_m->learned_clause_scores = new_score_arr;
  my_m->learned_clauses = arr;
  my_m->learned_clauses_array_len = new_len;
}

/*
 * adds the confict-driven clause captured by the conflict report,
 * and locks the literals so they will not be freed.
 *
 */
void add_conflict_driven_clause()
{
  void declare_watched_literals(my_clause*);
  
  my_clause *cdc = my_m->conflict_clause;
  int debug = 1;
  double score_inc = my_m->score_inc;
  int size = cdc->size;
  int* levels = my_m->level;

  assert(size>1);
  
  my_lit* lits = cdc->lits;
  
  int assertion_level = my_m->assertion_level;
  char found = 0;
  
  //Knot added new implementation of variable order adjustment
  //12/6/05
  for(int i=0;i<size;i++){
    my_lit l = lits[i];
    my_var v = var(l);

    //the default case!!!
    bumpLiteralActivity(l);
    
    //Knot added 4/19/06
    if(!found && levels[v]==assertion_level){
      //swap this literal with literal[1]
      my_lit temp = lits[1];
      lits[1] = lits[i];
      lits[i] = temp;
      found = 1;
    }
    
  }//end for i

  //if the clause is not unit, add it to the knowledgebase
  declare_watched_literals(cdc);
  
  if(my_m->cur_cdc_count>=my_m->learned_clauses_array_len){
    double_learned_clauses_array();
  }
  
  my_m->learned_clauses[my_m->cur_cdc_count] = cdc;
  
  my_m->learned_clause_scores[my_m->cur_cdc_count] = 0;

  /* set clause index */
  cdc->index = my_m->cur_cdc_count;

  /* update manager statistics */
  my_m->cur_cdc_count++;
  my_m->cdc_count++;
  my_m->cur_cdl_count+= size;
  my_m->cdl_count += size;

  bumpClauseActivity(cdc);

}//end function add_conflict_driven_clause

/* A conflict is reached due to a set of decisions that together 
 * lead to a contradiction with the cnf.
 * Decisions are made sequentially, so each decision has a level.
 *
 * The goal of conflict analysis is to find a set of literals from 
 *  previous levels which are sufficient
 *  to cause a conflict with the last decision
 *
 * The side effect is to set the conflict report, which contains
 * additional information about the conflict-driven clause, such
 * as:
 *  --the backtrack level: the level whose erasure will clear the conflict
 *  --the assertion level: the level which can be used to imply the
 *    conflict driven assertion, given the conflict driven clause
 */
void analyze_conflict(my_clause* conflicting_clause,int clevel)
{

  //perform check and update here!!!
  if(my_m->conflicts>=my_m->next_sp_switch){
    //flip the flag
    my_m->save_progress = 1-my_m->save_progress;
        
    //update threshold values
    int inc = 0;
    if(my_m->save_progress){
      //OFF==>ON
      my_m->on_th += my_m->on_th_inc;    
      //my_m->on_th = (int)(my_m->on_th*my_m->on_th_inc);
      inc = my_m->off_th;
    }else{
      //ON==>OFF
      my_m->off_th += my_m->off_th_inc;
      //my_m->off_th = (int)(my_m->off_th*my_m->off_th_inc);
      inc = my_m->on_th;
    }
    
    my_m->next_sp_switch += inc;
  }

  my_m->conflict_level = clevel;
  ++my_m->conflicts;

  int assertion_level = 0;

  //the normal case!!!
  int size = derive_conflict_clause(conflicting_clause,clevel,&assertion_level);
  my_m->assertion_level = assertion_level;

  if(size>0){
    
    my_clause* conflict_clause = (my_clause*)calloc(1,sizeof(my_clause));
    my_lit* lits = (my_lit*)calloc(size,sizeof(my_lit));
    my_lit* cdc = my_m->cdc;
    
    
    for(int i=0;i<size;i++){
      lits[i] = cdc[i];
    }
    
    conflict_clause->size = size;
    conflict_clause->index = 1;
    conflict_clause->lits = lits;
    my_m->conflict_clause = conflict_clause;      

  }else{
    //size==0, UNSAT
    my_m->assertion_level=0;
    my_m->conflict_clause = NULL;
  }
  
}//end function analyze_conflict
