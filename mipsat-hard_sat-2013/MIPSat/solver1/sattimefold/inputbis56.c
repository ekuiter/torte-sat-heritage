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
    if (length>3) 
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
