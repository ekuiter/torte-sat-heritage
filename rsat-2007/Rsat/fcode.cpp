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
extern my_manager* my_m;

#define min(x,y) (x<y?x:y)

void sort_clauses_by_activity(my_clause*** _arr,double** _score,int size)
{  
  
  my_clause** arr = *_arr;
  double* score = *_score;
  my_clause** target = (my_clause**)calloc(size,sizeof(my_clause*));
  double* starget = (double*)calloc(size,sizeof(double));
  
  //assume size = 2^k for some integer k.
  int i = 0;
  
  for(;;i++){
 
    int block_size = (int)(pow(2,i));

    if(block_size>size/2){
      break;
    }
    
    int lindex = 0;
    int rindex = block_size;
    
    int l = 0;
    int r = 0;

    //for each position in the target array
    for(int j=0;j<size;j++){

      if(l==block_size && r==block_size){
	lindex += block_size;
	rindex += block_size;
	l = 0;
	r = 0;
	
	double s1 = score[lindex];
	double s2 = score[rindex];

	if(s1<s2){
	  target[j] = arr[lindex];
	  starget[j] = score[lindex];

	  lindex++;
	  l++;
	}else{
	  target[j] = arr[rindex];
	  starget[j] = score[rindex];	  

	  rindex++;
	  r++;
	}

      }else if(l==block_size){
	target[j] = arr[rindex];
	starget[j] = score[rindex];
	
	rindex++;
	r++;
      }else if(r==block_size){
	target[j] = arr[lindex];
	starget[j] = score[lindex];
		  
	lindex++;
	l++;
      }else{
	//both l and r are not empty
	
	double s1 = score[lindex];
	double s2 = score[rindex];

	if(s1<s2){
	  target[j] = arr[lindex];
	  starget[j] = score[lindex];
  
	  lindex++;
	  l++;
	}else{
	  target[j] = arr[rindex];
	  starget[j] = score[rindex];
	  
	  rindex++;
	  r++;
	}//endif else s1<s2
      }//endif else l==block_size && r==block_size
      
      if(target[j]!=NULL){
	target[j]->index = j;
      }
      
    }//end for j
    
    //done filling up target array
    my_clause** temp = target;
    target = arr;
    arr = temp;

    double* stemp = starget;
    starget = score;
    score = stemp;
    
  }//end for i

  *_arr = arr;
  *_score = score;

  free(target);
  free(starget);
}

int get_luby(int i)
{
  if(i==1){return my_m->luby_unit;}  
  double k = log2(i+1);
  
  if(k==round(k)){
    return (int)(pow(2,k-1))*my_m->luby_unit;
  }else{
    k = (int)floor(k);
    return get_luby(i-(int)pow(2,k)+1);
  }
}
