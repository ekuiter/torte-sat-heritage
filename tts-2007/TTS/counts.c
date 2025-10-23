#include "counts.h"
#include "memutil.h"

#include <stdio.h>

#ifdef COUNT

#include "bignat.h"

BigNatPtr *counts;
int numOfCounts;

void doInitCounts (int n)
{
	int i;
	initBigNat();
	numOfCounts = n;
	counts = checkMalloc (n*sizeof (BigNatPtr), "counts");
	for (i=0;i<numOfCounts;i++)
		counts[i] = newBigNat(0);
}

void doCount (int c)
{
	incBigNat(counts[c]);
}

void doUnCount (int c)
{
	decBigNat(counts[c]);
}

char *getCountString(int c)
{
	static char buffer[100];
	strcpy (buffer, toStringBigNat (counts[c]));
	return buffer;
}

char *getMaxCountString()
{
	static char buffer[100];
	int i;
	BigNatPtr maxCount;
	maxCount = newBigNat(0);
	for (i=0;i<numOfCounts;i++)
		if (lessThanBigNatBigNat (maxCount, counts[i]))
			maxCount = counts[i];
	strcpy (buffer, toStringBigNat (maxCount));
	return buffer;
}


#endif
