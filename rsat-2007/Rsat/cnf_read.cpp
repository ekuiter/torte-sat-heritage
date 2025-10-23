#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

#include "flags.h"
#include "globals.h"
#include "types.h"
#include "structures.h"
#include "heap.h"

#define var_ind(x) (x<0?-x:x)
extern my_manager* my_m;

void sort_literals(CNF_LITERAL_INDEX* arr,int size)
{
  //for each position
  for(int i=0;i<size-1;i++){
    int cur_lit = arr[i];
    int best = var_ind(cur_lit);
    int best_index = i;

    for(int j=i+1;j<size;j++){
      int lit = arr[j];
      if(var_ind(lit)<best){
	best = var_ind(lit);
	best_index = j;
      }
    }//end for j

    //at this point, we found the fit for spot i
    //swap
    CNF_LITERAL_INDEX temp = arr[i];
    arr[i] = arr[best_index];
    arr[best_index] = temp;
  }
}

int parse_int(char** in)
{
  int val = 0;
  char neg = 0;
  
  //skipping white space
  while(**in==' ' || **in=='\t'){
    (*in)++;
  }

  if(**in=='-'){
    neg = 1;
    (*in)++;
  }else if(**in=='+'){
    (*in)++;
  }

  if(**in < '0' || **in > '9'){
    printf("c Parse error: unexpected character %c[%d]\n",**in,**in);
    exit(1);
  }

  while(**in >= '0' && **in <= '9'){
    val = val*10 + (**in - '0');
    (*in)++;
  }//end while

  return (neg?-val:val);
}

void init_clause(CNF_LITERAL_INDEX* literals,int index,int size)
{  
  my_manager* m = my_m;

  my_clause* temp = (my_clause*)calloc(1,sizeof(my_clause));  
  
  m->original_clauses[index] = temp;

  my_clause* cur = m->original_clauses[index];
  cur->size = size;
  cur->index = -(index+1);  //index of original clauses are negative (starting from -1)
  cur->lits = (my_lit*)calloc(size,sizeof(my_lit));

  my_m->cur_lit_count += size;
  for(int i=0; i < size; i++) {
    cur->lits[i] = lit_from_int(literals[i]);
  }//end for i
  
}//end function init_clause

void init_manager()
{
  my_manager* m = my_m;
  
  /* decisions start at level 2 since:
     level 1: is for literals implied by cnf (without justification) (either intrinsically unit or pure literal)
     level 0: is reserved to indicate variable is not implied */
  m->decision_level = 1;
  m->max_decision_level = 0;
  m->decisions = 0;
  m->conflicts = 0;
  m->cdc_count = 0;
  m->cdl_count = 0;
  m->cur_cdl_count = 0;
  m->cur_cdc_count = 0;
  m->cur_cc = 0;
  m->cur_lit_count = 0;
  m->original_clauses_count = 0;
  m->conflict_level = 0;
  
  m->restart = 0;
  m->next_restart_conflict = LUBY_UNIT;

  m->stack_offset = 0;
  m->ok = 1;  
  m->save_progress = 1;

  m->decisions_under_influence = 0;
  
  int vc = m->vc;

  /* a literal and its negation may be put on stack, hence, 2*stack_size */
  m->assign = (my_lit*)calloc(2*vc,sizeof(my_lit));
  m->assign_top = m->assign;
  
  int init_decision_lit_size = (vc>VC_THRESHOLD && RESIZE_DECISION_LIT_ARRAY?vc/10:vc);
  
  m->decision_lit = (my_lit**)calloc(init_decision_lit_size,sizeof(my_lit*));
  m->decision_lit[1] = m->assign;
  m->decision_lit_size = init_decision_lit_size;
  
  //Allocating various arrays for variables
  m->level = (int*)calloc(vc+1,sizeof(int));  //var-indexed
  m->status = (my_lit*)calloc(vc+1,sizeof(my_lit)); //var-indexed
  m->reason = (my_clause**)calloc(vc+1,sizeof(my_clause*)); //var-indexed
  m->score = (double*)calloc(vc+1,sizeof(double)); //var-indexed
  
  m->var_order_heap = (heap*)calloc(1,sizeof(heap));
  init_heap(m->var_order_heap,vc);

  m->saved = (my_lit*)calloc(vc+1,sizeof(my_lit));
  
  
  m->learned_clauses = (my_clause**)calloc(1,sizeof(my_clause*));
  m->learned_clauses_array_len = 1;
  
  m->imp = (implication_queue*)calloc(1,sizeof(implication_queue));
  init_heap(m->imp,vc);
  
  m->learned_clause_scores = (double*)calloc(1,sizeof(double));   
  
  int init_stack_and_save_size = (RESIZE_STACK_AND_SAVE_ARRAYS?vc/100:vc);
  init_stack_and_save_size = (init_stack_and_save_size<10?10:init_stack_and_save_size);

  int init_cdc_size = (RESIZE_CDC_ARRAY?vc/100:vc);
  init_cdc_size = (init_cdc_size<100?100:init_cdc_size);
  
  m->cdc = (my_lit*)calloc(init_cdc_size,sizeof(my_lit));
  m->cdc_size = init_cdc_size;
  m->seen = NULL;//allocation done in derive_conflict_clause //(char*)calloc(vc+1,sizeof(char)); //var-indexed
  m->stack = (my_lit*)calloc(init_stack_and_save_size,sizeof(my_lit));
  m->stack_size = init_stack_and_save_size;
  m->save = (int*)calloc(init_stack_and_save_size,sizeof(int));
  m->save_size = init_stack_and_save_size;
  
  //one slot for each literal
  m->watched = (my_clause***)calloc(2*vc,sizeof(my_clause**));
  m->watched_size = (int*)calloc(2*vc,sizeof(int));
  m->watched_len = (int*)calloc(2*vc,sizeof(int));
  
  //finishing up initialization
  m->score_inc = 1;
  m->score_inc_factor = 1/SCORE_INC_PARAM;
  
  m->db_reduction_count = 0;
  m->clause_score_inc = 1;
  m->reduced = 0;
  
  m->db_simplification_count = 0;
  m->num_conflicts_for_next_simplify = 0;
  m->next_simplify_increment = NEXT_SIMPLIFY_INCREMENT;
  m->simplify_count = 0;
  m->simplify_literals_count = 0;
  m->simplify_orig_kb = 0;
  m->simplify_learned_kb = 0;
  
    
    m->luby_unit = LUBY_UNIT;

    //these are in terms of number of conflicts I suppose
    m->save_progress = 0;  //start with no progress saving
    m->on_th = INIT_ON_TH;
    m->off_th = INIT_OFF_TH;
    m->on_th_inc = INIT_ON_TH_INC;
    m->off_th_inc = INIT_OFF_TH_INC;
    m->next_sp_switch = m->on_th;
}//end function init_manager

void finish_up_init_manager()
{
  
  my_m->max_learned_clauses = my_m->cc/3;
  my_m->min_learned_clauses_to_restart = (int)my_m->max_learned_clauses*LEARNED_CLAUSE_FACTOR;
  initialize_watched_literals();
}

inline void skipLine(FILE *ifp)
{  
  char line[MAX_LINE_LEN]; /* temporary space for reading a line */
  line[MAX_LINE_LEN-1] = '\n'; /* anything other than '\0' */
  fgets(line,MAX_LINE_LEN,ifp);
  if(line[MAX_LINE_LEN-1]=='\0'){
    printf("Line exceeds max length of %d!\n",MAX_LINE_LEN-2);
    exit(1);
  }
}

/* Reads a line into the array line. Signals an error in case the
lines fills the array completely. */
void read_line(char *line,unsigned long *i, FILE *ifp) 
{
  ++*i;
  fgets(line,MAX_LINE_LEN,ifp);
  if (line[MAX_LINE_LEN-1]=='\0') {
    printf("Line %d exceeds max length of %d!\n",*i,MAX_LINE_LEN-2);
    exit(1);
  }
}

void read_cnf(char* fname)
{
  void read_line(char*,unsigned long*,FILE*);
  void skipLine(FILE*);

  my_m = (my_manager*)calloc(1,sizeof(my_manager));
  
  my_manager* m = my_m;

  FILE *ifp;
  unsigned long i= 0;
  
  char line[MAX_LINE_LEN]; 
  line[MAX_LINE_LEN-1] = '\n';
  CNF_LITERAL_INDEX literals[MAX_CLAUSE_LEN];
  char c; 
  
  int cc,vc;
  int cur_clause_index = 0;

  if((ifp=fopen(fname,"r"))==NULL){
    printf("\nCannot open file: %s\n",fname);
    exit(1);
  }

  while((c=getc(ifp))!=EOF){
    if(isspace(c)) continue; //skip any space at the beginning of the line
    else ungetc(c,ifp);

    if(c=='c' || c== '%' || c== '0'){
      //comment
      read_line(line,&i,ifp);
    }else if(c=='p'){
      read_line(line,&i,ifp);
      if(sscanf(line,"p cnf %d %d",&vc,&cc)==2){
	m->original_clauses = (my_clause**)calloc(cc,sizeof(my_clause*));
	m->vc = vc;
	m->cc = cc;
	//m->cc is not finalized yet (we may skip clauses)
	init_manager();
      }else{
	printf("Unknown line %d: %s",i,line);
	exit(0);
      }
    }else if((c=='-') || (isdigit(c) > 0)){
      int j=0;
      read_line(line,&i,ifp);
      char* to_read = line;
      BOOLEAN skip_clause = 0;
      
      while(1){
	int parse_lit = parse_int(&to_read);
	if(parse_lit==0){
	  literals[j] = 0;
	  break;
	}//end if

	//check to make sure 
	//parse_lit is not set or resolved
	//if it is, take appropriate action
	
	int var_index = (parse_lit<0?-parse_lit:parse_lit);
	//char status = 0; //1 set, -1 resolved
	my_lit status = m->status[var_index];
	char is_set = 0;

	if(status==0){
	  is_set = 0;	  
	}else{
	  if(parse_lit<0){
	    is_set = (status==plit(var_index)?-1:1);
	  }else{
	    is_set = (status==plit(var_index)?1:-1);
	  }
	}

	if(is_set==1){
	  //this clause is already satisfied
	  //skip it
	  skip_clause = 1;
	  break;
	}

	if(is_set==-1){
	  //this literal is already resolved
	  //skip it
	  continue;
	}

	literals[j++] = parse_lit;
	
      }//end while(1)

      if(j==MAX_CLAUSE_LEN-1 && literals[j]!=0){
	printf("Clause at line %d exceeds maximum length\n",i);
	exit(0);
      }

      //check for duplication

      int k=0;
      int n=0;
      int duplicate_literals_count = 0;

      //j is the size (not including the trailing 0 of the current clause
      sort_literals(literals,j);
      

      if(!skip_clause){

	while(k<=j) {
	  //checks 
	  BOOLEAN duplicate_literal = 0;
	  for(int x=0; x<k; x++) {
	    
	    if(literals[x]==literals[k]) { 
	      // duplicate literals in same clause
	      duplicate_literal = 1;
	      ++duplicate_literals_count;
	      break;
	    }
	    else if(abs(literals[x])==abs(literals[k])) {
	      // literal and its negation in same clause 
	      skip_clause=1;
	      // keep reading clause; no interruption 
	    }
	  }//end for x
	  
	  if(duplicate_literal==0) {
	    literals[n++] = literals[k++];
	  }
	  else {
	    //literals[k] can be discarded
	    k++;
	  }
	}//end while k<j

      }//endif !skip_clause	
      
      if(n==2){
	//this clause is not sat and is unit (after removing any duplicate literal)
	int var_index = (literals[0]<0?-literals[0]:literals[0]);
	my_lit l;

	if(literals[0]<0){
	  l = nlit(var_index);
	}else{
	  l = plit(var_index);
	}

	
	my_lit* status = my_m->status;

	if(set(l)<0){
	  //this literal has already been resolved
	  //unsatisfiable
	  exit(0);
	  m->ok = 0;
	}

	if(m->status[var_index]==0){
	  m->level[var_index] = 1;
	  m->reason[var_index] = NULL;
	  enqueue(l);
	}

	skip_clause=1;
      }else if(n==1){	
	m->ok=0;
      }

      if(!skip_clause){
	//n is now the size of this clause (including the ending 0)
	//n-1 is the real size of the clause
	init_clause(literals,cur_clause_index,n-1);
	cur_clause_index++;
      }
      
    }else{
      read_line(line,&i,ifp);
      printf("Unknown line %d: %s",i,line);
      exit(0);
    }//end if else c==?
  }//end while not eof

  m->cc = cur_clause_index;
  m->cur_cc = cur_clause_index;

  m->original_clauses_array_len = cur_clause_index;
  if(cur_clause_index!=cc){
    //some clauses were skipped
    m->original_clauses = (my_clause**)realloc(m->original_clauses,cur_clause_index*sizeof(my_clause*));
  }
  finish_up_init_manager();
}
