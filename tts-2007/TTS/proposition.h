#ifndef PROPOSITION_H

#define PROPOSITION_H



#include "memutil.h"

#include "clause.h"

#include "assignment.h"
#include "list.h"


extern int *mapVariable, *unMapVariable;



typedef struct



{



	int numOfClauses, maxVariable, originalNumOfVariables, capacity;



	ClausePtr *clauses;

#ifdef PROOF
	int falsifyingClauseNum;
#endif

} Proposition, *PropositionPtr;



	

extern int usedCount;

extern PropositionPtr newProposition();

extern double propositionListSize (ListPtr list);

extern int countUsed (PropositionPtr p, int maxVariable, int *globalUsed);

extern int propositionVarCount (PropositionPtr proposition);

extern int propositionListVarCount (ListPtr list, int variable);

extern int propositionContains (PropositionPtr proposition, int variable);

extern double propositionListCount (ListPtr list, int variable,PropositionPtr trunk);

extern PropositionPtr removePropositionDuplicates (PropositionPtr p);
extern PropositionPtr removePropositionTautologies (PropositionPtr p);
extern PropositionPtr removePropositionSingletons (PropositionPtr p, int maxVariable,
											int *mapVariable, int *unMapVariable, int *used,
											int *singletonUsage, int *isSingleton,
											int *unitClauseUsage, int *isUnitClause);

extern PropositionPtr minimizeProposition (PropositionPtr p);

extern PropositionPtr mapProposition (PropositionPtr p, int *map);

extern PropositionPtr sortedCopyProposition (PropositionPtr p);

extern void sortClauses (PropositionPtr p);

extern void freeProposition(PropositionPtr proposition);

extern AssignmentPtr addSingletons (AssignmentPtr oldAssign);

extern int equalPropositions (PropositionPtr p1, PropositionPtr p2);

extern int addToPropositionList (ListPtr l, PropositionPtr p);

extern void addPropositionClause (PropositionPtr proposition, ClausePtr clause);

extern void initPropositions();
extern PropositionPtr trueProposition, falseProposition;

extern PropositionPtr propositionExcluding (PropositionPtr p, int v);

extern PropositionPtr propositionCurried (PropositionPtr p, int v, int positive);  
extern PropositionPtr reduceProposition (PropositionPtr proposition, int variable, int val);

extern void freePropositionList (ListPtr list);



extern int getPropositionSize(PropositionPtr proposition);

PropositionPtr minimizeProposition (PropositionPtr p);





extern char *toStringProposition(PropositionPtr proposition);


extern int evaluateProposition (PropositionPtr proposition, AssignmentPtr assignment);



extern PropositionPtr readProposition (FILE *f);

extern PropositionPtr make3SAT (PropositionPtr p);



#endif
