//version 1.04 in effect 9/1/06
#define RSAT_VERSION 2.0

/*********************************
 * flags below were added by Knot
 **********************************/

//Knot added 11/08/05
//whether to verify the solution after solving the instance
#define VERIFY_SOLUTION 0
//whether to print out solution if one is found
//whether to terminate execution after timeout limit
#define TIME_OUT 1
//prints out algorithm trace as the instance is solved
#define TRACE 0
//prints out trace in minisat_conflict_clause
#define LEARNING_BUG 0
//check whether each conflict directed clause is actually implied by the original cnf instance or not (using zchaff)
#define CHECK_LEARNED_CLAUSES 0 //currently this flag will work when the program is compiled with BENCHMARK flag set to 1
//use minisat's algorithm to simplify learned clauses
//#define MINIMIZE_CONFLICT 1
//whether to use the minisat implementation of vsid
//#define MINISAT_VSID 1  //score_inc is periodically increased and normalized 
//whether to use heap to store variable scores
//#define HEAP 1          //this must go with minisat_vsid, we must use heap to store variable scores in order to keep the ordering correct all the time
//for debugging: whether to trigger code to check the intregity of the heap 
#define CHECK_HEAP 0
//whether to time the time used for reading input into the program
#define TIME_READ_INPUT 0
//whether to use the restart scheme like in minisat 
//#define MINISAT_RESTART 1  //start out with low #of conflicts as a threshold, then increase the threshold by a multiplicative constant every time we restart
//whether to set a limit on the number of learned clauses
#define REDUCE_KB 1        //once the limit is reached, clauses are vigorously deleted (~50%)
//whether to remove learned clauses that are satisfied at the top level
#define SIMPLIFY_KB 1
//whether to use a limit on the number of learned clauses before restarting
#define POSTPONE_RESTART 0  //the idea is to make sure we have enough knowledge to carry on to the next restart
//whether to do the new vsid scheme
#define HYPERGRAPH_ORDER 0  //this flag is not supported on Win gcc (only MS compilers)
//whether to remove level-1 resolved literals from the knowledge base
#define SHRINK_CLAUSE 0  //if this is 1, literals that are resolved in level 1 will be removed in simplify
//whether to omit level-1 literals from learned clauses (and possibly in passive resolution)
//#define SKIP_LEVEL1 1
//whether to attempt to resolve clauses as the instance is solved.
#define PASSIVE_RESOLUTION 0 //if this is 1, the solver will consider resolving unit clauses with each of the literal's antecedent (see also global.h)

//#define CONSIDER_REMOVING_ASSERTION_LITERAL 1

#define SIMPLIFY_ORIGINAL_KB 1

//#define PICK_FURTHEST_CLAUSE_FROM_ERF_SCORE 0

#define CHECK_MULTIPLE_BACKTRACK 0

#define BEST_REASON_CRITERION 0

#define SHORTEST_ANTECEDENT 1

#define SAVE_BEST_ANTECEDENT 0
//whether to sometimes do pseudo random variable ordering
#define USE_RANDOM_ORDER 0 //in an attempt to do what minisat does

//#define DEBUG 0

#define COUNT_LEVELS_IN_CONFLICT_CLAUSE 0

#define ADD_WATCHED_CLAUSE_TO_TAIL 1

#define BIAS_INITIAL_SCORE 0

#define RANDOM_INITIAL_ORDER 0

#define TRY_NEGATIVE_PHASE_FIRST 1

#define REMOVE_INITIAL_SCORE 0

#define MEASURE_CLAUSES_INSPECTED_PER_CONFLICT 0

#define OUTPUT_BACKTRACK_DEPTH 0

#define CHECK_UNIT_CLAUSES_AT_THE_END 0

#define ECD 1

#define OUTPUT_CONFLICT_CLAUSE 0

#define CHECK_FOCUS_COMPONENT 0

/******************
 * flags below are specifically for like_minisat scheme
 *****************/
#define NO_PREPROCESS_PURE 1

#define FIND_WATCHED_LITERAL_FROM_BEGINNING 1

#define PRINT_DECISION_VARIABLE 0

#define PRINT_CONFLICT_INFORMATION 0

#define ENQUEUE_UNIT_LITERALS_WHILE_READING 1

#define LIKE_MINISAT 1

#define LIKE_MINISAT_LEVEL2 1

#define KEEP_BINARY_CLAUSES 1

/****************
 * end like_minisat scheme flags
 ****************/

#define BCP_SCORE 0

#define SAVE_PROGRESS 1

#define MEASURE_INVERTED_LITERAL 0 //time consuming!!

#define DECISION_LEVEL_THRESHOLD_RESTART 0

#define SMART_RESTART 0

#define REFINED_RESTART 0

#define CONSERVATIVE_KB_REDUCTION 0

#define APPROXIMATE_COMPONENT 0

#define DELETE_UNUSED_VARS 1

#define DETECT_COMPONENT_SWITCH 0

#define RANDOM_RESTART 0

#define IGNORE_OLD_PROGRESS 0

#define IGNORE_SAVED_PROGRESS_ON_FIRST_DECISION 0

#define MEASURE_SAVE_PROGRESS_STATS 0

#define SELECT_MOST_IMPLIED_PHASE 0

#define HINT_SCORE 0

#define GRADUALLY_INCREMENT_ALPHA 0

#define NONDETERMINISTICALLY_IGNORE_SAVED_PROGRESS 0

#define KEEP_VARIABLE_DECISION_RATIO 0  //if decision ratio is too low, force it

#define DEAL_WITH_CONFLICTING_DECISION_VARIABLE 0 //this is meant to be on top of {save type 6 ignore}

#define FLIP_CLEVEL_VARIABLES 0 //this is meant to be on top of {save type 6 ignore}

#define DETECT_BINARY_CLAUSE 0

#define BUMP_DECISION_VARIABLE_AT_CLEVEL 0

#define BUMP_CLEVEL_VARIABLES 0

#define FOCUS_SCORE 0

#define FLIP_FIRST_DECISION_AFTER_BACKTRACK 0

#define PICK_RANDOM_PHASE_WHEN_IGNORE 0

#define ESTIMATE_SOLUTION 0

#define CHOOSE_DOMINANT_LITERAL_AT_FIRST_DECISION_AFTER_BACKTRACK 0

#define SOLVE_TWICE 0

#define USE_SOLVED_INFORMATION 0

#define INVESTIGATE_SOLVE_TWICE 0

#define USE_LIT_COUNT_TO_BIAS 0

#define RESIZE_ORIGINAL_CLAUSES_ARRAY 1

#define RESIZE_DECISION_LIT_ARRAY 1

#define RESIZE_STACK_AND_SAVE_ARRAYS 1

#define RESIZE_CDC_ARRAY 1

#define MEASURE_MEMORY_USAGE 1  //\\

#define RESIZE_WATCHED_LIST_IN_SET_LITERAL 0  //introduce quite a bit of time penalty

#define PREVENT_STACK_OVERFLOW 0

#define PRIORITIZE_IMPLICATION_QUEUE 1

#define PRIORITIZE_IMPLICATION_QUEUE2 0

#define PRIORITIZE_IMPLICATION_QUEUE3 0

#define PRIORITIZE_IMPLICATION_QUEUE4 0 //initially not so successful

#define MEASURE_MAX_LEARNED_CLAUSE_SIZE 0 //\\

#define AGGRESSIVE_CONFLICT_MINIMIZATION 0

#define NON_RECURSIVE 1

#define FREEZE_PROGRESS 0

#define INVALIDATE_SAVED_PROGRESS 0 //invalidate saved phase, if it's a failed asserted literal

#define REFINED_INVALIDATE 0

#define PROBABILISTICALLY_INVALIDATE 0

#define PREVENT_DOUBLE_DECISIONS 0
                                   
#define MEASURE_WRONG_LEVELS_PER_CONFLICT 0 //\\

#define SAVE_SOLUTION 0

#define FREE_BACKTRACKING 0

#define COUNT_LITERAL_APPEARENCE 0

#define EXONERATE_LITERAL 0

#define GATHER_BLAME_INFORMATION 0

#define NEW_RESCALE_CRITERION 0

#define FLUCTUATION_SCORE 0

#define COMPACT_KB 0

#define PERIODIC_RANDOM_WALK 0

#define POLYNOMIAL_RESTART_LIMIT 0

#define KEEP_BEST_REASON 0

#define KEEP_ALL_REASONS 0

#define PICK_BEST_REASON_ON_THE_FLY 0

#define PICK_RELEVANT_CLAUSE 0

//The following flags were added after 7/10/06

#define UNDO_ALEVEL 0

#define MEASURE_LEARNED_CLAUSE_USEFULNESS 0

#define MINIMIZE_LEVEL_IN_LEARNED_CLAUSE 0

#define FOCUS_ON_LEARNED_CLAUSES 0

#define MAXIMUM_MIN_SCORE 0 //pick the reason whose literal w/ minimum score is the highest.

#define EXTREME_UNIT_DERIVATION 0

#define MINIMIZE_UNSEEN_DOMINANT 0

#define BYPASS_BINARY_CHAIN 0

#define POINT_REASON_TO_HIGH_SCORE_VARIABLE 0

#define FUIP_CANDIDATES 0

#define BYPASS_TO_ANTICIPATED_FUIP_CANDIDATE 0

#define PERIODICALLY_STRENGTHEN_LEARNED_CLAUSES 0

#define MEASURE_STUPID_DECISIONS 0

#define PREVENT_STUPID_DECISIONS 0 //not really effective. only a few decisions are prevented each time

#define DERIVE_MORE_UNIT_CLAUSE 0 //useless, all second unit clauses can be realized by asserting the first one

#define MEASURE_AVERAGE_LEARNED_CLAUSE_SIGNIFICANCE 0

#define CHECK_SUBSUMPTION_OPPORTUNITY 0

#define CA_HYPOTHESIS 0

#define FAST_MUC_DISCOVERY 0

#define HANDLE_STRENGTHENING_CONFLICT 0 // making sure conflict analysis of strengthening conflicts uses the previous learned clause to do resolution

#define TAKE_IN_SOLUTION 0

#if(TAKE_IN_SOLUTION)
#define CATEGORIZE_FLIPPED_LITERALS 0
#define CATEGORIZE_UNDONE_LITERALS 1
#define JUDGE_DECISIONS 1
#endif

#define CATEGORIZE_CONFLICTS 0

#define CATEGORIZE_ASSIGNMENTS 0

#if(CATEGORIZE_ASSIGNMENTS)
//basically, attribute every decision to something (default,saved,etc)
#define CATEGORIZE_DECISIONS 1
#endif

#define CMTF 0

#define CHECK_LEARNED_CLAUSE_SUBSUMPTION 0 //for each clause learned, check how many old clauses it subsumes

#define ORDER_WATCHED_CLAUSES 0

#if(ORDER_WATCHED_CLAUSES)
#define ORDER_WATCHED_CLAUSES_ON_ALL_ASSERTIONS 1
#define UNDO_WHOLE_ALEVEL 0
#define ACTIVE_HEAP 1
#define MIN_CLAUSE_HEAP_MEM_USAGE 0
#define OWC 2
#endif//endif order watched clauses

#define COUNT_SPECIAL_CLAUSES 2 //1 for binary, 2 for binary and ternary

#define MEASURE_WASTED_RESOURCE 0

#define SERIOUS_INVESTIGATION 0

#if(SERIOUS_INVESTIGATION)
#define MAX_CLAUSE_INFLUENCE 3 //the max size of learned clause to include
#endif

#define MEASURE_EARLY_IMPLICATIONS 0

#define MEASURE_EXPRESSIVE_SCORE 0

#define ORDER_LITERALS_BY_IMPLICATIONS 0

#define KEEP_PROOF_TRACE 0

#if(KEEP_PROOF_TRACE)
#define MEASURE_REDUNDANT_LEMMA 0 //for unit and binary learned clauses, for now.
#endif

//this flag must be coupled with analyze_implication_graph_up_to_alevel
#define ADVANCED_PROGRESS_SAVING 0 //1 for ignore, -1 for flip

//////////////////////////////////////
// after 9/20/06
//////////////////////////////////////

#define ASSERTED_DECISION_LITERAL 0 //force all asserted literals to be decision literals

#define ADVANCED_PROGRESS_SAVING_USING_BELIEF 0

#if(ADVANCED_PROGRESS_SAVING_USING_BELIEF)
//////////////////////////////////////////////
//make sure we have no conflicting flags
#define ADVANCED_PROGRESS_SAVING 0 //DO NOT CHANGE THIS FLAG!!! (well, without a lot of thought)
/////////////////////////////////////////////
#define SIMPLIFY_BELIEF_MODEL 0 //simplify belief update calculation to save time (let's face it, it's not accurate anyway)
#define ADJUST_FOR_UNIT_LITERAL 0 //adjust delta by the probability that the unit literal will take the other value
#define BELIEF_LIMIT 1
#define SMOOTH_PROGRESS_SAVING 0 //use non-determinism to determine whether to save a phase or not
#define ALTERNATE_IMPLEMENTATION_FOR_CLEVEL 0 //as of 9/22/06 NOT FUNCTIONING PROPERLY!!!
#define ALTERNATE 1
#endif//endif apsub

#define MEASURE_MISSED_RESOLUTION 0

#define CHECK_RESOLVED_MERGE 0 //check whether any merge literal is resolved in each conflict analysis

#define ALLOW_DECISION_EXPLANATION 0 //allow later unit clause to become an explanation for a decision
//#define REORDER_DECISIONS 1 //reorder decisions if it helps increase the number of reason assignments

#define DO_NOT_ALTER_KB 0 //disables simplify_*kb reduce_kb

#if(DO_NOT_ALTER_KB)
#define DO_NOT_REDUCE_KB 1
#define DO_NOT_SIMPLIFY_KB 0
#define DO_NOT_SIMPLIFY_ORIG_KB 0
#endif//endif do not alter kb

//gives conflict_root information and allow other information to be gathered
#define ANALYZE_IMPLICATION_GRAPH_UP_TO_ALEVEL 0 //analyze implication graph to determine which levels from [alevel+1,clevel] are roots of empty-clause proof

//this flag must be coupled with analyze_implication_graph_up_to_alevel
#define REMAKE_UNRELATED_DECISIONS 0 //remake those decisions that are not related to a conflict (before/after) the assertion

#if(REMAKE_UNRELATED_DECISIONS)
#define RESET_DECISION_STACK_EVERY_CONFLICT 0
#define MOVE_UNRELATED_DECISIONS_ABOVE_ALEVEL 0
#define REMAKE_AFTER_CONFLICT_CHAIN 0
#endif

#define MEASURE_BB_DISCOVERY_LEVEL 0

#define SPECIAL_CARE_FOR_FIRST_DECISION 0

//must be coupled with analyze implication graph up to alevel
#define ADVANCED_PROGRESS_SAVING2 0 //keep a more sophisticated account for each variable

#define SMART_KB_MANAGEMENT 1 //an attempt to manage KB smarter to reduce BCP time and improve the quality of the KB.

#if(SMART_KB_MANAGEMENT)
#define REMOVE_SUBSUMED_CLAUSES 0 //use subsumption information to remove certain clauses
#define SUBSUME_KB 0 //periodically completely clean up kb by removing subsumed clauses
#define REMOVE_INACTIVE_CLAUSES 0 //when time comes, remove clauses that are inactive (have not been unit [often enough])
#define FORCED_KB_REDUCTION 0 //if the number of cc's is greater than a threshold, force a call to reduce_kb every restart
#define IMPROVED_CLAUSE_SORTING_FUNCTION 1
#endif

#define FLIP_PROGRESS 0

#define REFINED_FLIP 0

#define BUMP_BY_IMPLICATIONS 0

#define MAX_SEEN_CARDINALITY 0

#define COUNT_REASONS_IN_CC_DERIVATION 0

#define MULTIPLE_CONFLICTS 0

#define DETECT_HARD_REGION 0

#define SMART_BACKTRACK 0//backtrack to flip certain decisions if implications from those levels involved in too many conflitcs

#define OUTPUT_BT_DEPTH_INFORMATION 0

#define SAVE_PROGRESS_ONLY_ON_HARD_PROBLEM 0 //the value of this flag, if > 0, indicates when the solver should start using progres saving in terms of restart

/**********************************
 * Flags below are after 11/05/06
 ***********************************/

#define LUBY_RESTART 1 //should really go with CONSERVATIVE KB LIMIT INCREASE, because this flag will result in so many restarts

#if(LUBY_RESTART)
#define INCREMENT_LUBY_UNIT 0
#endif//endif luby restart

#define CONSERVATIVE_KB_LIMIT_INCREASE 1 //only increment KB limit in reduce kb

#define PERIODIC_SAVE_PROGRESS (SAVE_PROGRESS && 1) //we need an 'on' and an 'off' criteria

#if(PERIODIC_SAVE_PROGRESS)
/*
 * off_type could be
 *   1: keep saving but don't use it
 *   2: stop saving but keep using the last saved asssignment
 *   3: stop saving, use default heuristic
 */
#define OFF_TYPE 3
#endif


#define BINARY_MERGE_RESOLUTION 0

#define SAVE_PROGRESS_FROM_SHORT_CLAUSES 0 //this number is the upperbound on the clause size to trust

#define ADVANCED_CA_DEBUG 0

//correctness check passed 11/14/06
#define ADVANCED_IMPLICATION_GRAPH_ANALYSIS 0 //part of adnan's idea. keep track of which literals really appear in last IG. Save an assignment IFF it does not appear in last IG.
//correctness check passed 11/15/06
//behavior at restart not yet decided (11/15/06)
#define IGNORE_CAUSING_MINORITY_LITERALS 0 //save an assignment IFF it does not appear in the last IG OR is the dominant phase

#define TRUE_IGNORE_SAVE_PROGRESS 0

#define RECOGNIZE_BINARY_IMPLICATIONS 0

#define CONSERVATIVE_SAVE_PROGRESS 0

#define RESET_RESTART_TH 0
#if(RESET_RESTART_TH)
#define DECREMENT_RESET_PROB 0
#endif

#define FAN_OUT 0

#define APPROXIMATE_CMTF 0

#if(APPRIXIMATE_CMTF)
//#define SAVE_PROGRESS 0 //make sure no conflict
#endif

#define MIX_SP_AND_AC 0 //mixing progress saving and approximate cmtf together

#if(MIX_SP_AND_AC && (!SAVE_PROGRESS || !APPROXIMATE_CMTF))
#define MY_X 0
#define MY_X 1
#endif

/************************************
 *   BEGIN IGNORE_RECENT_PROGRESS
 ************************************/
#define IGNORE_RECENT_PROGRESS 0

#if(IGNORE_RECENT_PROGRESS)
/*
 * IGNORE_CRITERIA could be 
 *  1: ignore if saved value is too recent (keep an additional array)
 *  2: ignore if the decision variable has too high a score (recent conflict)
 *  3: ignore probabilistically (ignore prob. proportional to the time it was saved)
 *  4: ignore probabilistically (ignore prob. proportional to variable score)
 *
 */
#define IGNORE_CRITERIA 1

#if(IGNORE_CRITERIA==1)
//#of conflicts within which to ignore progress
#define IGNORE_TH 10
#endif

#if(IGNORE_CRITERIA==2)
//the minimum fraction of score_inc to ignore progress
#define IGNORE_TH 0.8
#endif

#if(IGNORE_CRITERIA==3)
//#of conflicts within which to ignore progress with probability 1
#define IGNORE_TH 80
#endif

#if(IGNORE_CRITERIA==4)
//the minimum fraction of score_inc above which to ignore with prob. 1
#define IGNORE_TH 1
#endif

#endif//endif ignore_recent_progress

/**********************************
 *   END IGNORE_RECENT_PROGRESS
 **********************************/

#define EXTRACT_GLOBAL_CORE 0

#if(EXTRACT_GLOBAL_CORE)
//the frequency of incrementing gcount
#define GCOUNT_FREQ 0.01 //should be rather small
#endif
