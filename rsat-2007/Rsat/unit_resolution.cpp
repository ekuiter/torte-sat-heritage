#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#include "types.h"
#include "flags.h"
#include "structures.h"
#include "globals.h"
#include "heap.h"

#include <assert.h>  //must be defined after flags.h for NDEBUG to work

#define IND(literal,clause) (literal)==(clause)->literals[1]
#define max(x,y) (x>y?x:y)
#define min(x,y) (x<y?x:y)

extern my_manager* my_m;

void double_watched_len(int index)
{
  int old_len = my_m->watched_len[index];
  int new_len = 2*old_len;

  my_clause** arr = (my_clause**)calloc(new_len,sizeof(my_clause*));
  my_clause** old = my_m->watched[index];

  for(int i=0;i<old_len;i++){
    arr[i] = old[i];
  }

  free(old);
  my_m->watched[index] = arr;
  my_m->watched_len[index] = new_len;
}

/*
 * Invariant: the watched array is filled up to 1/4 of its current length
 *
 */
void half_watched_len(int index)
{
  int size = my_m->watched_size[index];
  if(size==1){return;}

  int new_len = my_m->watched_len[index]/2;  

  //printf("Half %d --> %d\n",my_m->watched_len[index],new_len);

  my_clause** arr = (my_clause**)calloc(new_len,sizeof(my_clause*));
  my_clause** old = my_m->watched[index];

  for(int i=0;i<size;i++){
    arr[i] = old[i];
  }

  free(old);
  my_m->watched[index] = arr;
  my_m->watched_len[index] = new_len;
}

/*
 * Add a set of literals (clause) to the list of watched clauses for literal.
 * It is required that literal be in either the 0 or 1 position of clause->literals.
 *
 */
void add_watched_clause(my_lit l,my_clause* clause)
{
  int ind = watched_index(l);
  int size = my_m->watched_size[ind];
  int len = my_m->watched_len[ind];
  
  if(len<=size){
    double_watched_len(ind);
  }
  
  my_clause** wl = my_m->watched[ind];
  wl[my_m->watched_size[ind]++] = clause;
  
}//end function add_watched_clause

/* 
 * Remove clause from the list of clauses being watched by literal.
 * It is required that literal be in either the 0 or 1 position of clause->literals.
 */
void remove_watched_clause(my_lit l,my_clause* clause) 
{
  if(clause->size<=1){
    return;
  }
  
  int index = watched_index(l);
  int size = my_m->watched_size[index];
  my_clause** clauses = my_m->watched[index];

  int j = 0;
  int i;
  for(i=0;clause!=clauses[i];i++){;}
  
  j = i;
  i++;

  for(;i<size;i++){
    clauses[j++] = clauses[i];
  }

  my_m->watched_size[index]--;

  if(my_m->watched_size[index]<my_m->watched_len[index]/4){
    half_watched_len(index);
  }
}//end function remove watched

BOOLEAN set_literal(my_lit l)
{

  /* last: keeps track of literals that needs processing:
     literals that need to be processed are at [top....last-1]
     they are processed starting at top and then moving towards
     last-1 */
  int* level = my_m->level;
  my_lit* status = my_m->status;
  my_clause** reason = my_m->reason;
  my_lit* last = my_m->assign_top;
  int* watched_size = my_m->watched_size;
  int slevel = level[var(l)];
  
  implication_queue* q = my_m->imp;
  
  //put this literal (l) into the priority queue
  undo(q,var(l));
  
  *(last++) = l;

  status[var(l)] = l;
  
  //stack_offset has compensated for *(last++) above
  last += my_m->stack_offset;
  
  my_var v;

  while((v=dequeue(q))!=0){
    
    my_lit lit = status[v];
    /* Check watched clauses of nliteral
       This affects all clauses (original and learned) */
    my_lit neg_lit = neg(lit);
    int wi = watched_index(neg_lit);
    
    my_clause** watched_list = my_m->watched[wi];
    my_clause** pointer = watched_list;
    my_clause** end = watched_list+watched_size[wi];
      
    //for each clause that nliteral is beging watched in
    //find a new literal to watch
    
    //for each clause in the watched list of neg_lit
    while(watched_list<end){
      
      my_clause* clause = *(watched_list++);
      my_lit* lits = clause->lits;

      my_lit first_lit = lits[1];
      
      if(first_lit!=neg_lit){
	lits[1] = lits[0];
	lits[0] = first_lit;		
      }
      
      if(set(lits[0])){
	//this clause is already satisfied, skip it
	//put it in the same watched list (even though this current literal is already resolved!) 	 	    
	*(pointer++) = clause;
	continue;
      }
      
      int clause_size = clause->size;
      
      /* search for an unresolved literal in literals[2...] */	  
      if(clause_size>2){
	char found = 0;
	my_lit temp = lits[1];
	
	int k = 2;
	for(;k<clause_size;k++){
	  my_lit temp2 = lits[k];
	  if(unresolved(temp2)){		
	    lits[1] = temp2;
	    lits[k] = temp;
	    add_watched_clause(temp2,clause);		
	    found = 1;
	    break;
	  }

	}//end for k

	if(found){
	  //done with this clause, move on to the next clause 	  	  
	  continue;
	}
	
	//this function is different from the original find_unresolved_literal
	//in a way that it takes into account literals that are on the implication queue.
	//it regards those literal as already been set.
	//this will allow the solver to realize boolean constraints faster.
	//found=find_unresolved_literal_for_ecd(clause
	
	//find_unresolved_literal doesn't(MUSTn't) return 'the other' watched literal
	//this is checked below
	
      }//end if clause_size > 2
      
      //Knot comment 5/19/06
      //at this point, we know we could not find a new unresolved literal from lits[2..n]
      //(this is because all of them are resolved or none of them even exist)
      //all we have to do now is check whether there is a conflict or not
      //basically, if lit[0] is undef, we have a unit clause
      //if lit[0] is resolved, we have a contradiction!
	
      my_lit other_lit = lits[0];
      if(status[var(other_lit)]==neg(other_lit)){//for the else if above
		
	//the other literal has already been resolved to 0, this clause cannot be satisfied
	
	/* reset literals on stack that are yet to be processed */
	//struct literal **stack=cnf_manager->sliterals_stack_top;
	
	/*****************************/
	//here, we must pop everything out of the queue
	//and put them in the assignment stack
	//set their levels to slevel
	//move stack_top to the top of the stack
	//because every literal on the stack has a chance of participating in the conflict
	//[even if it is not processed]
	      
	while((v=dequeue(q))!=0){
	}
	
	my_m->assign_top = last;
	
	//move the rest of neg_list watched clauses into their appropriate positions
	*(pointer++) = clause;
	while(watched_list<end){
	  *(pointer++) = *(watched_list++);
	}
	
	watched_size[wi] -= watched_list-pointer;
	
	/* analyze conflict */
	analyze_conflict(clause,slevel);
	
	return 0;
	
      }else{
	
	my_lit unit_lit = other_lit;
	my_var unit_var = var(unit_lit);
		
	char add_clause = 1;	    
	
	if(status[unit_var]==0) {
	  //this literal is implied for the first time

	  reason[unit_var] = clause;
	  
	  status[unit_var] = unit_lit;
	  level[unit_var] = slevel;
	  //put this unit variable into the piq
	  undo(q,unit_var);

	  *(last++) = unit_lit;
	  
	  /*
	  if(unit_lit==plit(unit_var)){
	    my_m->p_bcp_implied[unit_var]++;
	  }else{
	    my_m->n_bcp_implied[unit_var]++;		  
	  }
	  */
	}
	
	if(add_clause){
	  *(pointer++) = clause;
	}
	
      }//end if else
      
    }//end while more clauses in watched list of neg_lit		
    
    watched_size[wi] -= watched_list-pointer;
    
  }//end while more literal to process

  my_m->assign_top = last;

  return 1;

}//end function set_literal

/* 
 * Undo all set literals at level above and down to level
 * 
 * The distance between dest and my_m->decision_level may be 
 * greater than 1.
 */
void erase_level(int dest)
{
  //check this, otherwise, erase level will erase assignments of previous levels
  if(my_m->decision_level<dest){
    printf("Attempting to erase up to level %d while at level %d\n",dest,my_m->decision_level);
    exit(0);
  }
  
  my_lit* target = my_m->decision_lit[dest];
  my_lit* stack = my_m->assign_top;
  int* level = my_m->level;
  my_lit* status = my_m->status;
  my_clause** reason = my_m->reason;
  my_lit* saved = my_m->saved;
  
  while(stack>target){
    
    stack--;
    my_lit l = *(stack);   
    my_var v = var(l);
    
    if(my_m->save_progress==1){
      //save progress
      saved[v] = l;	  
    }else if(my_m->save_progress==-1){
      //erase progress
      saved[v] = 0;
    }
    
    reason[v] = NULL;
    level[v] = 0;
    status[v] = 0;
    undo(my_m->var_order_heap,v);      
  }//end main while loop

  my_m->assign_top = stack;
    
}//end function erase level

void double_decision_lit_len()
{
  int old_len = my_m->decision_lit_size;
  int new_len = old_len*2;
  my_lit** arr = (my_lit**)calloc(new_len,sizeof(my_lit*));
  my_lit** old = my_m->decision_lit;

  
  for(int i=0;i<old_len;i++){
    arr[i] = old[i];
  }

  free(old);
  my_m->decision_lit = arr;
  my_m->decision_lit_size = new_len;

}

/*
 * Assume at most only 1/4 of the array is meaningful
 *
 *
 */
void half_decision_lit_len()
{
  int old_len = my_m->decision_lit_size;
  if(old_len==1){return;}
  int new_len = old_len/2;
  
  my_lit** arr = (my_lit**)calloc(new_len,sizeof(my_lit*));
  my_lit** old = my_m->decision_lit;

  for(int i=0;i<new_len;i++){
    arr[i] = old[i];
  }

  free(old);
  my_m->decision_lit = arr;
  my_m->decision_lit_size = new_len;

}

/*
 * set literal to 1. set this literal to be the decision literal of the level.
 * propagate its effects (calling set_literal()).
 *
 */
BOOLEAN decide_literal(my_lit l) 
{
  if(my_m->decision_level > my_m->max_decision_level) {
    my_m->max_decision_level = my_m->decision_level;
  }

  int level = ++(my_m->decision_level);
  ++(my_m->decisions);  
  
  my_var v = var(l);
  my_m->level[v] = level;
  my_m->reason[v] = NULL;

  if(level>=my_m->decision_lit_size){
    double_decision_lit_len();
  }

  my_m->decision_lit[level] = my_m->assign_top;      

  return set_literal(l);  
}

/* 
 * Undo all implications at current decision level.
 * Some of these have been derived immediately after decisions,
 * while others have been added by assert_literal
 */

void undo_decide_literal()
{
  if(my_m->decision_level <=1){
    return;
  }
  
  int level = (my_m->decision_level);
  
  erase_level(level);
  
  my_m->decision_level--;
}

BOOLEAN assert_cd_literal()
{
  if(my_m->vc>VC_THRESHOLD && my_m->decision_level<my_m->decision_lit_size/4){
    half_decision_lit_len();
  }

  my_clause* conflict_clause = my_m->conflict_clause;
  int size = conflict_clause->size;
  
  if(size>1){
    //Add conflict clause

    add_conflict_driven_clause();
  }

  my_lit fuip = conflict_clause->lits[0];
  my_var fuip_var = var(fuip);
  my_m->level[fuip_var] = my_m->assertion_level;
  
  my_m->reason[fuip_var] = (size>1?conflict_clause:NULL);

  if(size==1){
    free(conflict_clause->lits);
    free(conflict_clause);
    my_m->conflict_clause = NULL;
    my_m->simplify_orig_kb = 1;
    my_m->simplify_learned_kb = 1;
  }

  my_m->score_inc *= my_m->score_inc_factor;  
  my_m->clause_score_inc *= CLAUSE_SCORE_INC_FACTOR;

  return set_literal(fuip);
}

void enqueue(my_lit l)
{
  my_var v = var(l);
  my_m->status[v] = l;
  my_m->level[v] = 1;
  my_m->reason[v] = NULL;
  undo(my_m->imp,v);
  my_m->assign[my_m->stack_offset++] = l; 
}

BOOLEAN process_unit_literal_queue()
{
  my_var v = dequeue(my_m->imp);
  if(v!=0){
    my_m->simplify_orig_kb = 1;
    my_m->simplify_learned_kb = 1;
    my_m->stack_offset--;
    int res = set_literal(my_m->status[v]);    
    my_m->stack_offset = 0;
    return res;
  }

  return 1;
}

void undo_assert_unit_clauses()
{
    erase_level(1);
}

/**************************************************************************
 *
 * Solution related utilities
 *
 **************************************************************************/

/* prints the current solution */
void print_solution()
{
  my_lit* stack = my_m->assign;
  int size = my_m->vc;
  rprintf("instance has %d variables, stack has %d variables\n",size,my_m->assign_top - my_m->assign);
  printf("v ");
  for(int i=0;i<size;i++){    
    printf("%d ",lit_index(my_m->assign[i]));
  }
  printf("0\n");
}

/******************************************************************************
 *      debugging utilities
******************************************************************************/
