// Actually, a line must contain MAX_LINE_LEN-2 chars at most
#define MAX_LINE_LEN 100000
// Actually, a clause must contain MAX_CLAUSE_LEN-1 literals at most
#define MAX_CLAUSE_LEN 1025
// maximum size of a label string: used for printing clauses, var sets, etc/
#define STRING_LABEL_SIZE 1024

#define MIN(A,B) ( (A)<(B) ? (A) : (B) )
#define MAX(A,B) ( (A)>(B) ? (A) : (B) )
#define KB 1024.0
#define MB 1048576
#define GB 1073741824
//#define rprintf printf //for MS VCC
#define rprintf(format, args...) ( printf("c "), printf(format , ## args), fflush(stdout) )

#define FIRST_RESTART 100
#define INITIAL_RESTART_INCR 100 //how many backtracks to increment by
#define RESTART_INCR_INCR 0 //add to the restart_incr by this amount each time
#define RESTART_MULTIPLIER 1.5

#if(REDUCE_KB)

#if(CONSERVATIVE_KB_LIMIT_INCREASE)
#define MAX_LEARNED_CLAUSES_MULTIPLIER 1.5
#else//else conservative limit
#define MAX_LEARNED_CLAUSES_MULTIPLIER 1.1
#endif//endif conservative limit

#define CLAUSE_SCORE_INC_FACTOR 1.001001001001001001001001001001 //based on minisat (1/0.999)
#define LEARNED_CLAUSE_FACTOR 0.7
#endif

#if(SIMPLIFY_KB)
#define NEXT_SIMPLIFY_INCREMENT 0
#endif

#if(PASSIVE_RESOLUTION)
#define MIN_CLAUSE_SIZE_TO_RESOLVE 14  //minimum clause size to do resolution in set_literal(). This number should be at least 3
#define MIN_RESTART_TO_RESOLVE 0 //minimum number of restarts before we do passive resolution. We don't want to do it if the problem is easy.
#define MAX_RESTART_TO_RESOLVE 1000
#endif

//END added globals

//Knot added 12/6/05
#define DECAY_PERIOD 1000 //number of decision counts before we decay vsid score
#define BUBBLE_STEP 1024 //the size of interval to initially look for the correct variable order 
                         //this number (bubble_step) is used in conflict_analysis.cpp->add_conflict_driven_clause
                         //1024 is the number zchaff uses

#define CD_CLAUSE_COUNT_LIMIT 15
#define AGGRESSIVE_DELETION 6
#define NON_AGGRESSIVE_DELETION 45
#define HEAD_COUNT_CONST 16

#define SCORE_INC_FACTOR 1.052632  //based on minisat
#define SCORE_INC_PARAM 0.95 

#define SCORE_LIMIT 1e100
#define CLAUSE_SCORE_LIMIT 1e20
#define SCORE_DIVIDER 1e-100  //must == 1/score_limit
#define CLAUSE_SCORE_DIVIDER 1e-20 //must == 1/clause_score_limit

#if(USE_RANDOM_ORDER || 1)
#define INIT_RANDOM_SEED 91648253 //from minisat
#define RANDOM_VAR_FREQ 0.02 //from minisat
#endif
//end Knot added

#define BOOLEAN_NULL -1
#define DEC -1
#define RECENT_THRESHOLD 0.01
#define SAVE_PROGRESS_THRESHOLD 0.01
#define MIN_LEVEL_TO_RESTART 10
#define CONSERVATIVE_KB_REDUCTION_FACTOR 0.5
#define SAVE_PROGRESS_RECENTNESS_THRESHOLD 0.4  //the higher this number, the more we save
#define RANDOM_RESTART_TIME_LIMIT 300  //if the time elapsed is > 600 seconds, the next restart is random restart
#define NUM_CONFLICTS_TO_CONSIDER_PROGRESS_OLD 1000
#define NUM_DECISIONS_TO_IGNORE 1
#define NUM_IMPLIED_THRESHOLD 1000
#define RATIO_THRESHOLD 2
#define INITIAL_ALPHA 1
#define MIN_RESTART_TO_INCREMENT_ALPHA 12
#define ALPHA_INCREMENT 0.1
#define IGNORE_SAVED_PROGRESS_FREQ 0.05
#define DECISION_RATIO_UPPER_LIMIT 50
#define DECISION_RATIO_LOWER_LIMIT 0.02
#define P_ESTIMATE_SOLUTION_THRESHOLD 0.9
#define N_ESTIMATE_SOLUTION_THRESHOLD 0.1
#define VC_THRESHOLD 100000
#define MAX_DECISION_LEVEL_THRESHOLD 10000
#define CONFLICTS_INVOLVED_THRESHOLD 100

#define VERY_LARGE_LEVEL_THRESHOLD 4
#define INIT_MAX_LEARNED_LITERALS 200000
#define MIN_DECISION_LEVEL_TO_COMPACT_KB 20
#define NUM_LARGE_LEVELS_THRESHOLD 0

#define PASSIVE_RESOLUTION_CDL_THRESHOLD 5000

#define MAX_RANDOM_STEPS 800
#define MAX_UNSAT_CLAUSES_TO_PERFORM_RANDOM_WALK 300

#define LOW_PROFILE_FACTOR 10
#define HIGH_PROFILE_FACTOR 15

#define CDC_REMOVE_PROB 1
#define SKIP_SCORE_UPDATE_PROB 0.9

#define NUM_FOCUS_CLAUSES 50
#define INITIAL_NEXT_SWITCH 3
#define NEXT_SWITCH_INCREMENT 3

#define UNSEEN_LOWER_LEVEL_PENALTY 1000000 //500
#define SEEN_LOWER_LEVEL_PENALTY 10000  //105
#define CLEVEL_PENALTY 20
#define CONFLICT_BONUS 100

#if(REMOVE_SUBSUMED_CLAUSES)
#define INIT_MAX_CLAUSE_SIZE 3 //good
#endif

#if(SMART_BACKTRACK)
#define INIT_CONFLICT_LIMIT 100
#endif

#if(LUBY_RESTART)
#define LUBY_UNIT 512
#if(INCREMENT_LUBY_UNIT)
#define LUBY_UNIT_INCREMENT 5
#endif
#endif

#if(PERIODIC_SAVE_PROGRESS)
#define INIT_ON_TH 100
#define INIT_ON_TH_INC 0
#define INIT_OFF_TH 400
#define INIT_OFF_TH_INC 0
#endif

#if(CONSERVATIVE_SAVE_PROGRESS)
#define BT_DEPTH_SAVE_PROGRESS_THRESHOLD 10
#endif

#if(RESET_RESTART_TH)
#define INIT_RESET_PROBABILITY 0.1
#define PROBABILITY_DECREMENT 0.000
#endif
