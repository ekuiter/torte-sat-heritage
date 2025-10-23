#include "set.h"
#include "memutil.h"


SetPtr newSet()
{
	SetPtr result;
	result = checkMalloc (sizeof(Set),"Set");
	result->numOfElements = 0;
	result->capacity = 10;
	result->elements = (void **)checkMalloc(result->capacity*sizeof(void *),"newSetElements");


	return result;
}



void reverseSet (SetPtr s)
{
	int i,j;
	void *temp;
	
	i=0;j=s->numOfElements-1;
	while (i<j)
	{
		temp = s->elements[i];
		s->elements[i] = s->elements[j];
		s->elements[j] = temp;
		i++;
		j--;
	}
}
void freeSet (SetPtr s)
{

	free(s->elements);
	free(s);
}

void clearSet (SetPtr s)
{
	s->numOfElements = 0;
}

void printSet (SetPtr s)
{
	int i;
	printf ("{");
	for (i=0;i<s->numOfElements;i++)
	{
		if (i>0)
			printf(",");
		printf ("%d",(int)s->elements[i]);
	}
	printf ("}");
}

void sortSet (SetPtr s)
{
	qsort(&s->elements[0], s->numOfElements, sizeof (void *), ptrCompare);
}

void removeDuplicatesSet (SetPtr s)
{
	int i1,i2;
	i1 = 0;
	i2 = 0;
	sortSet (s);
	while (i2 < s->numOfElements)
	{
		while (i2 < s->numOfElements-1 && s->elements[i2] == s->elements[i2+1])
		{
			i2++;
		}
		s->elements[i1++] = s->elements[i2++];
	}
	s->numOfElements = i1;
}

void sortSetCompare (SetPtr s, Comparison c)
{
	qsort(&s->elements[0], s->numOfElements, sizeof (void *), c);
}

int setMemberOf (SetPtr s, void *p)
{
	int i,result, l,r,x;
	if (s->numOfElements > 6)
	{
		l=0; r=s->numOfElements-1;
		while (r >= l)
		{
			x = (l+r)/2;
			if (p < s->elements[x])
				r = x-1;
			else
				l = x+1;
			if (p == s->elements[x])
				return true;
		}
		return false;
	}
	else
	{
		result = false;
		for (i=0;!result&&i<s->numOfElements&&s->elements[i]<=p;i++)
			result = s->elements[i] == p;
		return result;
	}
}

int setMemberOfU (SetPtr s, void *p)
{
	int i,result;

	{
		result = false;
		for (i=0;!result&&i<s->numOfElements;i++)
			result = s->elements[i] == p;
		return result;
	}
}

int setMemberOfRemoveU (SetPtr s, void *p)
{
	int i,result;

	result = false;
	for (i=0;i<s->numOfElements&&!result;i++)
	{
			result = s->elements[i] == p;
	}
	if (result)
	{
		for (i=i-1;i<s->numOfElements-1;i++)
			s->elements[i] = s->elements[i+1];
		s->numOfElements--;
	}

	return result;
}

int setMemberOfRemove (SetPtr s, void *p)
{
	int i,result;

	result = false;
	for (i=0;i<s->numOfElements&&s->elements[i]<=p;i++)
	{
			result = s->elements[i] == p;
	}
	if (result)
	{
		for (i=i-1;i<s->numOfElements-1;i++)
			s->elements[i] = s->elements[i+1];
		s->numOfElements--;
	}

	return result;
}


void copySet (SetPtr s1, SetPtr s2)
{
	int i;
	clearSet (s1);
	for (i=0;i<s2->numOfElements;i++)
		addToSetNoCheck (s1, s2->elements[i]);
}

SetPtr newCopySet (SetPtr s2)
{
	SetPtr s1;
	int i;
	s1 = newSet();
	for (i=0;i<s2->numOfElements;i++)
		addToSetNoCheck (s1, s2->elements[i]);
	return s1;
}


int equalSet (SetPtr s1, SetPtr s2)
{
	int result, i;
	
	result = false;
	if (s1->numOfElements == s2->numOfElements)
	{
		result = true;
		for (i=0;i<s1->numOfElements && result; i++)
			if (!setMemberOfU (s2, s1->elements[i]))
				result = false;
	}
	
	return result;
}

int equalSortedSet (SetPtr s1, SetPtr s2)
{
	int result, i;
	
	result = false;
	if (s1->numOfElements == s2->numOfElements)
	{
		result = true;
		for (i=0;i<s1->numOfElements && result; i++)
			if (s1->elements[i] != s2->elements[i])
				result = false;
	}
	
	return result;
}

int subsetSet (SetPtr s1, SetPtr s2)
{
	int result, i;
	
	result = true;
	for (i=0;i<s1->numOfElements && result; i++)
		if (!setMemberOfU (s2, s1->elements[i]))
			result = false;
	
	return result;
}

int subsetSortedSet (SetPtr s1, SetPtr s2)
{
	int result, i, j;
	result = true;
	i = 0;
	j = 0;
	while (result && i<s1->numOfElements && j < s2->numOfElements)
	{
		if (s1->elements[i] == s2->elements[j])
		{
			i++; j++;
		}
		else if (s1->elements[i] < s2->elements[j])
		{
			result = false;
		}
		else
		{
			j++;
		}
	}
	if (i < s1->numOfElements)
		result = false;
		
	return result;
}

void unionSet (SetPtr result, SetPtr s1, SetPtr s2)
{
	int i;
	clearSet (result);
	for (i=0;i<s1->numOfElements;i++)
		addToSetNoCheck (result, s1->elements[i]);
	for (i=0;i<s2->numOfElements;i++)
		addToSet (result, s2->elements[i]);
}

void unionToSet (SetPtr result, SetPtr s1)
{
	int i;
	for (i=0;i<s1->numOfElements;i++)
		addToSet (result, s1->elements[i]);
}



void differenceToSet (SetPtr result, SetPtr s1)
{
	int i;
	for (i=0;i<s1->numOfElements;i++)
		setMemberOfRemoveU (result, s1->elements[i]);
}

void intersectSet (SetPtr result, SetPtr s1, SetPtr s2)
{
	int i;
	clearSet (result);
	for (i=0;i<s1->numOfElements;i++)
		if (setMemberOfU (s2, s1->elements[i]))
			addToSetNoCheck (result, s1->elements[i]);
}
void intersectToSet (SetPtr result, SetPtr s1)
{
	int i;
	for (i=0;i<result->numOfElements;)
		if (setMemberOfU (s1, result->elements[i]))
			i++;
		else
			setMemberOfRemoveU (result, result->elements[i]);
}
int isSorted(SetPtr s)
{
	int ok,i;
	ok =true;
	
	for (i=0;ok&&i<s->numOfElements-1;i++)
		ok = s->elements[i] < s->elements[i+1];
		
	return ok;
}
int addToSet (SetPtr s, void *p)
{
	int i, found;


	found = false;
	for (i=0;!found && i<s->numOfElements;i++)
		found = p == s->elements[i];
	if (!found)
	{
		if (s->numOfElements == s->capacity)
		{
			s->capacity *= 2;
			s->elements = checkRealloc (s->elements, s->capacity*sizeof(void *),"reallocSetElements");
		}
		s->elements[s->numOfElements++] = p;
	}
	return !found;
}

void addToStart (SetPtr s, void *p)
{
	int i;
		if (s->numOfElements == s->capacity)
		{
			s->capacity *= 2;
			s->elements = checkRealloc (s->elements, s->capacity*sizeof(void *),"reallocSetElements");
		}
	for (i=s->numOfElements;i>=1;i--)
		s->elements[i] = s->elements[i-1];
	s->elements[0] = p;
	s->numOfElements++;

}
int sortedSetCompare (SetPtr a, SetPtr b)
{
	int i;
	
	for (i=0; i<a->numOfElements && i<b->numOfElements && a->elements[i] == b->elements[i];i++);
	
	if (i == a->numOfElements && i == b->numOfElements)
		return 0;
	else if ( (i == a->numOfElements) || (i < b->numOfElements && a->elements[i] < b->elements[i]) )
		return -1;
	else
		return 1;
}

void addToSortedSet (SetPtr s, void *p)
{
	int i,j, found;


	found = false;
	for (i=0;i<s->numOfElements && s->elements[i] < p;i++);
	if (i<s->numOfElements)
		found = p == s->elements[i];
	if (!found)
	{
		if (s->numOfElements == s->capacity)
		{
			s->capacity *= 2;
			s->elements = checkRealloc (s->elements, s->capacity*sizeof(void *),"reallocSetElements");
		}
		s->numOfElements++;
		for (j=s->numOfElements-1;j>i;j--)
			s->elements[j] = s->elements[j-1];
		s->elements[i] = p;
	}
	//sortSet (s);
}

SetPtr newSingletonSet (void *p)
{
	SetPtr result;
	
	result = newSet();
	addToSet (result, p);
	return result; 
}

int setPos (SetPtr s, void *p)
{
	int i;
	for (i=0;;i++)
		if (s->elements[i] == p)
			return i;
}

int setsDisjoint (SetPtr s1, SetPtr s2)
{
	int result,v;
	result = true;
	for (v=0;result&&v<s1->numOfElements;v++)
		result = !setMemberOfU (s2, s1->elements[v]);
	return result;
}

void addToSetNoCheck (SetPtr s, void *p)
{

	if (s->numOfElements == s->capacity)
	{
		s->capacity *= 2;
		s->elements = checkRealloc (s->elements, s->capacity*sizeof(void *),"reallocSetElements");
	}
	s->elements[s->numOfElements++] = p;
}
