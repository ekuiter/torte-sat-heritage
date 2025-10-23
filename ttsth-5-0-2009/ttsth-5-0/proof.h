#ifndef PROOF_H
#define PROOF_H

#include <stdio.h>
#include "memutil.h"
#include "assignment.h"
#include "ternarytree.h"
#include "set.h"

#define LINKVAR 1
#define LINKDIRECT 2
#define RESOLVEPROPOSITION 3
#define SUPERSETPROPOSITION 4
#define FALSESET 5
#define ASSERTANSWER 6

extern long int nextResId;

extern FILE *proofFile;

extern void openProofFile ();

extern void closeProofFile(int,int);

extern void printNodeLinks (TernaryTreePtr);
extern void assignNodeIds (TernaryTreePtr);

extern void printProofClause (AssignmentPtr, SetPtr);

extern void printResolution (int variable, int left, int right, int super, SetPtr s);
extern void printSuper (int sub, int super, SetPtr s);

extern void printFalseSet (SetPtr);

void resolveProofClauses (long int,long int, long int, long int);

extern int *setIdToResId;
void assignSetIdToResId (int,int);

extern void noteEndOfUnitClauses();
extern void resetToEndOfUnitClauses();

#endif
