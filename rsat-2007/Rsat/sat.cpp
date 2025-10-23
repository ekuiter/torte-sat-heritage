#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <math.h>

#include "flags.h"
#include "types.h"
#include "structures.h"
#include "globals.h"
#include "heap.h"

BOOLEAN progress_printed = 0;
clock_t start_time,end_time;

/* Commented out for SAT07 submission
//these global varables are used for calculating interesting information to print during the execution of the solver.
int previous_decision_count = 0;
int previous_conflicts = 0;
int previous_reduce_kb_count = 0;
int previous_simplify_kb_count = 0;
int previous_compact_kb_count = 0;
int previous_lits_removed = 0;
int previous_remove_attempts = 0;
int previous_subsume_kb = 0;
int previous_implications = 0;
int prev_decisions = 0;
int prev_conflicts = 0;
int previous_wasted = 0;
int wasted_propagation_literal_count = 0;
int multiple_backtrack = 0;
int base_unit_clause_learned;
*/

extern my_manager* my_m;

#define max(x,y) (x>y?x:y)

double var_score_vsids(struct var* v);

/*
 * remove this clause from the reason array
 * remove this clause from any watched list
 * free this clause (complete)
 *
 */
void remove_clause(my_clause* c)
{
  my_clause** reason = my_m->reason;
  my_var v0 = var(c->lits[0]);
  my_var v1 = var(c->lits[1]);

  if(reason[v0]==c){
    reason[v0] = NULL;
  }else if(/*c->size>1 &&*/ reason[v1]==c){
    reason[v1] = NULL;
  }

  remove_watched_clause(c->lits[0],c);
  remove_watched_clause(c->lits[1],c);
  
  free(c->lits);
  free(c);
}

/*
 * returns 1 if this clause is satisfied by some literals at level 1 (topmost).
 * (returning 1 means this clause can be removed from the knowledgebase)
 */
BOOLEAN simplify(my_clause* c)
{
  my_lit* lits = c->lits;
  my_lit* status = my_m->status;

  for(int i=0;i<c->size;i++){
    if(set(lits[i])>0){
      //this clause is satisfied
      return 1;
    }

  }//end for i

  return 0;
}

/*
 * This function should be called after BCP at the topmost level.
 * It removes all clauses in the knowledgebase that are satisfied at the topmost level (1). 
 *
 */
void simplify_kb()
{
  if(!my_m->simplify_learned_kb){return;}

  int size = my_m->cur_cdc_count;

  if(size==0){return;}

  int removed_lits = 0;
  int j=0;
  my_clause** clauses = my_m->learned_clauses;
  double* score = my_m->learned_clause_scores;

  for(int i=0;i<size;i++){
    my_clause* c = clauses[i];
    if(simplify(c)){
      removed_lits += c->size;
      remove_clause(c);
    }else{
      clauses[i]->index = j;
      score[j] = score[i];
      clauses[j++] = clauses[i];
    }
  }

  my_m->cur_cdc_count = j;
  my_m->cur_cdl_count -= removed_lits;

  my_m->db_simplification_count++;

  my_m->num_conflicts_for_next_simplify = my_m->conflicts+my_m->next_simplify_increment;
  double removed_ratio = ((double)(size-my_m->cur_cdc_count))/size;

  if(removed_ratio<0.01){
    my_m->next_simplify_increment = (int)(my_m->next_simplify_increment*1.1);
  }else{
    my_m->next_simplify_increment = (int)(my_m->next_simplify_increment*0.9);;
  }
  
  my_m->simplify_learned_kb = 0;

}//end function simplify_kb

void simplify_original_kb()
{
  
  if(!my_m->simplify_orig_kb){return;}

  int size = my_m->cur_cc;
  int removed_lits = 0;
  int j = 0;
  my_clause** clauses = my_m->original_clauses;

  for(int i=0;i<size;i++){
    my_clause* c = clauses[i];
    
    if(simplify(c)){
      removed_lits += c->size;
      remove_clause(c);
    }else{
      clauses[j++] = clauses[i];
    }
  }//end for i
  
  my_m->cur_cc = j;
  my_m->cur_lit_count -= removed_lits;

  //memory usage optimization
  if(j<my_m->original_clauses_array_len*0.75){
    my_m->original_clauses = (my_clause**)realloc(my_m->original_clauses,sizeof(my_clause*)*j);
    my_m->original_clauses_array_len = j;
  } 
  
  my_m->simplify_orig_kb = 0;
}

/*
 * returns 1 if c is a reason for some literal, 0 otherwise.
 *
 */
BOOLEAN locked(my_clause* c)
{
  if(c->size<=1){
    return 1;
  }
  
  my_clause** reason = my_m->reason;

  my_clause* reason1 = reason[var(c->lits[0])];
  my_clause* reason2 = reason[var(c->lits[1])];

  return (reason1==c || reason2==c);
}

/*
 * Attempts to remove approximately half the clauses in the knowledge base.
 * 
 */
void reduce_kb()
{
  
  int i;
  int num_learned_clauses = my_m->cur_cdc_count;

  if(num_learned_clauses<=0){
    return;
  }

  //after this point we can assume that num_learned_clauses is at least 1  
  double extra_lim = (double)(my_m->clause_score_inc)/num_learned_clauses;
  

  //sort the learned clauses by their activities
  double lg = log2(num_learned_clauses);
  int new_size = (int)lg;

  if(lg!=(int)lg){
    new_size = (int)(ceil(lg));
    new_size = (int)(pow(2,new_size));
  }

  my_clause** arr = (my_clause**)calloc(new_size,sizeof(my_clause*));
  double* score_arr = (double*)calloc(new_size,sizeof(double));

  for(int i=0;i<num_learned_clauses;i++){
    arr[i] = my_m->learned_clauses[i];
    score_arr[i] = my_m->learned_clause_scores[i];
  }
  
  for(int i=num_learned_clauses;i<new_size;i++){
    arr[i] = NULL;
    score_arr[i] = -100;
  }
  
  sort_clauses_by_activity(&arr,&score_arr,new_size);
  
  int kk = 0;
  for(int i=new_size-num_learned_clauses;i<new_size;i++){
    my_m->learned_clauses[kk] = arr[i];
    my_m->learned_clause_scores[kk] = score_arr[i];
    kk++;
  }//end for i

  free(arr);
  free(score_arr);

  my_clause** clause_array = my_m->learned_clauses;
  double* score = my_m->learned_clause_scores;

  int removed_lits = 0;
  int j = 0;
  //iterate through the first half of the sorted array
  for(i=0;i<num_learned_clauses/2;i++){

    char binary = 0;

    if(clause_array[i]->size <= 2){
      binary = 1;
    }

    if(!locked(clause_array[i]) && !binary){
      //remove it
      removed_lits += clause_array[i]->size;
      remove_clause(clause_array[i]);
    }else{      
      clause_array[i]->index = j;
      score[j] = score[i];

      clause_array[j++] = clause_array[i];
    }
  }//end for i=j

  //iterate through the second half of the sorted array
  for(;i<num_learned_clauses;i++){
    my_clause* cur = clause_array[i];

    char binary = 0;

    if(clause_array[i]->size <= 2){
      binary = 1;
    }

    if(!locked(cur) && ( score[i] < extra_lim) && !binary){
      //remove it
      removed_lits += clause_array[i]->size;
      remove_clause(clause_array[i]);
    }else{
      clause_array[i]->index = j;
      score[j] = score[i];
      clause_array[j++] = clause_array[i];
    }
  }//end for i

  my_m->cur_cdc_count = j;
  my_m->cur_cdl_count -= removed_lits;
  my_m->db_reduction_count++;

  my_m->reduced = 1;
  
  my_m->max_learned_clauses *= MAX_LEARNED_CLAUSES_MULTIPLIER;

}

void cancelUntil(int dest)
{
  while(my_m->decision_level>=dest){
    undo_decide_literal();
  }
}

int sat_aux()
{
  
  int result = -2;

  while(result==-2){
        
    while(1){        
      
      if(result==0){
	break;
      }
      

      if(my_m->conflicts>=my_m->next_restart_conflict){	
	//restarting!!!
	
	cancelUntil(2);

	my_m->restart++;

	int restart = my_m->restart;
	my_m->restart_conflict_incr = get_luby(restart+1);
	my_m->next_restart_conflict = my_m->conflicts + (int)my_m->restart_conflict_incr;
	
	simplify_original_kb();
	
	break;
      }//endif meet restart criteria

     if(my_m->simplify_learned_kb && my_m->decision_level==1 && my_m->conflicts >= my_m->num_conflicts_for_next_simplify){
       simplify_kb();
     }

    int num_assigned = (my_m->assign_top - my_m->assign);

    if(my_m->cur_cdc_count >= my_m->max_learned_clauses + num_assigned ){
      reduce_kb();
    }

    /**************************************************************************
     *
     * End checking restarting criteria and simplifying/reducing/compacting KB
     *
     **************************************************************************/

    /*****************************************************
     * Jump directly to the label "pre_decision" if you want a clean way to bypass the call to select_next_var
     *****************************************************/

    my_lit literal;
    
    //the default case!!!
    my_var dec_var = select_next_var();
    
    if(dec_var==0){      
      result = 1;
             
      
      extern char print_result;
      if(print_result){
	print_solution();
      }
      
      cancelUntil(2);
      break;
    }//endif solution found      

    save_progress_decision:

    // 1/4/07 added the || !my_m->save_progress part
      if(my_m->saved[dec_var]==0  //don't ever use saved array when save_progress is off
	 || !my_m->save_progress
	 ){
	
	literal = nlit(dec_var);
      }else{

	literal = my_m->saved[dec_var];
      }//end if else use saved phase or not
      
    pre_decision:
      
      int res = decide_literal(literal);                
      
      while(res!=1){
	
	if(my_m->assertion_level==0){
	  result = 0;
	  break;
	}
	
	//this is the default case!!!
	cancelUntil((my_m->assertion_level)+1);
	
	//this point of the program is after backtracking
	//and before asserting cd literal
	
	//this is the default case!!!
	res = assert_cd_literal();
	
	//at this point of the program, we are done with the assertion (may have resulted in success or conflict)
      }//end while conflict
      
      /***********************
       *
       * at this point, the SAT state is
       * 1-consistent.
       *
       ***********************/      

    }//end while not time to restart or solution found
   
  }//end while not solution found

  return result;

}//end function sat aux

/*
 * The top-level solving function.
 * This function calls a preprocessing function (unit+pure literals)
 * then calls sat_aux which solves the instance recursively.
 *
 */
int sat()
{
  int n = my_m->vc;
  
  int seed = 40973761;
  srand(seed);
    
  double* score = my_m->score;
  int cur_lit_count = my_m->cur_lit_count;

  //put every variable in the heap
  for(int i=1; i<=n; i++){
    insert(my_m->var_order_heap,i);
  }//end for i
  
  
  if(my_m->cc<=0){
    //trivial case
    return 1;
  }
  
  //Modified to handle pure literals(ADC 7/8/04)
  int status = 1;

  //assume no_preprocess_pure
  status = process_unit_literal_queue();
  
  simplify_original_kb();
  my_m->max_learned_clauses = my_m->cur_cc/3;
    
  //this is for progress printing
  start_time = clock();

  if(status==0){
    //UNSAT found in preprocessing
    //do nothing here.
    //let the code below handle the result
    //no need to call sat_aux
  }else{
    status = sat_aux();
  }
  
  //retract assertions (unit clauses) if any
  undo_assert_unit_clauses();

  return status;
}

/* Commented out for SAT07 submission
void printProgressHeader()
{

  rprintf("+----+-----------------+------------------+----------------------------------+---------------------------+---------+---------------------+\n");
  rprintf("| %-2s | %-15s | %-16s | %-32s | %-25s | %-7s | %-19s |\n","Re","Conflicts","Original","Learned","Decisions"," Time","KB");
  rprintf("| %2s | %7s %7s | %7s %8s | %7s %7s %8s %7s | %7s %10s %6s | %7s | %4s %4s %4s %4s |\n","st","Max","Actual","Clauses","Literals","Max","Clauses","Literals","LPC","Total","Per Sec","C/D","","Red.","Sim.","Cmp.","Sub.");
  rprintf("+----+-----------------+------------------+----------------------------------+---------------------------+---------+---------------------+\n");
}

void printProgressFooter()
{  
  rprintf("+----+-----------------+------------------+----------------------------------+---------------------------+---------+---------------------+\n");
}

extern int num_remove_attempts;

void printProgress()
{
  
  my_manager* m = my_m;

  end_time = clock();  
  double time_used = (end_time - start_time)/(double)CLOCKS_PER_SEC;
  int cur_decisions = m->decisions - previous_decision_count;
  int cur_conflicts = m->conflicts - previous_conflicts;

  int cur_reduce_kb = 0;

  cur_reduce_kb = m->db_reduction_count - previous_reduce_kb_count;

  int cur_simplify_kb = 0;
  cur_simplify_kb = m->db_simplification_count - previous_simplify_kb_count;

  int cur_compact_kb = 0;

  previous_decision_count = m->decisions;
  previous_conflicts = m->conflicts;

  previous_reduce_kb_count = m->db_reduction_count;

  previous_simplify_kb_count = m->db_simplification_count;

  int unit_clause_learned = my_m->decision_lit[2]-my_m->decision_lit[1];
  unit_clause_learned -= base_unit_clause_learned;

  start_time = end_time;
  
  int learned_clauses_removed = 0;
  int cur_subsume_kb = 0;

  rprintf("| %2d | %7d %7d | %7d %8d | %7d %7d %8d %7.1f | %7d %10.2f %6.3f | %7.3f | %4d %4d %4d %4d |(+ %d,- %d)\n",
	  m->restart,
	  m->next_restart_conflict,
	  m->conflicts,
	  m->cur_cc,
	  m->cur_lit_count,
	  (int)m->max_learned_clauses,
	  m->cur_cdc_count,
	  m->cur_cdl_count,
	  ((double)m->cur_cdl_count/m->cur_cdc_count),
	  cur_decisions,
	  (double)cur_decisions/time_used,
	  //(double)cur_implications/time_used,
	  (double)cur_conflicts/cur_decisions,
	  time_used,
	  cur_reduce_kb,
	  cur_simplify_kb,
	  cur_compact_kb,
	  cur_subsume_kb,
	  unit_clause_learned,
	  learned_clauses_removed
	  );
  
}
*/
