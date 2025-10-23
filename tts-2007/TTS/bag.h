#ifndef BAG_H
#define BAG_H

typedef struct BagStruct
{
	void **elements;
	int *counts;
	int capacity, numOfElements;
} Bag, *BagPtr;

extern BagPtr newBag();
void addToBag (BagPtr s, void *p);

extern void freeBag (BagPtr b);
extern void clearBag (BagPtr b);

extern int bagCountOfU (BagPtr b, void *p);

#endif
