/*
 * heap.cpp
 * Created 12/20/05 by Knot
 * based on MiniSat's Heap.C
 *
 */

#include <stdlib.h>
#include <math.h>

#include "globals.h"
#include "types.h"
#include "flags.h"
//#include "cnf.h"
#include "structures.h"
#include "heap.h"

#define left(i) (i*2)
#define right(i) ((i*2)+1)
#define parent(i) (i/2) //this automatically rounds down
#define max(x,y) (x>y?x:y)
#define min(x,y) (x<y?x:y)

#define gt(x,y) gt5(x,y)

extern my_manager* my_m;

inline double var_score_vsids(my_var v)
{
  //maximum of the positive literal count and the negative literal count
  //return MAX(var->pliteral->lit_count,var->nliteral->lit_count);
  //modified by Knot 12/12/05
  //return MAX(var->pliteral->score,var->nliteral->score);
  //modified by Knot 12/22/05 SUM is so much better than MAX, empirically
  //return var->score;

  //return var->score;
  return my_m->score[v];
  
}

/*
 * return true if the score(s) of v1 is greater than the score(s) of v2
 *
 */
inline BOOLEAN gt5(my_var v1,my_var v2)
{
  double s1 = my_m->score[v1];//var_score_vsids(v1);
  double s2 = my_m->score[v2];//var_score_vsids(v2);

  if(s1>s2){return 1;}

  return 0;
}

inline void percolateUp(heap* h,int i)
{
  my_var* order = h->order;
  int* indices = h->indices;
  double* score = my_m->score;
  my_var v = order[i];
  int parent_index = parent(i);
  double v_score = score[v];
  //while v's score is still greater than that of its parent
  //i is the order index of the current position of the thing we are percolating

  while(parent_index!=0 && gt(v,/*h->order[parent_index]*/order[parent_index])){
    int ind = order[parent_index];
    order[i] = ind;
    
    indices[ind] = i;
    i = parent_index;          //i moves up
    parent_index = parent(i);  //parent = next parent

  }//end while

  order[i] = v;
  indices[v] = i;
}

inline void percolateDown(heap* h,int i)
{
  my_var* order = h->order;
  int* indices = h->indices;
  my_var v = order[i];  
  double* score = my_m->score;
  double v_score = score[v];
  int h_size = h->size;

  //until we run out of heap elements
  while(left(i) <= h_size){
    int left_index = left(i);
    int right_index = right(i);

    int bigger_child = ((right_index<=h->size && gt(order[right_index],order[left_index])) ? right_index:left_index);    
    if(!gt(order[bigger_child],v)){
      //found the place
      break;
    }

    order[i] = order[bigger_child];
    indices[order[i]] = i;
    i = bigger_child;
  }//end while

  order[i] = v;
  indices[v] = i;
}

/*
 * allocate space for all variables in the instance
 *
 */
void init_heap(heap* h,CNF_VAR_INDEX n)
{
  h->order = (my_var*)calloc(n+1,sizeof(my_var));  //+1 because order[0] is not used
  h->indices = (int*)calloc(n+1,sizeof(int));
  h->size = 0;
}

/* 
 * Put variable v in the heap based on v's score
 */
void insert(heap* h,my_var v)
{  
  int position = (++h->size);
  
  h->indices[v] = position; //put l in the last position in the heap
  h->order[position] = v;
  percolateUp(h,position);

}

/*
 * get variable with index v to the right position
 *
 */
void increase(heap* h,my_var v)
{
  percolateUp(h,h->indices[v]);
}

/*
 * get variable with index v to the right position
 *
 */
void decrease(heap* h,my_var v)
{
  percolateDown(h,h->indices[v]);
}

/*
 * get the index of the variable with highest score 
 *
 */
my_var getMin(heap* h) 
{
  my_var r = h->order[1];   //min element is always kept in [1]
  h->order[1] = h->order[h->size];  //replace min with the last element to keep the heap's shape correct
  h->size--;                        //decrement the heap's size
  h->indices[h->order[1]] = 1;
  h->indices[r] = 0;             //r is now out of the heap
  if(h->size > 1){
    percolateDown(h,1);            //get the top element in the right place
  }
  
  return r;
}

BOOLEAN empty(heap* h)
{
  return h->size==0;
}

/*
 * is variable with index v still in the heap?
 *
 */
BOOLEAN inHeap(heap* h,my_var v)
{
  return h->indices[v]!=0;
}

/*
 * update the position of variable with index v in the heap
 */
void update(heap* h,my_var v)
{
  if(inHeap(h,v)){
    increase(h,v);
  }
}

/*
 * update the position of variable with index v in the heap
 */
void update2(heap* h,my_var v)
{
  if(inHeap(h,v)){
    decrease(h,v);
  }
}

void undo(heap* h,my_var v)
{
  if(!inHeap(h,v)){
    insert(h,v);
  }
}

my_var select_next_var()
{
  double random_var_freq = 0.2;

  heap* h = my_m->var_order_heap;

  while(!empty(h)){
    my_var next = getMin(h);   //this takes care of restructuring heap
    if(my_m->level[next]==0){
      //if this variable is free, return it
      return next;
    }//end if

  }//end while

  //no more free variable:SAT
  return 0;
}

my_var dequeue(implication_queue* h)
{
  while(!empty(h)){
    my_var next = getMin(h);   //this takes care of restructuring heap
    return next;
  }//end while

  //no more free variable:SAT
  return 0;
}

void rescaleVariableActivity()
{ 
  int size = my_m->vc;
  double* score = my_m->score;
  double score_divider;
  score_divider = SCORE_DIVIDER;

  for (int i = 1; i <= size; i++){    
    score[i] *= score_divider;
  }

  my_m->score_inc *= score_divider; 
}

void bumpLiteralActivity(my_lit l)
{  
  my_var v = var(l);
  double score = my_m->score_inc;
  my_m->score[v] += score;
  score = my_m->score[v];

  if(score > SCORE_LIMIT){
    rescaleVariableActivity();
  }
  
  update(my_m->var_order_heap,v);

}

void rescaleClauseActivity()
{
  int size = my_m->cur_cdc_count;
  double* scores = my_m->learned_clause_scores;
  for(int i=0;i<size;i++){
    scores[i] *= CLAUSE_SCORE_DIVIDER;
  }
  my_m->clause_score_inc *= CLAUSE_SCORE_DIVIDER;
}

/*
 * assume c is learned
 */
void bumpClauseActivity(my_clause* c)
{

  if((my_m->learned_clause_scores[c->index] += my_m->clause_score_inc) > CLAUSE_SCORE_LIMIT){
    rescaleClauseActivity();
  }

}

void free_heap(heap* h)
{
  free(h->order);
  free(h->indices);
}
