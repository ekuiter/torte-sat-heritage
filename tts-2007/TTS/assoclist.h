#ifndef ASSOCLIST_H
#define ASSOCLIST_H


typedef struct AssocListStruct
{
	void **keys;
	void **values;
	int capacity, numOfElements;
} AssocList, *AssocListPtr;

extern AssocListPtr newAssocList();
extern void printAssocList (AssocListPtr s);

void addToAssocList (AssocListPtr s, void *k, void *p);
void incAssocList (AssocListPtr s, void *k);
void * getAssocList (AssocListPtr s, void *k);

extern void freeAssocList (AssocListPtr s);
extern void clearAssocList (AssocListPtr s);



#endif
