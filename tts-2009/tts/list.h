#ifndef LIST_H
#define LIST_H

typedef struct LinkStruct
{
	void *element;
	struct LinkStruct *next,*previous;
} Link,*LinkPtr;

typedef struct
{
	LinkPtr first,last;
	long length;
} List, *ListPtr;

typedef int listComparator (const void *a,const void *b);

extern void freeListLinks (ListPtr list);
extern void freeList (ListPtr l);

extern ListPtr newList();

extern void initList (ListPtr list);
extern void addListFirst (ListPtr list, void *element);
extern void addListLast (ListPtr list, void *element);
extern void insertListBefore (ListPtr list, LinkPtr link, void *element);
extern void insertListAfter (ListPtr list, LinkPtr link, void *element);
extern ListPtr copyList (ListPtr list);
extern void removeList (ListPtr list, LinkPtr link);
extern void *removeListFirst (ListPtr list);
extern void *removeListLast (ListPtr list);
extern void sortList (ListPtr list, listComparator c);

#endif
