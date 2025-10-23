#include "bag.h"
#include "memutil.h"
#include "counts.h"

BagPtr newBag()
{
	BagPtr result;

	
	result = checkMalloc (sizeof(Bag),"Bag");
	result->numOfElements = 0;
	result->capacity = 10;
	result->elements = (void **)checkMalloc(result->capacity*sizeof(void *),"newBagElements");
	result->counts = (int *)checkMalloc(result->capacity*sizeof(int *),"newBagElements");
	
	


	return result;
}

void freeBag (BagPtr b)
{

	free(b->elements);
	free(b->counts);
	free(b);
}

void clearBag (BagPtr b)
{
	b->numOfElements = 0;
}

void printBag (BagPtr b)
{
	int i;
	printf ("{");
	for (i=0;i<b->numOfElements;i++)
	{
		if (i>0)
			printf(",");
		printf ("%d(%d)",(int)b->elements[i],b->counts[i]);
	}
	printf ("}");
}




int bagCountOfU (BagPtr b, void *p)
{
	int i,result;

	{
		result = false;
		for (i=0;!result&&i<b->numOfElements;i++)
			result = b->elements[i] == p;
		if (result)
			return b->counts[i];
		else
			return 0;
	}
}


void addToBag (BagPtr b, void *p)
{
	int i, found;


	found = false;
	for (i=0;!found && i<b->numOfElements;i++)
		found = p == b->elements[i];
	if (found)
		b->counts[i-1]++;
	else
	{
		if (b->numOfElements == b->capacity)
		{
			b->capacity *= 2;
			b->elements = checkRealloc (b->elements, b->capacity*sizeof(void *),"reallocBagElements");
			b->counts = checkRealloc (b->counts, b->capacity*sizeof(void *),"reallocBagElements");
			for (i=b->numOfElements;i<b->capacity;i++)
				b->counts[i] = 0;
		}
		b->counts[b->numOfElements] = 1;
		b->elements[b->numOfElements++] = p;
	}
	//sortBag (s);
}
