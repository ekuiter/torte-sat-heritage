#ifndef CLAUSE_H
#define CLAUSE_H
#include "memutil.h"
#include "assignment.h"
#include "set.h"
#include "intset.h"

typedef struct

{

	int numOfVariables, maxVariable;

	int *variables,*positive;

#ifdef PROOF
	int resId;
	//IntSetPtr originalVariables;
#endif

} Clause, *ClausePtr;


int evalCount;
extern ClausePtr trueClause, falseClause;

extern void initClauses();   
extern void sortClauseSet (SetPtr c);
extern int equalClauseSets (SetPtr s1, SetPtr s2);

ClausePtr clauseCurried (ClausePtr c, int v, int positive);
int clauseContains (ClausePtr c, int v);
extern void removeClauseDuplicates (ClausePtr c);
extern void removeSortedClauseDuplicates (ClausePtr c);
extern ClausePtr newClause()
;
int isClauseTautology (ClausePtr clause);
extern void freeClause(ClausePtr clause);
extern void freeClauseSet (SetPtr s);

extern ClausePtr sortedCopyClause(ClausePtr clause);
extern ClausePtr newCopyClause(ClausePtr clause);
extern void sortClause(ClausePtr clause);

extern int minVariable(ClausePtr clause);
extern void removeClauseVariable (ClausePtr clause, int variable);
extern void addClauseVariableValue (ClausePtr clause, int variable, int positive);

extern int equalClauses (ClausePtr clause1, ClausePtr clause2);

extern int clauseCountV (ClausePtr clause, int variable);

extern int equalClausesExcept (ClausePtr clause1, ClausePtr clause2, int v);

extern ClausePtr mapClause (ClausePtr clause, int *map);

extern int clauseCompare (const void *c1, const void *c2);

extern int oneVarSatisfiesClause (ClausePtr clause, int variable, int positive)
;
	
extern ClausePtr reduceClause (ClausePtr clause, int variable, int val);

extern int clauseSize (ClausePtr clause);

extern void addClauseVariable (ClausePtr clause, int variable)
;


extern int evaluateClause (ClausePtr clause, AssignmentPtr assignment);
extern int evaluateUnMappedClause (ClausePtr clause, AssignmentPtr assignment);

extern char *toStringClause(ClausePtr clause)
;

extern ClausePtr readClause (FILE *f);

extern ClausePtr minimizeClause (ClausePtr);
#endif
