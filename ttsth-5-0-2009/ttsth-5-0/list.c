#include "memutil.h"
#include "list.h"
#include "set.h"


static SetPtr linkBuffer=NULL;

void initList (ListPtr list)
{
	list->length = 0;
	list->first=NULL;
	list->last=NULL;
	
	if (linkBuffer == NULL)
		linkBuffer = newSet();
}


ListPtr newList()
{
	ListPtr result;
	result = checkMalloc (sizeof(List),"List");
	initList (result);
	return result;
}

ListPtr copyList (ListPtr list)
{
	ListPtr result;
	LinkPtr iterator;
	result = newList ();
	iterator = list->first;
	while (iterator != NULL)
	{
		addListLast (result, iterator->element);
		iterator = iterator->next;
	}
	return result;
}

void freeListLinks (ListPtr list)
{
	LinkPtr iterator, nextIterator;
	iterator = list->first;
	while (iterator != NULL)
	{
		nextIterator = iterator->next;
		free (iterator);
		iterator = nextIterator;
	}
	list->first = NULL;
	list->last = NULL;
	list->length = 0;
}

void freeList (ListPtr l)
{
	freeListLinks (l);
	free (l);
}

void addListFirst (ListPtr list, void *element)
{
	LinkPtr link;
	link = checkMalloc (sizeof(Link),"addListFirst");
	link->next = list->first;
	link->previous = NULL;
	link->element = element;
	if (list->last == NULL)
		list->last = link;
	else
		list->first->previous = link;
	list->first = link;
	list->length++; 
}

void addListLast (ListPtr list, void *element)
{
	LinkPtr link;
	if (linkBuffer->numOfElements > 0)
		link = linkBuffer->elements[--linkBuffer->numOfElements];
	else
		link = checkMalloc (sizeof(Link),"addListFirst");
	link->next = NULL;
	link->previous = list->last;
	link->element = element;
	if (list->first == NULL)
		list->first = link;
	else
		list->last->next = link;
	list->last = link;
	list->length++; 
}

void removeList (ListPtr list, LinkPtr link)
{
	if (list->first == link)
		list->first = link->next;
	if (list->last == link)
		list->last = link->previous;
	if (link->previous != NULL)
		link->previous->next = link->next;
	if (link->next != NULL)
		link->next->previous = link->previous;
	list->length--;
	addToSetNoCheck (linkBuffer,link);
}

void insertListBefore (ListPtr list, LinkPtr link, void *element)
{
	LinkPtr newLink;
	newLink = checkMalloc (sizeof(Link),"addListFirst");
	newLink->element = element;
	newLink->previous = link->previous;
	newLink->next = link;
	if (list->first == link)
		list->first = newLink;
	if (link->previous != NULL)
		link->previous->next = newLink;
	link->previous = newLink;
	list->length++; 
	
}

void insertListAfter (ListPtr list, LinkPtr link, void *element)
{
	LinkPtr newLink;
	newLink = checkMalloc (sizeof(Link),"addListFirst");
	newLink->element = element;
	newLink->previous = link;
	newLink->next = link->next;
	if (list->last == link)
		list->last = newLink;
	if (link->next != NULL)
		link->next->previous = newLink;
	link->next = newLink;
	list->length++;
}
void *removeListFirst (ListPtr list)
{
	void *result;
	LinkPtr link;
	link = list->first;
	result = link->element;
	if (link->next == NULL)
	{
		list->first = NULL;
		list->last = NULL;
	}
	else
	{
		link->next->previous = NULL;
		list->first = link->next;
	}
	free (link);
	list->length--;
	return result;
}
void *removeListLast(ListPtr list)
{
	void *result;
	LinkPtr link;
	link = list->last;
	result = link->element;
	if (link->previous == NULL)
	{
		list->first = NULL;
		list->last = NULL;
	}
	else
	{
		link->previous->next = NULL;
		list->last = link->previous;
	}
	free (link);
	list->length--;
	return result;
}

void sortList (ListPtr l, listComparator c)
{
	LinkPtr link;
	void **items;
	int i;
	
	items = (void **)checkMalloc (l->length*sizeof(void *),"");
	
	i=0;
	for (link=l->first;link!=NULL;link=link->next)
		items[i++] = link->element;

	qsort (items, l->length, sizeof (void *), c);
		
		
	i=0;
	for (link=l->first;link!=NULL;link=link->next)
		link->element = items[i++];
		
	free (items);
	
}
