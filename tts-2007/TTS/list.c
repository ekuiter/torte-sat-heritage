#include "memutil.h"
#include "list.h"

void initList (ListPtr list)
{
	list->length = 0;
	list->first=NULL;
	list->last=NULL;
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
	free (link);
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
