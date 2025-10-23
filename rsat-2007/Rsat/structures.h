#ifndef STRUCTURES_DEFINED 
#define STRUCTURES_DEFINED 

#include <time.h>
#include <stdio.h>

#include "flags.h"

#define calloc my_calloc
#define malloc my_malloc

typedef struct heap_t{
  my_var* order;    //order[i] = variable at heap position i
  int* indices;  //indices[x] = index of variable x in the (heap) order array
  int size;      //current heap size (# things in heap)
} heap;

typedef heap implication_queue;

typedef struct clause_tt{

  int index;
  my_lit* lits;
  int size;
} my_clause;

typedef struct manager_tt{

  int vc;
  int cc;  //original clauses only
  my_clause** original_clauses;
  int original_clauses_array_len;
  my_clause** learned_clauses;
  int learned_clauses_array_len;
  
  int* level;
  my_lit* assign;
  my_lit* assign_top;
  my_lit** decision_lit;
  int decision_lit_size;
  my_lit* status;
  my_clause** reason;
  double* score;
  my_clause*** watched;  //the entry of literal l is at index 2*(var(l)-1)+sign(l)
  int* watched_size;   //the size of the watched list of each literal
  int* watched_len;    //the actual length of the watched list of each literal

  int decision_level;
  double restart_conflict_incr;
  int next_restart_conflict;
  int restart;
  int stack_offset;
  char ok;
  my_clause* conflict_clause;
  int assertion_level;
  int conflict_level;

  implication_queue* imp;  

  char save_progress;
  int decisions_under_influence;
  my_lit* saved;
  
  double* learned_clause_scores;

  int decisions;
  int conflicts;
  int cdc_count;          //total number of cd clauses ever learned
  int cdl_count;
  int cur_cdc_count;      //the current number of cd clauses exist
  int cur_cdl_count;      //the current number of cd literals
  int cur_cc;             //the current number of original clauses
  int cur_lit_count;      //the current number of literals in the original clauses
  int max_decision_level;
  int original_literals_count; //total number of literals in the original instance that still remain
  int original_clauses_count;  //total number of clauses in the original instance that still remain
  
  my_lit* cdc;
  int cdc_size;

  int cdc_index;
  int lits_at_clevel;

  char* seen;
  my_lit* stack;
  int stack_size;
  int* save;
  int save_size;
  heap* var_order_heap;

  double score_inc;
  double score_inc_factor;
  
  double clause_score_inc;
  double max_learned_clauses;    
  double min_learned_clauses_to_restart;
  char reduced;

  int db_reduction_count;
  int db_simplification_count;
  int num_conflicts_for_next_simplify;
  int next_simplify_increment;
  int simplify_count;
  int simplify_literals_count;
  char simplify_orig_kb;
  char simplify_learned_kb;

  int luby_unit;

  int on_th;
  int on_th_inc;
  int off_th;
  int off_th_inc;
  int next_sp_switch;
} my_manager;

#define sign(l) (l&1)   //sign(l) is 1 if l is a negative literal
#define neg(l)  (l^1)   //compute the negation of literal l
#define lit_index(l) (sign(l)?-(l>>1):l>>1)  //compute the integer representation of literal l
#define var(l) (l>>1)
#define plit(v) (v<<1)
#define nlit(v) ((v<<1)|1)
#define set(l) (status[var(l)]==l)//status[var(l)]*lit_index(l)
#define watched_index(l) (2*(var(l)-1)+sign(l))
#define lit_from_int(in) (in<0?((-in)<<1)|1:in<<1)
#define unresolved(l) (status[var(l)]!=neg(l))
#define free_lit(l) (status[var(l)]==0)
#define resolved(l) (status[var(l)]==neg(l))

void read_cnf(char*);
void initialize_watched_literals();
void remove_watched_clause(my_lit,my_clause*);
BOOLEAN process_unit_literal_queue();
void add_conflict_driven_clause();
void analyze_conflict(my_clause*,int level);
void erase_level(int level);
void free_cnf_manager();
void print_clause(my_clause*);
void check_watched_list(char);
void check_watched_literal_completeness();
void print_watched_list(my_lit l);
void print_stats();
BOOLEAN verify_solution();
void print_solution();
void enqueue(my_lit);
BOOLEAN set_literal(my_lit);
char is_key_literal(my_lit l);
double drand();
void print_reasons(my_var);
void remove_reason(my_lit,my_clause*);
void print_location();
void add_watched_clause(my_lit,my_clause*);
void declare_watched_literals(my_clause*);
void remove_clause(my_clause*);
void sort_clauses_by_activity(my_clause***,double**,int);
void check_sorted_clause_array(my_clause**,double*,int);
int get_luby(int i);
void check_assignment_stack();
int compute_init_learned_clause_limit();
int sat();
void undo_assert_unit_clauses();
BOOLEAN decide_literal(my_lit);
void undo_decide_literal();
BOOLEAN assert_cd_literal();
BOOLEAN at_assertion_level();
void save_solution();
void print_solution();
void* my_calloc(size_t,size_t);
void* my_malloc(size_t); 
#endif//endif ifndef (topmost)
