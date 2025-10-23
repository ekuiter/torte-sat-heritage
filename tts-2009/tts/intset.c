#include "intset.h"
#include "memutil.h"

IntSetPtr newIntSet()
{
	IntSetPtr result;
	result = checkMalloc (sizeof(IntSet),"IntSet");
	result->numOfElements = 0;
	result->capacity = 1;
	result->elements = (int *)checkMalloc(result->capacity*sizeof(int),"newIntSetElements");



	return result;
}

void freeIntSet (IntSetPtr s)
{

	free(s->elements);
	free(s);
}

void clearIntSet (IntSetPtr s)
{
	s->numOfElements = 0;
}

IntSetPtr newIntSetCapacity(int capacity)
{
	IntSetPtr result;
	result = checkMalloc (sizeof(IntSet),"IntSet");
	result->numOfElements = 0;
	result->capacity = capacity;
	result->elements = (int *)checkMalloc(result->capacity*sizeof(int),"newIntSetElements");


	return result;
}


void unionToSortedIntSet (IntSetPtr result, IntSetPtr s1)
{
	int i,j,k;
	IntSetPtr temp;
	temp = newIntSetCapacity (result->numOfElements + s1->numOfElements);
	
	i=0;
	j=0;
	k=0;
	//printIntSet (result); printIntSet (s1);
	while (i<result->numOfElements && j < s1->numOfElements)
	{
		while (i<result->numOfElements && result->elements[i] < s1->elements[j])
			temp->elements[k++] = result->elements[i++];
		if (i<result->numOfElements)
		{
			while (j < s1->numOfElements && s1->elements[j] < result->elements[i])
				temp->elements[k++] = s1->elements[j++];
		}
		while (i<result->numOfElements && j < s1->numOfElements && result->elements[i] == s1->elements[j])
		{
			temp->elements[k++] = result->elements[i++];
			j++;
		}
	}
	for (i=i; i<result->numOfElements;i++)
		temp->elements[k++] = result->elements[i];
	for (j=j; j<s1->numOfElements;j++)
		temp->elements[k++] = s1->elements[j];
		
	temp->numOfElements = k;
	
	copyIntSet (result, temp);
	freeIntSet (temp);
	//printIntSet (result); printf("\n");
	
}
void printIntSet (IntSetPtr s)
{
	int i;
	printf ("{");
	for (i=0;i<s->numOfElements;i++)
	{
		if (i>0)
			printf(",");
		printf ("%d",s->elements[i]);
	}
	printf ("}");
}

void sortIntSet (IntSetPtr s)
{
	qsort(&s->elements[0], s->numOfElements, sizeof (int), intCompare);
}

void removeDuplicatesIntSet (IntSetPtr s)
{
	int i1,i2;
	i1 = 0;
	i2 = 0;
	sortIntSet (s);
	while (i2 < s->numOfElements)
	{
		while (i2 < s->numOfElements-1 && s->elements[i2] == s->elements[i2+1])
			i2++;
		s->elements[i1++] = s->elements[i2++];
	}
	s->numOfElements = i1;
}


int intSetMemberOf (IntSetPtr s, int p)
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

int intSetMemberOfU (IntSetPtr s, int p)
{
	int i,result;

	{
		result = false;
		for (i=0;!result&&i<s->numOfElements;i++)
			result = s->elements[i] == p;
		return result;
	}
}

int intSetMemberOfRemoveU (IntSetPtr s, int p)
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

int intSetMemberOfRemove (IntSetPtr s, int p)
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


void copyIntSet (IntSetPtr s1, IntSetPtr s2)
{
	int i;
	clearIntSet (s1);
	for (i=0;i<s2->numOfElements;i++)
		addToIntSetNoCheck (s1, s2->elements[i]);
}

IntSetPtr newCopyIntSet (IntSetPtr s2)
{
	IntSetPtr s1;
	int i;
	s1 = newIntSet();
	for (i=0;i<s2->numOfElements;i++)
		addToIntSetNoCheck (s1, s2->elements[i]);
	return s1;
}



int equalIntSet (IntSetPtr s1, IntSetPtr s2)
{
	int result, i;
	
	result = false;
	if (s1->numOfElements == s2->numOfElements)
	{
		result = true;
		for (i=0;i<s1->numOfElements && result; i++)
			if (!intSetMemberOfU (s2, s1->elements[i]))
				result = false;
	}
	
	return result;
}

int equalSortedIntSet (IntSetPtr s1, IntSetPtr s2)
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

int subsetIntSet (IntSetPtr s1, IntSetPtr s2)
{
	int result, i;
	
	result = true;
	for (i=0;i<s1->numOfElements && result; i++)
		if (!intSetMemberOfU (s2, s1->elements[i]))
			result = false;
	
	return result;
}

int subsetSortedIntSet (IntSetPtr s1, IntSetPtr s2)
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

void unionIntSet (IntSetPtr result, IntSetPtr s1, IntSetPtr s2)
{
	int i;
	clearIntSet (result);
	for (i=0;i<s1->numOfElements;i++)
		addToIntSetNoCheck (result, s1->elements[i]);
	for (i=0;i<s2->numOfElements;i++)
		addToIntSet (result, s2->elements[i]);
}

void unionToIntSet (IntSetPtr result, IntSetPtr s1)
{
	int i;
	for (i=0;i<s1->numOfElements;i++)
		addToIntSet (result, s1->elements[i]);
}

void differenceToIntSet (IntSetPtr result, IntSetPtr s1)
{
	int i;
	for (i=0;i<s1->numOfElements;i++)
		intSetMemberOfRemoveU (result, s1->elements[i]);
}

void intersectIntSet (IntSetPtr result, IntSetPtr s1, IntSetPtr s2)
{
	int i;
	clearIntSet (result);
	for (i=0;i<s1->numOfElements;i++)
		if (intSetMemberOfU (s2, s1->elements[i]))
			addToIntSetNoCheck (result, s1->elements[i]);
}
void intersectToIntSet (IntSetPtr result, IntSetPtr s1)
{
	int i;
	for (i=0;i<result->numOfElements;)
		if (intSetMemberOfU (s1, result->elements[i]))
			i++;
		else
			intSetMemberOfRemoveU (result, result->elements[i]);
}


void addToIntSet (IntSetPtr s, int p)
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
			s->elements = checkRealloc (s->elements, s->capacity*sizeof(int),"reallocIntSetElements");
		}
		s->elements[s->numOfElements++] = p;
	}
	//sortIntSet (s);
}

int sortedIntSetCompare (IntSetPtr a, IntSetPtr b)
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

void addToSortedIntSet (IntSetPtr s, int p)
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
			s->elements = checkRealloc (s->elements, s->capacity*sizeof(int),"reallocIntSetElements");
		}
		s->numOfElements++;
		for (j=s->numOfElements-1;j>i;j--)
			s->elements[j] = s->elements[j-1];
		s->elements[i] = p;
	}
	//sortIntSet (s);
}

IntSetPtr newSingletonIntSet (int p)
{
	IntSetPtr result;
	
	result = newIntSet();
	addToIntSet (result, p);
	return result; 
}

int intSetPos (IntSetPtr s, int p)
{
	int i;
	for (i=0;;i++)
		if (s->elements[i] == p)
			return i;
}

int intSetsDisjoint (IntSetPtr s1, IntSetPtr s2)
{
	int result,v;
	result = true;
	for (v=0;result&&v<s1->numOfElements;v++)
		result = !intSetMemberOfU (s2, s1->elements[v]);
	return result;
}

void addToIntSetNoCheck (IntSetPtr s, int p)
{

	if (s->numOfElements == s->capacity)
	{
		s->capacity *= 2;
		s->elements = checkRealloc (s->elements, s->capacity*sizeof(void *),"reallocIntSetElements");
	}
	s->elements[s->numOfElements++] = p;
}

void intPush (IntSetPtr s, int p)
{
	addToIntSetNoCheck (s,p);
}

int intPop (IntSetPtr s)
{
	return s->elements[--s->numOfElements];
}

int intTop (IntSetPtr s)
{
	return s->elements[s->numOfElements-1];
}
