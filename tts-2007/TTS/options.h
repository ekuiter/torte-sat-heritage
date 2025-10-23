#ifndef OPTIONS_H
#define OPTIONS_H

#define FIX_ORDER 0
#define SOLVE 1
#define EXPONENT 2
#define PROOF_CLAUSES 3
#define HELP 4
#define PRINT_PURE_LITERALS 5
#define PRINT_LEVEL_COUNTS 6
#define PRINT_ARRANGEMENT 7
#define MERGE_CLAUSES 8
#define PRE_FILL 9
#define TRY_BRANCH 10
#define DYNAMIC_SINGLETONS 11
#define PRINT_TRACE 12
#define MAX_BOOLEAN_OPTIONS 13
int *booleanOptions;


#define PERMUTATION_IN 0
#define PERMUTATION_OUT 1
#define PROOF_LENGTH_FILE 2
#define PRINT_REDUCED 3
#define DOT_FILE 4
#define STATIC_DOT_FILE 5
#define MAX_STRING_OPTIONS 6
char **stringOptions;

#define PERMUTATION_SEED 0
#define SIMULATED_ANNEALING_DIVISOR 1
#define PERMUTATION_METHOD 2
#define PROOF_FORMAT 3
#define REHEAT_COUNT 4
#define MAX_INT_OPTIONS 5
int *intOptions;

extern void getOptions (int *argc, char * argv[]);

#endif
