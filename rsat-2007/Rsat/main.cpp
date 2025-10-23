#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <signal.h>

#include "types.h"
#include "flags.h"
#include "globals.h"
#include "structures.h"

void printUsage();
double get_cpu_time();
char* fname;
my_manager* my_m;
char print_result;

//Knot added 1/5/06 for time out implementation
double time_out;  //initialized in main
clock_t start_t,end_t;

FILE* res = NULL;

void SIGINT_handler(int signum) 
{
  /*Commented out for SAT07 submission
  void printProgress();
  void printProgressFooter();

  printProgress();
  printProgressFooter();  
  */
  rprintf("\n");
  rprintf("\n");
  rprintf("INTERRUPTED\n");
  print_stats();
  
  end_t = clock();
  double time_used = (double)(end_t-start_t)/CLOCKS_PER_SEC;

  rprintf("Time used: %fs",(double)(end_t-start_t)/CLOCKS_PER_SEC);

  exit(0); 
}

void SIGSEGV_handler(int signum) 
{
  /*Commented out for SAT07 submission
  void printProgress();
  void printProgressFooter();

  printProgress();
  printProgressFooter();  
  */
  
  rprintf("\n");
  rprintf("\n");
  rprintf("SEGMENTATION FAULT\n");
  rprintf("\n");
  print_stats();
  exit(3); 
}

void parseOptions(int num,char** argv)
{
  //skip the cnf file in argv[0]
  for(int i=1;i<num;i++){
    char* arg = argv[i];

    //time out
    if(!strcmp(arg, "-t")){
      //next arg must be time out
      if(i==num-1){
	printf("Expecting a time-out argument following -t.\n");
	exit(0);
      }
      time_out = atof(argv[++i]);
      //rprintf("Time out set to %.4f seconds\n",time_out);
    }

    //result file (for integration with preprocessor)
    if(!strcmp(arg, "-r")){
      if(i==num-1){
	printf("Expecting a result filename argument following -r.\n");
	exit(0);
      }
      char* fname = argv[++i];
      res = fopen(fname,"wb");

      if(!res){
	rprintf("Error opening result file %s for writing.\n",fname);
      }
    }

    if(!strcmp(arg, "-s")){
      //rsat must print out results
      print_result = 1;
    }
  }//end for
}

int main(int argc, char *argv[])
{
  start_t = clock();
  my_m = NULL;
  
  rprintf("Rsat version %0.2f\n",RSAT_VERSION);
  fflush(stdout);
  
  //Knot added 11/6/05
  if(argc<2){
    printUsage();
    
    return 0;
  }
  //end Knot added
  
  char *cnf_fname = argv[1];
  fname = argv[1];
  
  time_out = -1.0;

  parseOptions(argc,argv);
  
  //handle signals
  signal(SIGINT,SIGINT_handler);
  signal(SIGSEGV,SIGSEGV_handler);
  
  //read cnf from .cnf file
  read_cnf(cnf_fname);

  BOOLEAN result;
  if(!my_m->ok){
    result = 0;
  }else{
    result = sat();
  }

  if (result>0) {
    if(print_result){
      rprintf("\n");
      rprintf("\n");
      printf("s SATISFIABLE\n");      
    }else{
      //Knot added 1/18/06 for integration with SatElite
      if(res!=NULL){
	fprintf(res,"SAT\n");
	int size = my_m->vc;
	
	my_lit* cur = my_m->assign;
	for(int i=0;i<size;i++){
	  fprintf(res,"%s%d",(i==0?"":" "),lit_index(cur[i]));
	}
	fprintf(res," 0\n");
      }
    }
    
  }else if(result<0){
    //in the case of timeout, ALWAYS print out solution!
    //because rsat.sh will not call SatElite (what whould we???)
    rprintf("\n");
    rprintf("\n");
    printf("s UNKNOWN\n");
    
  }else{
    if(print_result){
      rprintf("\n");
      rprintf("\n");
      printf("s UNSATISFIABLE\n");
    }else{
    
      //Knot added 1/18/06 for integration with SatElite
      if(res!=NULL){ 
	fprintf(res,"UNSAT\n"); 
      }       
    }
    
    if(my_m->conflict_clause!=NULL){
      free(my_m->conflict_clause->lits);
      free(my_m->conflict_clause);
    }
  }
  
  //Knot added 1/18/06
  if(res!=NULL){
    fclose(res);
  }
  
  print_stats();

  free_cnf_manager();

  end_t = clock();                   //solving time	
  double time_used = (double)(end_t-start_t)/CLOCKS_PER_SEC;

  rprintf("Time: %0.5fs\n",time_used);

  if(result>0){
    exit(10);
  }else if(result==0){
    exit(20);
  }else{
    exit(0);
  }

}

void printUsage()
{
  printf("Usage: rsat <cnf-file-name> {-t <timeout> -r <result-filename> -s}\n");
}
