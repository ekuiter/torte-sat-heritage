#ifndef PERMUTE_H
#define PERMUTE_H

#include "set.h"
#include "proposition.h"
#include "clause.h"

typedef struct SetItemStruct
{
	int score;
	SetPtr set, activeClauses;
} SetItem, *SetItemPtr;

extern void findPermutation (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute);
extern void readPermutation (int numOfVariables, int *permute, int *unPermute);

#endif
