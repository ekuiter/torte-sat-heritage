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

/* Based on ag2clw1bis20, use previous_flip_time instead of flip_time
 */

/*Based on ag2clw1bis28, remove uneven case
 */

/*Based on ag2clw1bis28n, for competition
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

/* Based on inputbis.c de G2WSAT, en y ajoutant la partie propagation 
unitaire
*/

/* test if the new clause is redundant or subsompted by another */
#define OLD_CLAUSE_REDUNDANT -77
#define NEW_CLAUSE_REDUNDANT -7
#define FIXED 2;

int smaller_than(int lit1, int lit2) {
  return ((lit1<NB_VAR) ? lit1 : lit1-NB_VAR) < 
    ((lit2<NB_VAR) ? lit2 : lit2-NB_VAR);
}

my_type redundant(int *new_clause, int *old_clause) {
  int lit1, lit2, old_clause_diff=0, new_clause_diff=0;
    
  lit1=*old_clause; lit2=*new_clause;
  while ((lit1 != NONE) && (lit2 != NONE)) {
    if (smaller_than(lit1, lit2)) {
      lit1=*(++old_clause); old_clause_diff++;
    }
    else
      if (smaller_than(lit2, lit1)) {
	lit2=*(++new_clause); new_clause_diff++;
      }
      else
	if (complement(lit1, lit2)) {
	  return FALSE; /* old_clause_diff++; new_clause_diff++; j1++; j2++; */
	}
	else {
          lit1=*(++old_clause);  lit2=*(++new_clause);
	}
  }
  if ((lit1 == NONE) && (old_clause_diff == 0))
    /* la nouvelle clause est redondante ou subsumee */
    return NEW_CLAUSE_REDUNDANT;
  if ((lit2 == NONE) && (new_clause_diff == 0))
    /* la old clause est redondante ou subsumee */
    return OLD_CLAUSE_REDUNDANT;
  return FALSE;
}

void remove_passive_clauses() {
  int  clause, put_in, first=NONE;
  for (clause=0; clause<NB_CLAUSE; clause++) {
    if (clause_state[clause]==PASSIVE) {
      first=clause; break;
    }
  }
  if (first!=NONE) {
    put_in=first;
    for(clause=first+1; clause<NB_CLAUSE; clause++) {
      if (clause_state[clause]==ACTIVE) {
	sat[put_in]=sat[clause]; var_sign[put_in]=var_sign[clause];
	clause_state[put_in]=ACTIVE; 
	clause_length[put_in]=clause_length[clause];
	put_in++;
      }
    }
    NB_CLAUSE=put_in;
  }
}

void remove_passive_vars_in_clause(int clause) {
  int *vars_signs, *vars_signs1, var, var1, first=NONE;
  vars_signs=var_sign[clause];
  for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2)) {
    if (var_state[var]!=ACTIVE) {
      first=var; break;
    }
  }
  if (first!=NONE) {
    for(vars_signs1=vars_signs+2, var1=*vars_signs1; var1!=NONE; 
	var1=*(vars_signs1+=2)) {
      if (var_state[var1]==ACTIVE) {
	*vars_signs=var1; *(vars_signs+1) = *(vars_signs1+1);
	vars_signs+=2;
      }
    }
    *vars_signs=NONE;
  }
}

int clean_structure() {
  int clause, var, *vars_signs, length;
  remove_passive_clauses();
  if (NB_CLAUSE==0) 
    return SATISFIABLE;
  for (clause=0; clause<NB_CLAUSE; clause++) 
    remove_passive_vars_in_clause(clause);
  NB_ACTIVE_VAR=0;
  for (var=0; var<NB_VAR; var++) { 
    neg_nb[var] = 0;
    pos_nb[var] = 0;
    if (var_state[var]==ACTIVE) NB_ACTIVE_VAR++;
    //   else var_state[var]=FIXED;
  }
  for (clause=0; clause<NB_CLAUSE; clause++) {
    vars_signs=var_sign[clause]; length=0;
    for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2)) {
      length++;
      if (*(vars_signs+1)==POSITIVE) 
	pos_in[var][pos_nb[var]++]=clause;
      else  neg_in[var][neg_nb[var]++]=clause;
    }
    clause_length[clause]=length;
    static_clause_length[clause]=length;
  }
  // si une variable etait passive ou n'apparait dans aucune clause
  // neg_in[var][0] et pos_in[var][0] doit être NONE
  for (var=0; var<NB_VAR; var++) { 
    neg_in[var][neg_nb[var]]=NONE;
    pos_in[var][pos_nb[var]]=NONE;
  }
  return TRUE;
}

int unitclause_process();

int lire_clauses(FILE *fp_in) {
  int i, j, jj, ii, length, tautologie, lits[5000], lit, lit1;
  for (i=0; i<NB_CLAUSE; i++) {
    length=0; 
    fscanf(fp_in, "%d", &lits[length]);
    while (lits[length] != 0) {
      length++;
      if (fscanf(fp_in, "%d", &lits[length])==EOF) {
	fprintf(stdout, "c error in the input file\n");
	return FALSE;
      }
    }
    if (length!=3) 
      TROISSAT=FALSE;
    tautologie = FALSE;
    /* test if some literals are redundant and sort the clause */
    for (ii=0; ii<length-1; ii++) {
      lit = lits[ii];
      for (jj=ii+1; jj<length; jj++) {
	if (abs(lit)>abs(lits[jj])) {
	  lit1=lits[jj]; lits[jj]=lit; lit=lit1;
	}
	else
	  if (lit == lits[jj]) {
	    lits[jj] = lits[length-1]; 
	    jj--; length--; lits[length] = 0;
	    printf("c literal %d is redundant in clause %d \n", lit, i+1);
	  }
	  else
            if (abs(lit) == abs(lits[jj])) {
	      tautologie = TRUE; break;
            }
      }
      if (tautologie == TRUE) break;
      else lits[ii] = lit;
    }
    if (tautologie == FALSE) {
      sat[i]= (int *)malloc((length+1) * sizeof(int));
      for (j=0; j<length; j++) {
	if (lits[j] < 0) 
	  sat[i][j] = abs(lits[j]) - 1 + NB_VAR ;
	else 
	  sat[i][j] = lits[j]-1;
      }
      sat[i][length]=NONE;
      clause_length[i]=length;
      clause_state[i] = ACTIVE;
    }
    else { i--; NB_CLAUSE--;}
  }
  return TRUE;
}

void build_structure() {
  int i, j, var, *lits1, length, clause, *vars_signs, lit;
  for (i=0; i<NB_VAR; i++) { 
    neg_nb[i] = 0; pos_nb[i] = 0;
  }
  for (i=0; i<NB_CLAUSE; i++) {
    for(j=0; j<clause_length[i]; j++) {
      if (sat[i][j]>=NB_VAR) {
	var=sat[i][j]-NB_VAR; neg_nb[var]++;
      }
      else {
	var=sat[i][j]; pos_nb[var]++;
      }
    }
    if (sat[i][clause_length[i]] !=NONE)
      printf("c erreur ");
  }
  for(clause=0;clause<NB_CLAUSE;clause++) {
    length = clause_length[clause];
    var_sign[clause] = (int *)malloc((2*length+1)*sizeof(int));
    lits1 = sat[clause]; vars_signs = var_sign[clause];
    for(lit=*lits1; lit!=NONE; lit=*(++lits1),(vars_signs+=2)) {
      if (negative(lit)) {
	*(vars_signs+1)= NEGATIVE;
	*vars_signs = get_var_from_lit(lit);
      }
      else {
	*(vars_signs+1)=POSITIVE;
	*vars_signs = lit;
      }
    }
    *vars_signs = NONE;  
  }
  for (i=0; i<NB_VAR; i++) { 
    neg_in[i] = (int *)malloc((neg_nb[i]+pos_nb[i]+1) * sizeof(int));
    pos_in[i] = (int *)malloc((pos_nb[i]+neg_nb[i]+1) * sizeof(int));
    neg_in[i][neg_nb[i]]=NONE; pos_in[i][pos_nb[i]]=NONE;
    neg_nb[i] = 0; pos_nb[i] = 0;
    var_state[i] = ACTIVE;
  }   
  for (i=0; i<NB_CLAUSE; i++) {
    lits1 = sat[i];
    for(lit=*lits1; lit!=NONE; lit=*(++lits1)) {
      if (positive(lit)) 
	pos_in[lit][pos_nb[lit]++] = i;
      else
	neg_in[get_var_from_lit(lit)]
	  [neg_nb[get_var_from_lit(lit)]++] = i;
    }
  }
}

void eliminate_redundance() {
  int *lits, i, lit, *clauses, res, clause;

  for (i=0; i<NB_CLAUSE; i++) {
    if (clause_state[i]==ACTIVE) {
      if (clause_length[i]==1)
	push(i, UNITCLAUSE_STACK);
      lits = sat[i]; res=FALSE;
      for(lit=*lits; lit!=NONE; lit=*(++lits)) {
	if (positive(lit)) 
	  clauses=pos_in[lit];
	else clauses=neg_in[lit-NB_VAR];
	for(clause=*clauses; clause!=NONE; clause=*(++clauses)) {
	  if ((clause<i) && (clause_state[clause]==ACTIVE)) {
	    res=redundant(sat[i], sat[clause]);
	    if (res==NEW_CLAUSE_REDUNDANT) {
	      clause_state[i]=PASSIVE;  push(i, CLAUSE_STACK);
	    //  printf("c Clause %d redondante a cause de la clause %d\n", i+1, clause+1);
	      break;
	    }
	    else if (res==OLD_CLAUSE_REDUNDANT) {
	      clause_state[clause]=PASSIVE;  push(i, CLAUSE_STACK);
            //  printf("c Clause %d redondante a cause de la clause %d\n", clause+1, i+1);
            }
	  }
	}
	if (res==NEW_CLAUSE_REDUNDANT)
	  break;
      }
    }
  }
}

my_type build_simple_sat_instance(char *input_file) {
  FILE* fp_in=fopen(input_file, "r");
  char ch, word2[WORD_LENGTH];
  int i, j, length, NB_CLAUSE1, res, ii, jj, tautologie, lit1,
    lits[1000], *lits1, lit, var, *pos_nb, *neg_nb;
  int clause,*vars_signs,cpt;
  if (fp_in == NULL) return FALSE;

  fscanf(fp_in, "%c", &ch);
  while (ch!='p') {
    while (ch!='\n') fscanf(fp_in, "%c", &ch);  
    fscanf(fp_in, "%c", &ch);
  }
  
  fscanf(fp_in, "%s%d%d", word2, &NB_VAR, &NB_CLAUSE);
  if ((tab_variable_size<NB_VAR) || (tab_clause_size<NB_CLAUSE)) {
    fprintf(stdout, 
	 "c Too large formula, please increase tab_variable_size or tab_clause_size\n");
    return FALSE;
  }
  INIT_NB_CLAUSE = NB_CLAUSE;

  if (lire_clauses(fp_in)==FALSE)
    return FALSE;
  fclose(fp_in);
  build_structure();
  eliminate_redundance();
  printf("c %d clauses are redundant and removed\n",
	 CLAUSE_STACK_fill_pointer);
  if (unitclause_process()==NONE) return NONE;
  printf("c %d vars are fixed and %d clauses are removed after unit propagation\n", VARIABLE_STACK_fill_pointer,CLAUSE_STACK_fill_pointer);
  res=simplify_formula();
  printf("c %d vars are fixed and %d clauses are removed after lookahead\n", VARIABLE_STACK_fill_pointer, CLAUSE_STACK_fill_pointer);
  if (res==NONE) return NONE;
  else if (res==FALSE) return FALSE;
  res=clean_structure();
  // to3sat();
  if (res==FALSE)
    return FALSE;
  else if (res==SATISFIABLE)
    return SATISFIABLE;
  return TRUE;
}

int unitclause_process() {
  int unitclause_position;
  int clause;
  for (unitclause_position = 0; 
       unitclause_position < UNITCLAUSE_STACK_fill_pointer;
       unitclause_position++) {
    clause=UNITCLAUSE_STACK[unitclause_position];
    if ((clause_state[clause] == ACTIVE)  && (clause_length[clause]>0)) 
      if (satisfyUnitClause(clause)==NONE)
	return NONE;
  }
  UNITCLAUSE_STACK_fill_pointer=0;
  return TRUE;
}

int satisfyUnitClause(int clause) {
  int var, *vars_signs; 

  vars_signs = var_sign[clause];
  for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2)) {
    if (var_state[var] == ACTIVE) {
      return assignValue(var, *(vars_signs+1));
    }
  }
  printf("c error unit clause");
  return NONE;
}

void removeClause(int clause) {
 clause_state[clause]=PASSIVE;
 push(clause, CLAUSE_STACK);
}

void removeClauses(int* clauses) {
  int clause;

  for(clause=*clauses; clause!=NONE; clause=*(++clauses))
    if (clause_state[clause]==ACTIVE)
      removeClause(clause);
}

int reduceClause(int clause) {
  clause_length[clause]--;
  push(clause, REDUCEDCLAUSE_STACK);
  switch(clause_length[clause]) {
  case 0:
    return NONE;
  case 1: push(clause, UNITCLAUSE_STACK);
    break;
  }
  return TRUE;
}

int reduceClauses(int* clauses) {
  int clause;

  for(clause=*clauses; clause!=NONE; clause=*(++clauses)) {
    if ((clause_state[clause]==ACTIVE) && (reduceClause(clause) == NONE))
      return NONE;
  }
  return TRUE;
}

int assignValue(int var, int value) {
  int *clauses_to_remove, *clauses_to_reduce;
  var_current_value[var]=value; 
  var_state[var]=PASSIVE;
  push(var, VARIABLE_STACK);
  if (value==TRUE) {
    clauses_to_remove=pos_in[var];
    clauses_to_reduce=neg_in[var];
  }
  else {
    clauses_to_remove=neg_in[var];
    clauses_to_reduce=pos_in[var];
  }
  if (reduceClauses(clauses_to_reduce)==NONE)
    return NONE;
  removeClauses(clauses_to_remove);
  return TRUE;
}

int assignValue_and_UP(int var, int value) {
  if (assignValue(var, value)==NONE)
    return NONE;
  if (unitclause_process()==NONE)
    return NONE;
  return TRUE;
}

int unitResolutionForVar(int var, int value) {
  int *clauses_to_reduce;
  var_current_value[var]=value; 
  var_state[var]=PASSIVE;
  push(var, VARIABLE_STACK);
  if (value==TRUE) {
    clauses_to_reduce=neg_in[var];
  }
  else {
    clauses_to_reduce=pos_in[var];
  }
  if (reduceClauses(clauses_to_reduce)==NONE)
    return NONE;
  else return TRUE;
}

int unitResolutionForClause(int clause) {
  int var, *vars_signs; 

  vars_signs = var_sign[clause];
  for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2)) {
    if (var_state[var] == ACTIVE) {
      return unitResolutionForVar(var, *(vars_signs+1));
    }
  }
  return TRUE;
}

int unitResolution() {
  int unitclause_position;
  int clause;
  for (unitclause_position = 0; 
       unitclause_position < UNITCLAUSE_STACK_fill_pointer;
       unitclause_position++) {
    clause=UNITCLAUSE_STACK[unitclause_position];
    if ((clause_state[clause] == ACTIVE)  && (clause_length[clause]>0)) 
      if (unitResolutionForClause(clause)==NONE)
	return NONE;
  }
  UNITCLAUSE_STACK_fill_pointer=0;
  return TRUE;
}

/*
int IMPLIED_LIT_FLAG=0;
int LIT_IMPLIED[2*tab_variable_size];
int IMPLIED_LIT_STACK[2*tab_variable_size];
int IMPLIED_LIT_STACK_fill_pointer=0;
*/

void reset_context1(int saved_var_stack_fill_pointer,
		   int saved_reducedclause_fill_pointer) {
  int i, var;

  UNITCLAUSE_STACK_fill_pointer = 0;

  for (i=saved_var_stack_fill_pointer; 
       i<VARIABLE_STACK_fill_pointer; i++) {
    var=VARIABLE_STACK[i];
    var_state[var] = ACTIVE;
    if (var_current_value[var]==TRUE) 
      LIT_IMPLIED[2*var]=IMPLIED_LIT_FLAG;
    else LIT_IMPLIED[2*var+1]=IMPLIED_LIT_FLAG;
  }
  VARIABLE_STACK_fill_pointer = saved_var_stack_fill_pointer;

  for (i=saved_reducedclause_fill_pointer; 
       i<REDUCEDCLAUSE_STACK_fill_pointer; i++)
    clause_length[REDUCEDCLAUSE_STACK[i]]++;
  REDUCEDCLAUSE_STACK_fill_pointer = 
    saved_reducedclause_fill_pointer;  
}

void reset_context2(int saved_var_stack_fill_pointer,
		   int saved_reducedclause_fill_pointer) {
  int i, var;

  UNITCLAUSE_STACK_fill_pointer = 0;

  for (i=saved_var_stack_fill_pointer; 
       i<VARIABLE_STACK_fill_pointer; i++) {
    var=VARIABLE_STACK[i];
    var_state[var] = ACTIVE;
    if (var_current_value[var]==TRUE) {
      if (LIT_IMPLIED[2*var]==IMPLIED_LIT_FLAG) 
	push(2*var, IMPLIED_LIT_STACK);
    }
    else if (LIT_IMPLIED[2*var+1]==IMPLIED_LIT_FLAG)
      push(2*var+1, IMPLIED_LIT_STACK);
  }
  VARIABLE_STACK_fill_pointer = saved_var_stack_fill_pointer;

  for (i=saved_reducedclause_fill_pointer; 
       i<REDUCEDCLAUSE_STACK_fill_pointer; i++)
    clause_length[REDUCEDCLAUSE_STACK[i]]++;
  REDUCEDCLAUSE_STACK_fill_pointer = 
    saved_reducedclause_fill_pointer;  
}

int satisfyImpliedLits() {
  int lit, var, i, value;
  for(i=0; i<IMPLIED_LIT_STACK_fill_pointer; i++) {
    lit=IMPLIED_LIT_STACK[i];
    if (lit%2==0) {
      var=lit/2; value=TRUE;
    }
    else {
      var=(lit-1)/2; value=FALSE;
    }
    if (var_state[var]==ACTIVE && assignValue_and_UP(var, value)==NONE)
      return NONE;
  }
  return TRUE;
}

int examine(int tested_var) {
  int res, saved_var_stack_fill_pointer, 
    saved_reducedclause_fill_pointer;
  IMPLIED_LIT_FLAG++; IMPLIED_LIT_STACK_fill_pointer=0;
  saved_var_stack_fill_pointer=VARIABLE_STACK_fill_pointer;
  saved_reducedclause_fill_pointer=
    REDUCEDCLAUSE_STACK_fill_pointer;
  unitResolutionForVar(tested_var, TRUE);
  res=unitResolution();
  reset_context1(saved_var_stack_fill_pointer, 
		 saved_reducedclause_fill_pointer);
  if (res==NONE) {
    if (assignValue_and_UP(tested_var, FALSE)==NONE)
      return NONE;
  }
  else {
    unitResolutionForVar(tested_var, FALSE);
    res=unitResolution();
    reset_context2(saved_var_stack_fill_pointer, 
		   saved_reducedclause_fill_pointer);
    if (res==NONE) {
      if (assignValue_and_UP(tested_var, TRUE)==NONE)
	return NONE;
    }
    else {
      if (satisfyImpliedLits()==NONE)
	return NONE;
    }
  }
  return TRUE;
}

int simplify_formula() {
  int var, saved_reducedclause_fill_pointer;

  do {
    saved_reducedclause_fill_pointer=
      REDUCEDCLAUSE_STACK_fill_pointer;
    for(var=0; var<NB_VAR; var++) {
      if (var_state[var]==ACTIVE)
	if (examine(var)==NONE)
	  return NONE;
    }
  } while (saved_reducedclause_fill_pointer<
	   REDUCEDCLAUSE_STACK_fill_pointer);
  return TRUE;
}

int LIT_STACK[2*tab_variable_size];
int LIT_STACK_fill_pointer=0;
int nb_occ[2*tab_variable_size];
int clause_mark[tab_clause_size];
int SPLIT_CLAUSE_CANDIDATE_STACK[tab_clause_size];
int SPLIT_CLAUSE_CANDIDATE_STACK_fill_pointer=0;
int SPLIT_CLAUSE_STACK[tab_clause_size];
int SPLIT_CLAUSE_STACK_fill_pointer=0;

int choose_couple_for_lit(int the_var, int the_sign, int *clauses, 
			  int max, int *couple) {
  int i, *vars_signs, var, clause, lit;
  for(i=0; i<LIT_STACK_fill_pointer; i++) {
    nb_occ[LIT_STACK[i]]=0;
  }
  LIT_STACK_fill_pointer=0;
  for(clause=*clauses; clause!=NONE; clause=*(++clauses)) {
    if (clause_length[clause]>3) {
      vars_signs = var_sign[clause];
      for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2)) {
	if (var != the_var) {
	  if (*(vars_signs+1)==POSITIVE)
	    lit=2*var;
	  else lit=2*var+1;
	  if (nb_occ[lit]==0) 
	    push(lit, LIT_STACK); 
	  nb_occ[lit]++;
	}
      }
    }
  }
  for(i=0; i<LIT_STACK_fill_pointer; i++) {
    if (nb_occ[LIT_STACK[i]]>max) {
      lit=LIT_STACK[i];
      max=nb_occ[lit]; couple[0]=the_var; couple[1]=the_sign;
      if (lit%2==0) {
	couple[2]=lit/2; couple[3]=POSITIVE;
      }
      else {
	couple[2]=(lit-1)/2; couple[3]=NEGATIVE;
      }
    }
  }
  return max;
}

int get_nb_occ3(int *clauses) {
  int clause, cpt=0;
  for(clause=*clauses; clause!=NONE; clause=*(++clauses)) {
    if (clause_length[clause]>3)
      cpt++;
  }
  return cpt;
}

int MAX=100;
/*
int choose_couple(int *couple) {
  int var, max=0, *clauses, chosen_var, chosen_sign, nb;
  for(var=0; var<NB_VAR; var++) {
    clauses=neg_in[var]; nb=get_nb_occ3(clauses);
    if (nb>max) {
      max=nb; chosen_var=var; chosen_sign=NEGATIVE; 
      if (MAX==1) break;
    }
    clauses=pos_in[var]; nb=get_nb_occ3(clauses);
    if (nb>max) {
      max=nb; chosen_var=var; chosen_sign=POSITIVE;
      if (MAX==1) break;
    }
  }
  if (max==0)
    return 0;
  else {
    if (chosen_sign==POSITIVE)
      clauses=pos_in[chosen_var];
    else clauses=neg_in[chosen_var];
    max=choose_couple_for_lit(chosen_var, chosen_sign, clauses, 0, couple);
    MAX=max;
    return max;
  }
}
*/

int choose_couple(int *couple) {
  int var, max=0, *clauses, chosen_var, chosen_sign,
    nb, clause, *vars_signs;
  if (MAX>1) {
    for(var=0; var<NB_VAR; var++) {
      max=choose_couple_for_lit(var, NEGATIVE, neg_in[var], max, couple);
      max=choose_couple_for_lit(var, POSITIVE, pos_in[var], max, couple);
    }
    MAX=max;
    return max;
  }
  else
    for(clause=0; clause<NB_CLAUSE; clause++) {
      if (clause_length[clause]>3) {
	vars_signs=var_sign[clause];
	couple[0]=vars_signs[0]; 
	couple[1]=vars_signs[1]; 
	couple[2]=vars_signs[2]; 
	couple[3]=vars_signs[3];
	return 1;
      }
    } 
  return 0;
}

void remove_clause_in_list(int oldclause, int *clauses) {
  int clause, clause1, first, *suite_clauses;
  for(clause=*clauses; clause!=NONE; clause=*(++clauses)) {
    if (clause==oldclause) {
      break;
    }
  }
  for(suite_clauses=clauses+1; *suite_clauses!=NONE; suite_clauses++) {
    *clauses=*suite_clauses; clauses++;
  }
  *clauses=NONE;
}

int replace_couple_in_clause(int the_var, int the_sign, 
			     int clause, int *couple) {
  int *vars_signs, var;

  var_state[couple[0]]=PASSIVE;  var_state[couple[2]]=PASSIVE;
  remove_passive_vars_in_clause(clause);
  var_state[couple[0]]=ACTIVE;  var_state[couple[2]]=ACTIVE;
  vars_signs=var_sign[clause];
  for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2));
  *vars_signs=the_var;
  *(vars_signs+1)=the_sign;
  *(vars_signs+2)=NONE;
  clause_length[clause]--; static_clause_length[clause]--;
  return TRUE;
}

int create_ternary_clause(int var1, int sign1, int var2, int sign2, 
			  int var3, int sign3) {
  int *vars_signs, new_clause;
  new_clause=NB_CLAUSE++;     clause_mark[new_clause]=0;
  clause_state[new_clause]=ACTIVE; 
  clause_length[new_clause]=3; static_clause_length[new_clause]=3;
  vars_signs=(int *)malloc(7 * sizeof(int));
  vars_signs[0]=var1;  vars_signs[1]=sign1;
  vars_signs[2]=var2;  vars_signs[3]=sign2;
  vars_signs[4]=var3;  vars_signs[5]=sign3;
  vars_signs[6]=NONE; var_sign[new_clause]=vars_signs;
  return new_clause;
}

int create_binary_clause(int var1, int sign1, int var2, int sign2) {
  int *vars_signs, new_clause;
  new_clause=NB_CLAUSE++;     clause_mark[new_clause]=0;
  clause_state[new_clause]=ACTIVE; 
  clause_length[new_clause]=2; static_clause_length[new_clause]=2;
  vars_signs=(int *)malloc(5 * sizeof(int));
  vars_signs[0]=var1;  vars_signs[1]=sign1;
  vars_signs[2]=var2;  vars_signs[3]=sign2;
  vars_signs[4]=NONE; var_sign[new_clause]=vars_signs;
  return new_clause;
}

// negclause is l1 l2 -the_var, posclause1 is -l1 the_var, 
// posclause2 is -l2 the_var.
void create_clause_lists_for_var(int the_var, int negclause, 
				 int posclause1, int posclause2) {
  int *posclauses, i, nb_clauses;
  neg_in[the_var]= (int *)malloc((SPLIT_CLAUSE_STACK_fill_pointer+5)
				 * sizeof(int));
  neg_nb[the_var]=1;
  neg_in[the_var][0]=negclause; neg_in[the_var][1]=NONE;
  pos_in[the_var]= (int *)malloc((SPLIT_CLAUSE_STACK_fill_pointer+5)
				 * sizeof(int));
  pos_nb[the_var]=SPLIT_CLAUSE_STACK_fill_pointer+2;
  posclauses=pos_in[the_var]; nb_clauses=0;
  for(i=0; i<SPLIT_CLAUSE_STACK_fill_pointer; i++) {
    posclauses[nb_clauses++]=SPLIT_CLAUSE_STACK[i];
  }
  posclauses[nb_clauses++]=posclause1;
  posclauses[nb_clauses++]=posclause2;
  posclauses[nb_clauses]=NONE;
}

// couple is l1 l2, clause1 is l1 l2 -l3, clause2 is -l1 l3, 
// clause3 is -l2 l3
void add_clauses_to_lists(int clause1, int clause2, int clause3, int *couple) {
  int *clauses1, *oppclauses1, *clauses2, *oppclauses2;
  if (couple[1]==POSITIVE) {
    clauses1=pos_in[couple[0]];
    oppclauses1=neg_in[couple[0]];
  }
  else {
    clauses1=neg_in[couple[0]];
    oppclauses1=pos_in[couple[0]];
  }
  if (couple[3]==POSITIVE) {
    clauses2=pos_in[couple[2]];
    oppclauses2=neg_in[couple[2]];
  }
  else {
    clauses2=neg_in[couple[2]];
    oppclauses2=pos_in[couple[2]];
  }
  for(; *clauses1!=NONE; ++clauses1);
  *clauses1=clause1; *(clauses1+1)=NONE;
  for(; *clauses2!=NONE; ++clauses2);
  *clauses2=clause1; *(clauses2+1)=NONE;
  for(; *oppclauses1!=NONE; ++oppclauses1);
  *oppclauses1=clause2, *(oppclauses1+1)=NONE;
  for(; *oppclauses2!=NONE; ++oppclauses2);
  *oppclauses2=clause3, *(oppclauses2+1)=NONE;
}

int split_clauses(int the_var, int *clauses1, int *clauses2, int *couple) {
  int clause, *clauses, clause1, clause2, clause3, i;

  for(i=0; i<SPLIT_CLAUSE_CANDIDATE_STACK_fill_pointer; i++) {
    clause_mark[SPLIT_CLAUSE_CANDIDATE_STACK[i]]=0;
  }
  SPLIT_CLAUSE_CANDIDATE_STACK_fill_pointer=0;
  clauses=clauses1;
  for(clause=*clauses; clause!=NONE; clause=*(++clauses)) {
    //  if (clause_length[clause]>3) {
      clause_mark[clause]=1;
      push(clause, SPLIT_CLAUSE_CANDIDATE_STACK);
      // }
  }
  clauses=clauses2; SPLIT_CLAUSE_STACK_fill_pointer=0;
  for(clause=*clauses; clause!=NONE; clause=*(++clauses)) {
    if (clause_mark[clause]==1) {
      push(clause, SPLIT_CLAUSE_STACK);
    }
  }
  clause1=create_ternary_clause(couple[0], couple[1], couple[2], 
				couple[3], the_var, NEGATIVE);
  clause2=create_binary_clause(couple[0], 1-couple[1], the_var, POSITIVE);
  clause3=create_binary_clause(couple[2], 1-couple[3], the_var, POSITIVE);
  for(i=0; i<SPLIT_CLAUSE_STACK_fill_pointer; i++) {
    clause=SPLIT_CLAUSE_STACK[i];
    replace_couple_in_clause(the_var, POSITIVE, clause, couple);
    remove_clause_in_list(clause, clauses1);
    remove_clause_in_list(clause, clauses2);
  }
  add_clauses_to_lists(clause1, clause2, clause3, couple);
  create_clause_lists_for_var(the_var, clause1, clause2, clause3);
  return TRUE;
}

int to3sat() {
  int couple[4], *clauses1, *clauses2, var, clause, l, *vars_signs,
    saved_nb_clause, saved_nb_var;

  saved_nb_clause=NB_CLAUSE; saved_nb_var=NB_VAR;
  for(var=0; var<NB_VAR; var++) {
    nb_occ[2*var]=0;  nb_occ[2*var+1]=0;
  }
  for(clause=0; clause<NB_CLAUSE; clause++)
    clause_mark[clause]=0;
  while (choose_couple(couple)>0) {
    if (couple[1]==POSITIVE)
      clauses1=pos_in[couple[0]];
    else clauses1=neg_in[couple[0]];
    if (couple[3]==POSITIVE)
      clauses2=pos_in[couple[2]];
    else clauses2=neg_in[couple[2]];
    var=NB_VAR++;  NB_ACTIVE_VAR++; var_state[var]=ACTIVE;
    nb_occ[2*var]=0; nb_occ[2*var+1]=0;
    split_clauses(var, clauses1, clauses2, couple); 
  }
  for(clause=0; clause<NB_CLAUSE; clause++) {
    if (clause_length[clause]>3)
      printf("c erreur\n");
    vars_signs=var_sign[clause]; l=0;
    for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2)) l++;
    if (l>3) 
      printf("c erreur\n");
  }
  printf("c %d vars and %d clauses are added to transform to 3sat\n",
	 NB_VAR-saved_nb_var, NB_CLAUSE-saved_nb_clause);
  return TRUE;
}

int verify_sol_input(char *input_file) {
  FILE* fp_in=fopen(input_file, "r");
  char ch, word2[WORD_LENGTH];
  int i, j, lit, var, nb_var1, nb_clause1;

  if (fp_in == NULL) return FALSE;

  fscanf(fp_in, "%c", &ch);
  while (ch!='p') {
    while (ch!='\n') fscanf(fp_in, "%c", &ch);  
    fscanf(fp_in, "%c", &ch);
  }
  
  fscanf(fp_in, "%s%d%d", word2, &nb_var1, &nb_clause1);
  for (i=0; i<nb_clause1; i++) {
    fscanf(fp_in, "%d", &lit);
    while (lit != 0) {
      if (lit<0) {
	if (var_current_value[abs(lit)-1]==FALSE)
	  break;
      }
      else {
	if (var_current_value[lit-1]==TRUE)
	  break;
      }
      fscanf(fp_in, "%d", &lit);
    }
    if (lit==0) {
      fclose(fp_in);
      printf("c something is wrong\n");
      return FALSE;
    }
    else {
      do fscanf(fp_in, "%d", &lit);
      while (lit != 0) ;
    }
  }
  fclose(fp_in);
  return TRUE;
}

//#include "inputbis56.c"

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
int previous_flip_time[tab_variable_size];
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
//int MaxClauseWeight, TotalClauseWeight, NB_EMPTY;
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
  int var, neibor_var, i, **vector_relations, *vector, *relations;

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
  int var, gradient, neg_gradient, pos_gradient, clause;

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
  MAX_COUNTER=0; // MaxClauseWeight=1; TotalClauseWeight=NB_CLAUSE;
  /*
  NB_EMPTY=0;
  for(clause=0; clause<NB_CLAUSE; clause++) {
    most_recent[clause]=NONE;
    most_recent_count[clause]=0;
    clause_weight[clause]=1;
  }
  */
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
      if (previous_flip_time[var]>previous_flip_time[recent_var]) {
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
    LNOISE1;
  register int *vars_signs;

  if (random_integer(100)<LNOISE) 
    return diversify(random_clause_unsat);

  vars_signs = var_sign[random_clause_unsat]; max_nb=-NB_CLAUSE;
  for(var=*vars_signs; var!=NONE; var=*(vars_signs+=2)) {
    nb=score[var];
    if ((nb>max_nb) || ((nb==max_nb) && (previous_flip_time[var]<previous_flip_time[best_var]))) {
      second_best_var=best_var; second_max=max_nb; best_var=var; max_nb=nb;
    }
    else if ((nb>second_max) || 
	     ((nb==second_max) && (previous_flip_time[var]<previous_flip_time[second_best_var]))) {
      second_max=nb; second_best_var=var;
    }
    if (previous_flip_time[var]>flip) 
      flip=previous_flip_time[var];
  }
  if (previous_flip_time[best_var]==flip) {
    /*    if (most_recent_count[random_clause_unsat]==1)
      LNOISE1=NOISE/15;
    else LNOISE1=NOISE/10;
    switch(most_recent_count[random_clause_unsat]) {
      case 1: LNOISE1=NOISE/15; break;
	//case 2: LNOISE1=NOISE/10; break;
	// case 3: LNOISE1=NOISE/10; break;
	// case 4: LNOISE1=NOISE/9; break;
	//  case 5: LNOISE1=NOISE/8; break;
	//case 6: LNOISE1=NOISE/7; break;
	// case 7: LNOISE1=NOISE/6; break;
      default: LNOISE1=NOISE/10; break;
    }
    */
    if (random_integer(100)<NOISE/10)
      return diversifyForUneven(random_clause_unsat, best_var);
    else if (random_integer(100)<NOISE) 
      return second_best_var; 
    else return best_var;
  }
  else return best_var;
}

void satisfy_clauses(int var, int *clauses) {
  int clause,  neibor_var, *vars_signs, dommage=0, last_unsatisfied_clause,
    index;
  for (clause=*clauses;clause!=NONE;clause=*(++clauses)) {
    vars_signs=var_sign[clause];
    switch(nb_lit_true[clause]) {
    case 0: clause_truth[clause]=TRUE; nb_lit_true[clause]++;
      //  dommage++;
      last_unsatisfied_clause=pop(MY_CLAUSE_STACK);
      index=index_in_MY_CLAUSE_STACK[clause];
      MY_CLAUSE_STACK[index]=last_unsatisfied_clause;
      index_in_MY_CLAUSE_STACK[last_unsatisfied_clause]=index;
      for(neibor_var=*vars_signs; neibor_var!=NONE; 
	  neibor_var=*(vars_signs+=2)) {
	if (neibor_var!=var) {
	    tmp_score[neibor_var]--;
	}
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
  // if (dommage==0) 
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
    satisfy_clauses(var, pos_in[var]);
    unsatisfy_clauses(var, neg_in[var]);
  }
  else {
    satisfy_clauses(var, neg_in[var]);
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
  flip=flip_time[var];//previous_flip_time[var]; 
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
    NOISE += (int) ((100 - NOISE) / invPhi);
    LNOISE= (int) NOISE/10;
    lastAdaptFlip = flip;      
    // NB_BETTER=0;
    // if (MY_CLAUSE_STACK_fill_pointer < lastBest)
      lastBest = MY_CLAUSE_STACK_fill_pointer;
      //     lastEvenSteps=nb_even; lastUnEvenSteps=nb_uneven;
  } 
  else if (MY_CLAUSE_STACK_fill_pointer < lastBest) {
    //  NB_BETTER++;
    //  if (NB_BETTER>1) {
    NOISE -= (int) (NOISE /(2*invPhi));
    LNOISE= (int) NOISE/10;
    lastAdaptFlip = flip;
    lastBest = MY_CLAUSE_STACK_fill_pointer;
    //    lastEvenSteps=nb_even; lastUnEvenSteps=nb_uneven;
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
	previous_flip_time[k]=0;
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
      previous_flip_time[var_to_flip]=flip_time[var_to_flip];
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
    SEED=SEED+113;
    //   srand(SEED);
    printf("c initial seed is %u\n", SEED);
    if (search(argv[1])==TRUE) {
      exit(10);
    }
    else 
      exit(0);
    break;
  case NONE: fprintf(stdout, "c A contradiction is found at top!\n");
    fprintf(stdout, "s UNSATISFIABLE\n");
    exit(20);
  }
  return TRUE;
}

