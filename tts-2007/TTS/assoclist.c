#include "assoclist.h"
#include "memutil.h"
#include "counts.h"

AssocListPtr newAssocList()
{
	AssocListPtr result;
	result = checkMalloc (sizeof(AssocList),"AssocList");
	result->numOfElements = 0;
	result->capacity = 10;
	result->keys = (void **)checkMalloc(result->capacity*sizeof(void *),"newAssocListElements");
	result->values = (void **)checkMalloc(result->capacity*sizeof(void *),"newAssocListElements");


	return result;
}

void freeAssocList (AssocListPtr s)
{

	free(s->values);
	free(s->keys);
	free(s);
}

void clearAssocList (AssocListPtr s)
{
	s->numOfElements = 0;
}

void printAssocList (AssocListPtr s)
{
	int i;
	printf ("{");
	for (i=0;i<s->numOfElements;i++)
	{
		if (i>0)
			printf(",");
		printf ("%d->%d",(int)s->keys[i],(int)s->values[i]);
	}
	printf ("}");
}


void addToAssocList (AssocListPtr s, void *k, void *v)
{
	int i, found;


	found = false;
	for (i=0;!found && i<s->numOfElements;i++)
		found = k == s->keys[i];
	if (found)
		i--;
	else
	{
		if (s->numOfElements == s->capacity)
		{
			s->capacity *= 2;
			s->keys = checkRealloc (s->keys, s->capacity*sizeof(void *),"reallocAssocListElements");
			s->values = checkRealloc (s->values, s->capacity*sizeof(void *),"reallocAssocListElements");
		}
		i = s->numOfElements++;
		s->keys[i] = k;
		s->values[i] = v;
	}
	//sortAssocList (s);
}

void incAssocList (AssocListPtr s, void *k)
{
	int i, found;


	found = false;
	for (i=0;!found && i<s->numOfElements;i++)
		found = k == s->keys[i];
	if (found)
		i--;
	else
	{
		if (s->numOfElements == s->capacity)
		{
			s->capacity *= 2;
			s->keys = checkRealloc (s->keys, s->capacity*sizeof(void *),"reallocAssocListElements");
			s->values = checkRealloc (s->values, s->capacity*sizeof(void *),"reallocAssocListElements");
		}
		i = s->numOfElements++;
		s->keys[i] = k;
		s->values[i] = (void *) 0;
	}
	s->values[i] = (void *) ( ((int) s->values[i]) + 1);
	//sortAssocList (s);
}

void * getAssocList (AssocListPtr s, void *k)
{
	int i, found;
	void * result = (void *) 0;


	found = false;
	for (i=0;!found && i<s->numOfElements;i++)
		found = k == s->keys[i];
	if (found)
		result = s->values[i];
	
	return result;
}
