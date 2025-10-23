/****************************************************************************/
/*                    no preprocess for updating decreasing vars            */
/*                                                                          */
/*                  Author: Chu Min LI (cli@laria.u-picardie.fr)            */
/*                  Copyright LaRIA, Universite de Picardie Jules Verne     */
/*                  Sept. 2004                                              */
/****************************************************************************/

/* Based on newg2wsat3, integrating adapt noise technique in adaptnovelty
 */

/* Based on adaptg2wsat3, change watch length and adjust
 */

/* Based on adaptg2wsat16at5, use walk strategy of newg2wsat3bis1.c
 */

/* Based on adaptg2wsat16at5_3bis1,
when there are several promising decreasing variables, choose the least
recently flipped one
 */

/* Based on adaptg2wsat16at5_3bis2,
when there are several promising decreasing variables, choose the earliest
one
*/

/* Based on adaptg2wsat16at5_3bis3. When diversifying,
randomly choosing a variable in a clause, excluding the least recently
and the second least recently flipped variables
*/

/* Based on adaptg2wsat16at5_3bis4. When diversifying,
randomly choose a variable from the unsat clause excluding
the most recently flipped one
*/

/* Based on adaptg2wsat16at5_3bis6, clean useless code and arrays
use UP and lookahead to simplify the input formula before local search
 */

/* Based on adaptg2wsat16at5_3bis10, for 2009 SAT competition
when diversify for 3-SAT, do not adapt noises
 */

/* Based on TNM, winner of sat2009 competition, calculating variable
weight using integer weight
 */

/* Based on ag2, use clause weight instead of variable weight
 */

/* Based on ag2clw, use uneven case only when the weight of the selected
unsat clause is large
*/ 

/* Based on ag2clw1, adjust coefficient_prm to make fewer uneven steps
 */

/* Based on ag2clw1bis, use NB_CLAUSE instead of NB_EMPTY to
compute the average clause weight
*/

/* Based on ag2clw1bis2, initialize clause_weight to 1 instead of 0.
The initial average clause weight is 1 consequently
*/

/* Based on ag2clw1bis3, in uneven case and the best variable is the most
recently falsifying variable, random walk with probability NOISE1/20
 */

/* Based on ag2clw1bis4, decrease prm until 8 instead of 10
 */

/* Based on ag2clw1bis5, random walk in even case when the best variable 
is the most recently falsifying variable
*/

/* Based on ag2clw1bis6, in uneven case and the best variable is the most
recently falsifying variable, random walk with probability NOISE1/10,
contrarily to ag2clw1bis4 which makes random walk with probility NOISE1/20
in uneven cases
*/

/* Based on ag2clw1bis6, in uneven case and the best variable is the most
recently falsifying variable, random walk with probability NOISE1/10,
contrarily to ag2clw1bis4 which makes random walk with probility NOISE1/20
in uneven cases. Random walk in even case with probability variable depending
on most_recent_count[random_clause_unsat] (NOISE/20,NOISE/15,NOISE/10)
*/

/* Based on ag2clw1bis6, Random walk in even case with probability variable depending
on most_recent_count[random_clause_unsat] (NOISE/15,NOISE/10)
*/

/* Based on ag2clw1bis9, diversification by novelty for uneven promising 
decreasing variable
 */

/* Based on ag2clw1bis16, when clause and var uneven, diversify
 */

/* Based on ag2clw1bis18, when the unsat clause is the most falsified,
increase diversification probability when uneven clause and var
 */

/* Based on ag2clw1bis20, introduce sat_var array and used in 
uneven case
 */

/* Based on ag2clw1bis20, distinguish sat_time for each clause
by each variable in get_var...(even case and uneven case)
*/

/* Based on ag2clw1bis34, remove uneven case
 */

/* Based on ag2clw1bis34n, for competition
 */

#include <sys/resource.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <sys/times.h>
#include <sys/types.h>
#include <limits.h>

typedef signed int my_type;
typedef unsigned int my_unsigned_type;

#define WORD_LENGTH 100 
#define TRUE 1
#define FALSE 0
#define NONE -1
#define SATISFIABLE 2
#define walk_satisfiable() (MY_CLAUSE_STACK_fill_pointer == 0)

#define WEIGTH 4
#define T 10

/* the tables of variables and clauses are statically allocated. Modify the 
   parameters tab_variable_size and tab_clause_size before compilation if 
   necessary */
#define tab_variable_size  2000000
#define tab_clause_size 10000000
#define tab_unitclause_size \
 ((tab_clause_size/4<2000) ? 2000 : tab_clause_size/4)
#define my_tab_variable_size \
 ((tab_variable_size/2<1000) ? 1000 : tab_variable_size/2)
#define my_tab_clause_size \
 ((tab_clause_size/2<2000) ? 2000 : tab_clause_size/2)
#define my_tab_unitclause_size \
 ((tab_unitclause_size/2<1000) ? 1000 : tab_unitclause_size/2)
#define tab_literal_size 2*tab_variable_size
#define double_tab_clause_size 2*tab_clause_size
#define positive(literal) literal<NB_VAR
#define negative(literal) literal>=NB_VAR
#define get_var_from_lit(negative_literal) negative_literal-NB_VAR
#define RESOLVANT_LENGTH 3
#define RESOLVANT_SEARCH_THRESHOLD 5000
#define complement(lit1, lit2) \
 ((lit1<lit2) ? lit2-lit1 == NB_VAR : lit1-lit2 == NB_VAR)

#define inverse_signe(signe) \
 (signe == POSITIVE) ? NEGATIVE : POSITIVE
#define unsat(val) (val==0)?"UNS":"SAT"
#define pop(stack) stack[--stack ## _fill_pointer]
#define push(item, stack) stack[stack ## _fill_pointer++] = item
#define satisfiable() CLAUSE_STACK_fill_pointer == NB_CLAUSE

#define NEGATIVE 0
#define POSITIVE 1
#define PASSIVE 0
#define ACTIVE 1

int *neg_in[tab_variable_size];
int *pos_in[tab_variable_size];
int neg_nb[tab_variable_size];
int pos_nb[tab_variable_size];
my_type var_current_value[tab_variable_size];
my_type var_rest_value[tab_variable_size];
my_type var_state[tab_variable_size];

int *sat[tab_clause_size];
int *var_sign[tab_clause_size];
my_type clause_state[tab_clause_size];
my_type clause_length[tab_clause_size];
my_type static_clause_length[tab_clause_size];

int VARIABLE_STACK_fill_pointer = 0;
int CLAUSE_STACK_fill_pointer = 0;
int UNITCLAUSE_STACK_fill_pointer = 0;
int REDUCEDCLAUSE_STACK_fill_pointer = 0;

int VARIABLE_STACK[tab_variable_size];
int CLAUSE_STACK[tab_clause_size];
int UNITCLAUSE_STACK[tab_unitclause_size];
int REDUCEDCLAUSE_STACK[tab_clause_size];

int NB_VAR;
int NB_ACTIVE_VAR;
int NB_CLAUSE;
int INIT_NB_CLAUSE;
my_type R = 3;
int TROISSAT=TRUE;

long NB_UNIT=1, NB_MONO=0, NB_BRANCHE=0, NB_BACK = 0;

unsigned int SEED;
int SEED_FLAG=FALSE, BUILD_FLAG=TRUE;
char saved_input_file[WORD_LENGTH];
char *INPUT_FILE;

unsigned long IMPLIED_LIT_FLAG=0;
int IMPLIED_LIT_STACK_fill_pointer=0;
int IMPLIED_LIT_STACK[tab_variable_size];
unsigned long LIT_IMPLIED[tab_variable_size]={0};          

#include "inputbis56.c"

int verify_solution() {
  int i, var, *vars_signs, clause_truth,cpt;

  for (i=0; i<NB_CLAUSE; i++) {
    clause_truth = FALSE;
    vars_signs = var_sign[i];
    for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2))
      if (*(vars_signs+1) == var_current_value[var] ) {
	clause_truth = TRUE;
	break;
      }
    if (clause_truth == FALSE) return FALSE;
  }
  return TRUE;
}

my_type build(int build_flag, char* input_file) {
  if (build_flag==TRUE)
    return build_simple_sat_instance(input_file);
  else return build_simple_sat_instance(input_file);
}

#define DECREASING 1
#define INCREASING 2
#define PLATEAU 0
int var_count[tab_variable_size]={1};
int neibor_stack[tab_variable_size];
int neibor_stack_fill_pointer=0;
int *neibor[tab_variable_size];
int score[tab_variable_size];
int tmp_score[tab_variable_size];
int decreasing_vars_stack[tab_variable_size];
int decreasing_vars_stack_fill_pointer=0;
int MY_CLAUSE_STACK_fill_pointer=0;
int MY_CLAUSE_STACK[tab_clause_size];
int nb_lit_true[tab_clause_size];
int clause_truth[tab_clause_size];
int dommage_if_flip[tab_variable_size];
int zerodommage[tab_variable_size];
int zerodommage_vars_stack[tab_variable_size];
int zerodommage_vars_stack_fill_pointer=0;
int flip_time[tab_variable_size];
//int previous_flip_time[tab_variable_size];
int *sat_time[tab_clause_size];
int sat_var[tab_clause_size];
//int sat_count[tab_clause_size];
//int walk_time[tab_variable_size];
int MAXTRIES=10000;
//int MAXSTEPS=10000000;
int MAXSTEPS=2000000000;
int NOISE=50;
int LNOISE=5;
int saved_var_current_value[tab_variable_size];
int index_in_MY_CLAUSE_STACK[tab_clause_size];
//int most_recent[tab_clause_size];
//int most_recent_count[tab_clause_size];
int MAX_COUNTER;
int counter[tab_variable_size];
//int clause_weight[tab_clause_size];
// int MaxClauseWeight, TotalClauseWeight, NB_EMPTY;
int coefficient_for_prm=10;

void clause_value() {
  int clause, var, *vars_signs, nb_true;
   
  MY_CLAUSE_STACK_fill_pointer=0;  
  for (clause=0; clause<NB_CLAUSE; clause++) {
    nb_true=0;
    vars_signs = var_sign[clause];
    for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2))
      if (var_current_value[var]==*(vars_signs+1))  
	nb_true++;        
    nb_lit_true[clause]=nb_true;
    if (nb_true ==0 ) {
      clause_truth[clause]=FALSE;
      index_in_MY_CLAUSE_STACK[clause]=MY_CLAUSE_STACK_fill_pointer;
      push(clause, MY_CLAUSE_STACK);
    }
    else
      clause_truth[clause]=TRUE;    
  } 
}

void pass(int the_var, int *clauses) {
  int clause, var, *vars_signs;
  for(clause=*clauses; clause!=NONE; clause=*(++clauses)) {
    if (clause_state[clause] == ACTIVE) {
      vars_signs = var_sign[clause];
      for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2)) {
	if (var_state[var]==ACTIVE && var != the_var) {
	  if (var_count[var]==1) push(var, neibor_stack);
	  var_count[var]+=2*(clause_length[clause]-1);
	}
      }
    }
  }
}

void preprocess() {
  int var, neibor_var, i, **vector_relations, *vector, *relations, clause;

  for(clause=0; clause<NB_CLAUSE; clause++) {
    sat_time[clause]=malloc((clause_length[clause]+1)*sizeof(int));
    for(i=0; i<clause_length[clause]; i++)
      sat_time[clause][i]=0;
    sat_time[clause][clause_length[clause]]=NONE;
  }
  for (var=0; var<NB_VAR; var++) {
    var_count[var]=1;
  }

  for (var=0; var<NB_VAR; var++) {
    for(i=0; i<neibor_stack_fill_pointer; i++) var_count[neibor_stack[i]]=1;
    neibor_stack_fill_pointer=0;
    pass(var, neg_in[var]);
    pass(var, pos_in[var]);
    vector=(int *)malloc((neibor_stack_fill_pointer+1)*sizeof(int));
    for(i=0; i<neibor_stack_fill_pointer; i++) {
      neibor_var=neibor_stack[i];
      vector[i]=neibor_var;
    }
    vector[i]=NONE;
    neibor[var]=vector;
  }
}

int random_integer(int max)
{
  unsigned long int RAND;
  RAND=rand();
  return RAND % max;
}

//-------------------------------------------------------------------------

//Modifaction du germe du générateur aléatoire

struct timeval tv;
struct timezone tzp;

unsigned int *SEED_STACK;
int SEED_STACK_fill_pointer=0;
/*
int seedTried(unsigned int seed) {
  int i;

  for(i=0; i<SEED_STACK_fill_pointer; i++) {
    if (seed == SEED_STACK[i])
      return TRUE;
  }
  return FALSE;
}

void modify_seed() {
  int seed;
  if (SEED_FLAG==TRUE) {
    do {
      SEED=random_integer(2000000000);
    } while (seedTried(SEED)==TRUE || SEED ==0);
    printf("%u\n", SEED);
    push(SEED, SEED_STACK);
    srand(SEED); 
  }
  else {
    gettimeofday(&tv,&tzp);
    seed = (( tv.tv_sec & 0177 ) * 1000000) + tv.tv_usec;
    srand(seed);
  }
}
*/
void modify_seed() {
  int seed;
  if (SEED_FLAG==TRUE) {
    srand(SEED); SEED=SEED+17;
    if (SEED==0) SEED=17;
  }
  else {
    gettimeofday(&tv,&tzp);
    seed = (( tv.tv_sec & 0177 ) * 1000000) + tv.tv_usec;
    srand(seed);
  }
}

int get_gradient(int var, int *clauses) {
  int clause, var1, *vars_signs, gradient=0, clause_gradient=1;
  for(clause=*clauses; clause!=NONE; clause=*(++clauses)) {
    if (clause_state[clause] == ACTIVE) {
      vars_signs = var_sign[clause];  clause_gradient=1;
      for(var1=*vars_signs; var1!=NONE; var1=*(vars_signs+=2)) {
	if ((var_state[var1]==ACTIVE) && (var1!=var)) {
	  if (var_current_value[var1]==*(vars_signs+1)) {
	    clause_gradient=0; break;
	  }
	}
      }
      gradient+=clause_gradient;
    }
  }
  return gradient;
}

inline int decreasing_var(int var) {
  return (score[var]>0);
}

int initialize() {
  int var, gradient, neg_gradient, pos_gradient, clause, *sattimes;

  decreasing_vars_stack_fill_pointer=0;
  zerodommage_vars_stack_fill_pointer=0;
  for (var=0; var<NB_VAR; var++) {
    tmp_score[var]=0; counter[var]=0;
    if (var_state[var]==ACTIVE) {
      neg_gradient=get_gradient(var, neg_in[var]);
      pos_gradient=get_gradient(var, pos_in[var]);
      if (var_current_value[var]==TRUE)
	score[var]=neg_gradient-pos_gradient;
      else score[var]=pos_gradient-neg_gradient;
      if (var_current_value[var]==TRUE)
	dommage_if_flip[var]=pos_gradient;
      else dommage_if_flip[var]=neg_gradient;
      if ((dommage_if_flip[var]==0)  && ( score[var] != 0)) {
	push(var, zerodommage_vars_stack);
	zerodommage[var]=TRUE;
      }
      else zerodommage[var]=FALSE;
      if (score[var]>0) {
	push(var, decreasing_vars_stack);
      }
    }
  }
  MAX_COUNTER=0;// MaxClauseWeight=1; TotalClauseWeight=NB_CLAUSE;
  // NB_EMPTY=0;
  for(clause=0; clause<NB_CLAUSE; clause++) {
    /*
    most_recent[clause]=NONE;
    most_recent_count[clause]=0;
    clause_weight[clause]=1;
    */
    // sat_count[clause]=0;
    sat_var[clause]=NONE;
    sattimes=sat_time[clause];
    for(; *sattimes!=NONE; sattimes++)
      *sattimes=0;
  }
  return TRUE;
}

int diversify(int random_clause_unsat) {
  int flip_index, i, var, recent_var, *vars_signs;

  flip_index=random_integer(clause_length[random_clause_unsat]);
  return var_sign[random_clause_unsat][2*flip_index];

  if (clause_length[random_clause_unsat]<=2) {
    flip_index=random_integer(clause_length[random_clause_unsat]);
    return var_sign[random_clause_unsat][2*flip_index];
  }
  else {
    vars_signs = var_sign[random_clause_unsat];
    recent_var=*vars_signs;
    for(var=*(vars_signs+=2); var!=NONE; var=*(vars_signs+=2)) {
      if (flip_time[var]>flip_time[recent_var]) {
	recent_var=var;
      }
    }
    flip_index=random_integer(clause_length[random_clause_unsat]-1);
    vars_signs = var_sign[random_clause_unsat]; i=0; 
    for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2)) {
      if (var != recent_var) {
	if (i==flip_index)
	  return var;
	else i++;
      }
    }
  }
}

int get_var_to_flip_in_clause(int random_clause_unsat) {
  int var, best_var=NONE, second_best_var, nb, max_nb, pos_gradient, second_max,
    neg_gradient, real_nb, flip=-1, flip_index, i, var_to_flip, old=MAXSTEPS, old_var,
    LNOISE1, besttime, second_besttime, *sattimes;
  register int *vars_signs;

  if (random_integer(100)<LNOISE) 
    return diversify(random_clause_unsat);

  vars_signs = var_sign[random_clause_unsat]; max_nb=-NB_CLAUSE;
  sattimes=sat_time[random_clause_unsat]; besttime=MAXSTEPS;
  for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2), sattimes++) {
    nb=score[var];
    if ((nb>max_nb) || ((nb==max_nb) && (*sattimes<besttime))) {
      second_best_var=best_var; second_max=max_nb; best_var=var; max_nb=nb; 
      second_besttime=besttime; besttime=*sattimes;
    }
    else if ((nb>second_max) || 
	     ((nb==second_max) && (*sattimes<second_besttime))) {
      second_max=nb; second_best_var=var; second_besttime=*sattimes;
    }
    //   if (previous_flip_time[var]>flip) 
    // flip=previous_flip_time[var];
  }
  if (best_var==sat_var[random_clause_unsat]) {
    if (random_integer(100)<NOISE/10)
      return diversifyForUneven(random_clause_unsat, best_var);
    else if (random_integer(100)<NOISE) 
      return second_best_var; 
    else return best_var;
  }
  else return best_var;
}

void satisfy_clauses(int var, int *clauses, int step) {
  int clause,  neibor_var, *vars_signs, dommage=0, last_unsatisfied_clause,
    index, *sattimes;
  for (clause=*clauses;clause!=NONE;clause=*(++clauses)) {
    vars_signs=var_sign[clause];
    switch(nb_lit_true[clause]) {
    case 0: clause_truth[clause]=TRUE; nb_lit_true[clause]++;
      sat_var[clause]=var;
      /*
      if (sat_var[clause]==var) 
	sat_count[clause]++;
      else {    
	sat_var[clause]=var;
	sat_count[clause]=1;
      }
      */
      //  dommage++;
      last_unsatisfied_clause=pop(MY_CLAUSE_STACK);
      index=index_in_MY_CLAUSE_STACK[clause];
      MY_CLAUSE_STACK[index]=last_unsatisfied_clause;
      index_in_MY_CLAUSE_STACK[last_unsatisfied_clause]=index;
      sattimes=sat_time[clause];
      for(neibor_var=*vars_signs; neibor_var!=NONE; 
	  neibor_var=*(vars_signs+=2), sattimes++) {
	if (neibor_var!=var) {
	    tmp_score[neibor_var]--;
	}
	else
	  *sattimes=step;
      }
      break;
    case 1: nb_lit_true[clause]++;
      for(neibor_var=*vars_signs; neibor_var!=NONE; 
	  neibor_var=*(vars_signs+=2)) {
	if ((neibor_var!=var) && 
	    (var_current_value[neibor_var]==*(vars_signs+1))) {
	  // dommage_if_flip[neibor_var]--;
	  tmp_score[neibor_var]++;
	  break;
	}
      }
      break;
    default:  nb_lit_true[clause]++;
    }
  }
  //  if (dommage==0) 
  //  printf("c c'est curieux...");
}

void unsatisfy_clauses(int var, int *clauses) {
  int clause, neibor_var, *vars_signs;
  for (clause=*clauses;clause!=NONE;clause=*(++clauses)) {
    vars_signs=var_sign[clause];
    switch(nb_lit_true[clause]) {
    case 1:  clause_truth[clause]=FALSE; nb_lit_true[clause]--;
      /*
      if (most_recent[clause]==var) 
	most_recent_count[clause]++;
      else {
	most_recent[clause]=var;
	most_recent_count[clause]=1;
      }
      clause_weight[clause]++; TotalClauseWeight++;
      if (clause_weight[clause]==1)
	NB_EMPTY++;
      if (MaxClauseWeight<clause_weight[clause])
	MaxClauseWeight=clause_weight[clause];
      */
      index_in_MY_CLAUSE_STACK[clause]=MY_CLAUSE_STACK_fill_pointer;
      push(clause,MY_CLAUSE_STACK);
      for(neibor_var=*vars_signs; neibor_var!=NONE; 
	  neibor_var=*(vars_signs+=2)) {
	if (neibor_var!=var) {
	  tmp_score[neibor_var]++;
	}
      }
      break;
    case 2: nb_lit_true[clause]--;
      for(neibor_var=*vars_signs; neibor_var!=NONE; 
	  neibor_var=*(vars_signs+=2)) {
	if ((neibor_var!=var) && 
	    (var_current_value[neibor_var]==*(vars_signs+1))) {
	  tmp_score[neibor_var]--;
	  // dommage_if_flip[neibor_var]++;
	  break;
	}
      }
      break;
    default:  nb_lit_true[clause]--;
    }
  }
}

void check_implied_clauses(int var, int value, int nb_flip) { 
  int *neibors, neibor_var;
  if (value==TRUE) {
    satisfy_clauses(var, pos_in[var], nb_flip);
    unsatisfy_clauses(var, neg_in[var]);
  }
  else {
    satisfy_clauses(var, neg_in[var], nb_flip);
    unsatisfy_clauses(var, pos_in[var]);
  }
  neibors=neibor[var];
  for(neibor_var=*neibors; neibor_var!=NONE; neibor_var=*(++neibors)) {
    if ((score[neibor_var]<=0) && (score[neibor_var]+tmp_score[neibor_var]>0)) {
      push(neibor_var, decreasing_vars_stack);
    }
    score[neibor_var]+=tmp_score[neibor_var];
    tmp_score[neibor_var]=0;
  }
}

/*
int simple_eliminate_increasing_vars() {
  int i, first=NONE, put_in, var, current=0, nb=0, chosen_var=NONE, flip=MAXSTEPS, enter;
  for (i=0; i<decreasing_vars_stack_fill_pointer; i++) {
    var=decreasing_vars_stack[i];
    if (score[var]<=0) {
      first=i;
      break;
    }
    else {
      if (flip>flip_time[var]) {
	nb=score[var]; flip=flip_time[var];
	chosen_var=var; 
      }
      else if (flip==flip_time[var]) {
	if  (nb<score[var]) {
	  chosen_var=var; nb=score[var];
	}
	else if (nb==score[var]) {
	  if (random_integer(2)==0)
	    chosen_var=var;
	}
      }
    }
  }
  if (first !=NONE) {
    put_in=first;
    for (i=first+1; i<decreasing_vars_stack_fill_pointer; i++) {
      var=decreasing_vars_stack[i];
      if (score[var]>0) {
	decreasing_vars_stack[put_in++]=var;
	if (flip>flip_time[var]) {
	  nb=score[var]; flip=flip_time[var];
	  chosen_var=var; 
	}
	else if (flip==flip_time[var]) {
	  if  (nb<score[var]) {
	    chosen_var=var; nb=score[var];
	  }
	  else if (nb==score[var]) {
	    if (random_integer(2)==0)
	      chosen_var=var;
	  }
	}
      }
    }
    decreasing_vars_stack_fill_pointer=put_in;
  }
  if (decreasing_vars_stack_fill_pointer>0) 
    return decreasing_vars_stack[0];
  else return NONE;
}
  
int update_gradient_and_choose_flip_var(int var) {
  int neibor_var, chosen1, sign, index,
    other_var_in_same_clause, clause_gradient, gradient, chosen_var=NONE;

  score[var]=0-score[var];
  chosen_var=simple_eliminate_increasing_vars();
  return chosen_var;
}
*/

int simple_eliminate_increasing_vars() {
  int i, first=NONE, put_in, var;
  for (i=0; i<decreasing_vars_stack_fill_pointer; i++) {
    var=decreasing_vars_stack[i];
    if (score[var]<=0) {
      first=i;
      break;
    }
  }
  if (first !=NONE) {
    put_in=first;
    for (i=first+1; i<decreasing_vars_stack_fill_pointer; i++) {
      var=decreasing_vars_stack[i];
      if (score[var]>0) {
	decreasing_vars_stack[put_in++]=var;
      }
    }
    decreasing_vars_stack_fill_pointer=put_in;
  }
  return put_in;
}


int choose_best_decreasing_var() {
  int var, chosen_var, i, flip;
  var=decreasing_vars_stack[0]; 
  //the_counter=counter[var]; 
  flip=flip_time[var]; 
  chosen_var=var;
  for (i=1; i<decreasing_vars_stack_fill_pointer; i++) {
     var=decreasing_vars_stack[i];
     if (flip_time[var]<flip) {
          flip=flip_time[var]; chosen_var=var;	
     }
  }
  return chosen_var;
}

int update_gradient_and_choose_flip_var(int var) {
  int neibor_var, chosen1, sign, index, for_swap, 
    other_var_in_same_clause, clause_gradient, gradient, chosen_var=NONE;

  score[var]=0-score[var];
  simple_eliminate_increasing_vars();
  if (decreasing_vars_stack_fill_pointer>1)
    return choose_best_decreasing_var();
  else if (decreasing_vars_stack_fill_pointer==1)
    return decreasing_vars_stack[0];
  return chosen_var;
}

int choose_var_by_random_walk() {
  int  random_unsatisfied_clause,  var_to_flip;
  random_unsatisfied_clause=random_integer(MY_CLAUSE_STACK_fill_pointer);
  random_unsatisfied_clause=MY_CLAUSE_STACK[random_unsatisfied_clause];
  var_to_flip=get_var_to_flip_in_clause(random_unsatisfied_clause);
  return  var_to_flip;
}

void save_assignment() {
  int i;
  for(i=0; i<NB_VAR; i++) 
    saved_var_current_value[i]=var_current_value[i];
}

// #include "adaptnoisebis1at5.c"

int invPhi=10;
#define invTheta 5

int lastAdaptFlip, lastBest, AdaptLength, NB_ADAPT, NB_BETTER;
int lastEvenSteps, lastUnEvenSteps;
int nb_even, nb_uneven, uneven_var_threshold=9, nb_uneven_var;

int initNoise() {
  lastAdaptFlip=0; lastEvenSteps=0; lastUnEvenSteps=0;
  lastBest = MY_CLAUSE_STACK_fill_pointer;
  NOISE=0; LNOISE=0; NB_BETTER=0; NB_ADAPT=0;
  AdaptLength=NB_CLAUSE / invTheta;
  coefficient_for_prm=10;
}

void adaptNoveltyNoise(int flip) {
  int deltaEven, deltaUnEven;

  if ((flip - lastAdaptFlip) > AdaptLength) {
    deltaEven=nb_even-lastEvenSteps; 
    deltaUnEven=nb_uneven-lastUnEvenSteps;
    if (deltaUnEven>deltaEven)
      // coefficient_for_prm+=(int) ((20-coefficient_for_prm)/6);
      coefficient_for_prm++;
    else if (coefficient_for_prm>8 && deltaUnEven<deltaEven/20)
      // coefficient_for_prm-=(int) (coefficient_for_prm/9);
      coefficient_for_prm--;
    NOISE += (int) ((100 - NOISE) / invPhi);
    LNOISE= (int) NOISE/10;
    lastAdaptFlip = flip;      
    // NB_BETTER=0;
    // if (MY_CLAUSE_STACK_fill_pointer < lastBest)
      lastBest = MY_CLAUSE_STACK_fill_pointer;
      lastEvenSteps=nb_even; lastUnEvenSteps=nb_uneven;
  } 
  else if (MY_CLAUSE_STACK_fill_pointer < lastBest) {
    //  NB_BETTER++;
    //  if (NB_BETTER>1) {
    NOISE -= (int) (NOISE /(2*invPhi));
    LNOISE= (int) NOISE/10;
    lastAdaptFlip = flip;
    lastBest = MY_CLAUSE_STACK_fill_pointer;
    lastEvenSteps=nb_even; lastUnEvenSteps=nb_uneven;
      //   NB_BETTER=0;
      // }
  }
}

int NBSOL=tab_variable_size;

int diversifyForUneven(int random_clause_unsat, int best_var) {
  int flip_index, i, var, *vars_signs;

  flip_index=random_integer(clause_length[random_clause_unsat]-1);
  vars_signs = var_sign[random_clause_unsat]; i=0; 
  for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2)) {
    if (var != best_var) {
      if (i==flip_index)
	return var;
      else i++;
    }
  }
}
/*
int get_var_to_flip_in_clause_as_new(int random_clause_unsat, int total_steps) {
  int var, best_var, second_best_var, nb, max_nb, pos_gradient, second_max,
    neg_gradient, real_nb, flip=-1, flip_index, var_to_flip, old=MAXSTEPS,
    old_var, chosen_var, NOISE1, randomInt, besttime, second_besttime, *sattimes;
  register int *vars_signs;

  vars_signs = var_sign[random_clause_unsat]; max_nb=-NB_CLAUSE; 
  besttime=MAXSTEPS; sattimes=sat_time[random_clause_unsat];
  for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2), sattimes++) {
    nb=score[var];
    if ((nb>max_nb) || ((nb==max_nb) && (*sattimes<besttime))) {
      second_best_var=best_var; second_max=max_nb; best_var=var; max_nb=nb;
      second_besttime=besttime; besttime=*sattimes;
    }
    else if ((nb>second_max) || 
	     ((nb==second_max) && (*sattimes<second_besttime))) {
      second_max=nb; second_best_var=var; second_besttime=*sattimes;
    }
    if (flip_time[var]<old) {
      old=flip_time[var];
      old_var=var;
    }
  }
  if (random_integer(100)<LNOISE)
    chosen_var=old_var;
  else {
    // if (best_var==most_recent[random_clause_unsat]) {
    //  switch(most_recent_count[random_clause_unsat]) {
    if (best_var==sat_var[random_clause_unsat]) {
      switch(sat_count[random_clause_unsat]) {
      case 1: NOISE1=20; break;
      case 2: NOISE1=50; break;
      case 3: NOISE1=65; break;
      case 4: NOISE1=72; break;
      case 5: NOISE1=78; break;
      case 6: NOISE1=86; break;
      case 7: NOISE1=90; break;
      case 8: NOISE1=95; break;
      case 9: NOISE1=98; break;
      default: NOISE1=100; break;
      }
      if (random_integer(100)<NOISE1/20)
	chosen_var=diversifyForUneven(random_clause_unsat, best_var);
      else if (random_integer(100)<NOISE1) {
	chosen_var=second_best_var; 
      }
      else 
	chosen_var=best_var;
    }
    else
	chosen_var=best_var;
  }
  if (uneven_var(chosen_var, total_steps)) {
    randomInt=random_integer(100);
    if (randomInt<LNOISE || (MaxClauseWeight==clause_weight[random_clause_unsat] 
			      && randomInt<2*LNOISE))
	return old_var;
    else return chosen_var;
  }
  else 
    return chosen_var;
}

int choose_var_by_random_walk_as_new() {
  int  random_unsatisfied_clause,  var_to_flip;
  random_unsatisfied_clause=random_integer(MY_CLAUSE_STACK_fill_pointer);
  random_unsatisfied_clause=MY_CLAUSE_STACK[random_unsatisfied_clause];
  var_to_flip=get_var_to_flip_in_clause_as_new(random_unsatisfied_clause);
  return  var_to_flip;
}
*/

int my_choose_var_by_random_walk(int total_steps) {
  int  random_unsatisfied_clause,  var_to_flip;
  random_unsatisfied_clause=random_integer(MY_CLAUSE_STACK_fill_pointer);
  random_unsatisfied_clause=MY_CLAUSE_STACK[random_unsatisfied_clause];
  return get_var_to_flip_in_clause(random_unsatisfied_clause);
}

double ave_counter; 
void update_counter(int var_to_flip) {
  double the_old_counter, the_new_counter;
  counter[var_to_flip]++;
  // ave_counter=ave_counter+1.0/(double)NB_VAR;
  if (counter[var_to_flip]>MAX_COUNTER)
    MAX_COUNTER=counter[var_to_flip];
}

int uneven_var(int var, int total_steps) {
  return counter[var]>uneven_var_threshold*(total_steps/NB_VAR+1);
}

int search(char *input_file) {
  int i,j,k,SAT=FALSE,random_clause,var_to_flip=NONE, index, flag, saved,
    *min,  nbminima=0, nb_suc=0, walkperiod=FALSE, nbwalk=0, 
    nb, dist, verify=0, var; //init_seed;
  // double avgdepth, depth=0;
  // double avgdepths=0, total_min=0, total_last=0, total_nbwalk=0, total_nb_flip=0,
  //  total_sat_nb_even=0, total_sat_nb_uneven=0,  total_unsat_nb_even=0, 
  //  total_unsat_nb_uneven=0;
  // clock_t start, end;
  // FILE *fp_time;
  struct rusage starttime, endtime;
  // long begintime, endtime, mess;
  // struct tms *a_tms;
  // char result_table[100];

  getrusage(RUSAGE_SELF, &starttime );
  // start=clock(); init_seed=SEED;
  //  min=(int *)malloc(MAXTRIES*sizeof(int));
  preprocess();

  for (i=0;i<MAXTRIES;i++) {
    modify_seed(); nbwalk=0; SAT=FALSE; // depth=0;  
    nb_even=0; nb_uneven=0; // nb_uneven_var=0;
    // Generation de valeurs aleatoires pour le vecteur var_current_value
    for (k=0;k<NB_VAR;k++) {
      if (var_state[k]==ACTIVE) {
	var_current_value[k]=random_integer(2);
	flip_time[k]=0;
	// previous_flip_time[k]=0;
	//	walk_time[k]=0;
      }
    }
    initialize();
    clause_value();
    initNoise();
    if (zerodommage_vars_stack_fill_pointer>0) {
      index=random_integer(zerodommage_vars_stack_fill_pointer);
      var_to_flip=zerodommage_vars_stack[index];
    }
    else 
    if (decreasing_vars_stack_fill_pointer>0) {
      index=random_integer(decreasing_vars_stack_fill_pointer);
      var_to_flip=decreasing_vars_stack[index];
    }
    else var_to_flip=NONE;
    ave_counter=0;
    for (j=0;j<MAXSTEPS;j++) {
      if (walk_satisfiable()) {
	SAT=TRUE;
	break;
      }
      if (var_to_flip==NONE || 
	  (uneven_var(var_to_flip, j) && random_integer(100)<LNOISE)) {
	// when there is no non-tabu decreasing var
	nbwalk++;   
	var_to_flip=my_choose_var_by_random_walk(j);
      }
      var_current_value[var_to_flip]=1-var_current_value[var_to_flip]; 
      check_implied_clauses(var_to_flip, var_current_value[var_to_flip], j);
      // previous_flip_time[var_to_flip]=flip_time[var_to_flip];
      flip_time[var_to_flip]=j;
      update_counter(var_to_flip);
      adaptNoveltyNoise(j);
      var_to_flip=update_gradient_and_choose_flip_var(var_to_flip);
      // if ( MY_CLAUSE_STACK_fill_pointer<min[i])
      //	min[i]= MY_CLAUSE_STACK_fill_pointer;
      // depth+=MY_CLAUSE_STACK_fill_pointer;
    }
    fprintf(stdout, "c try=%d flip j=%d walk=%d seed %u\n", 
	    i+1, j, nbwalk, SEED);
    if (SAT==TRUE) {
      clause_value();
      if (walk_satisfiable()) {
	SAT=verify_sol_input(input_file);
	if (SAT==TRUE) {
	  fprintf(stdout, "c A solution is found\n");
	  break;
	}
	else {
	  fprintf(stdout, "c I'AM SORRY SOMETHING IS WRONG\n");
	  SAT=FALSE;
	}
      }
      else {
	fprintf(stdout, "c I'AM SORRY SOMETHING IS WRONG\n");
	SAT=FALSE;
      }
    }
  }
  if (SAT==TRUE) {
    fprintf(stdout, "s SATISFIABLE\n");
    fprintf(stdout, "v ");
    for (var=0; var<NB_VAR; var++) {
      if (var_current_value[var]==TRUE)
	fprintf(stdout, "%d ", var+1);
      else fprintf(stdout, "%d ", -(var+1));
    }
    fprintf(stdout, "\n");
    fprintf(stdout, "v 0 \n");
  }
  else {
    fprintf(stdout, "s UNKNOWN\n");
  }
  getrusage( RUSAGE_SELF, &endtime );
  fprintf(stdout, "c Done (mycputime is %d seconds)\n", 
	  (int) (endtime.ru_utime.tv_sec - starttime.ru_utime.tv_sec));
  if (SAT==TRUE) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

void scanone(int argc, char *argv[], int i, int *varptr) {
  if (i>=argc || sscanf(argv[i],"%i",varptr)!=1){
    fprintf(stderr, "Bad argument %s\n", i<argc ? argv[i] : argv[argc-1]);
    exit(-1);
  }
}

int HELP_FLAG=FALSE;

void parse_parameters(int argc,char *argv[]) {
  int i, temp, j;
  if (argc<2)
    HELP_FLAG=TRUE;
  else 
    for (i=1;i < argc;i++) {
      if (strcmp(argv[i],"-seed") == 0) {
	scanone(argc,argv,++i,&SEED);
	SEED_FLAG=TRUE; 
      }
      else if (strcmp(argv[i],"-cutoff") == 0)
	scanone(argc,argv,++i,&MAXSTEPS);
      else if (strcmp(argv[i],"-tries") == 0 || strcmp(argv[i],"-restart") == 0) 
	scanone(argc,argv,++i,&MAXTRIES);
      else if (strcmp(argv[i], "-s")==0)
	BUILD_FLAG=FALSE;
      else if (strcmp(argv[i], "-noise")==0)
	scanone(argc, argv, ++i,&NOISE);
      else if (strcmp(argv[i], "-lnoise")==0)
	scanone(argc, argv, ++i,&LNOISE);
      else if (strcmp(argv[i], "-dp")==0)
	scanone(argc, argv, ++i,&LNOISE);
      else if (strcmp(argv[i], "-help")==0)
	HELP_FLAG=TRUE;
      else if (strcmp(argv[i], "-nbsol")==0)
	scanone(argc, argv, ++i, &NBSOL);
      else if (strcmp(argv[i], "-coef")==0)
	scanone(argc, argv, ++i, &coefficient_for_prm);
      else   
	for (j=0; j<WORD_LENGTH; j++) {
	  INPUT_FILE=argv[i];
	  saved_input_file[j]=INPUT_FILE[j];
	}
    }
}

main(int argc, char *argv[]) {
  int i,  var; 

 if (argc==3) {
    SEED_FLAG=TRUE; 
    if (sscanf(argv[2],"%u",&SEED)!=1) {
      fprintf(stdout, "c Bad argument %s\n", argv[2]);
      fprintf(stdout, "s UNKNOWN\n");
      exit(0);
    }
  } 
 //  parse_parameters(argc,argv);
  INPUT_FILE=argv[1];

  switch (build(BUILD_FLAG, INPUT_FILE)) {
  case FALSE: fprintf(stdout, "c Input file error or too large formula\n"); 
    fprintf(stdout, "s UNKNOWN\n");
    exit(0);
    return FALSE;
  case SATISFIABLE:
    if (verify_sol_input(argv[1])==TRUE) {
      fprintf(stdout, "c Satisfied at top\n");
      fprintf(stdout, "s SATISFIABLE\n");
      fprintf(stdout, "v ");
      for (var=0; var<NB_VAR; var++) {
	if (var_current_value[var]==TRUE)
	  fprintf(stdout, "%d ", var+1);
	else fprintf(stdout, "%d ", -(var+1));
      }
      fprintf(stdout, "\n");
      fprintf(stdout, "v 0 \n");
      exit(10);
    }
    else {
      fprintf(stdout, "c problem at top\n");
      fprintf(stdout, "s UNKNOWN\n");
      exit(0);
    }
    return FALSE;
  case TRUE:
    VARIABLE_STACK_fill_pointer=0;
    CLAUSE_STACK_fill_pointer = 0;
    REDUCEDCLAUSE_STACK_fill_pointer = 0;
    //   srand(SEED);
    printf("c initial seed is %u\n", SEED);
    if (search(argv[1])==TRUE) {
      exit(10);
    }
    else 
      exit(0);
    break;
  case NONE: fprintf(stdout, "c A contradiction is found at top!\n");
    fprintf(stdout, "c UNSATISFIABLE\n");
    exit(0);
  }
  return TRUE;
}

