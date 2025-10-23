#include "hashtable.h"
#include "memutil.h"


HashTablePtr newHashTable (HashFunction hashFunction, HashKeyEqual hashKeyEqual, HashKeyFree hashKeyFree,
							HashKeyPrint hashKeyPrint, HashKeyActive hashKeyActive)
{
	HashTablePtr result;
	int i;

	result = (HashTablePtr) checkMalloc (sizeof(HashTable),"Hash table");
	result->hashFunction = hashFunction;
	result->hashKeyEqual = hashKeyEqual;
	result->hashKeyFree = hashKeyFree;
	result->hashKeyPrint = hashKeyPrint;
	result->hashKeyActive = hashKeyActive;

	result->currentNumOfSlots = 3;
	result->previousNumOfSlots = 1;
	result->currentSlots = (HashLinkPtr *)checkMalloc (result->currentNumOfSlots*sizeof(HashLinkPtr),"Hash slots");
	result->previousSlots = (HashLinkPtr *)checkMalloc (result->previousNumOfSlots*sizeof(HashLinkPtr),"Hash slots");

	for (i=0;i<result->currentNumOfSlots;i++)
		result->currentSlots[i] = NULL;
	for (i=0;i<result->previousNumOfSlots;i++)
		result->previousSlots[i] = NULL;

	result->currentNumOfKeys = 0;
	result->previousNumOfKeys = 0;
	result->currentRehashSlot = 0;
	result->currentRehashLink = NULL;

	return result;
}

void printHashTable (HashTablePtr hashTable)
{
	int i;
	HashLinkPtr link;
	printf ("id=%d current keys = %d previous keys = %d rehash slot = %d\n",
				(int)hashTable, hashTable->currentNumOfKeys, hashTable->previousNumOfKeys, hashTable->currentRehashSlot);
	printf ("Previous\n");
	for (i=0;i<hashTable->previousNumOfSlots;i++)
	{
		printf ("Slot %d", i);
		for (link=hashTable->previousSlots[i];link!=NULL;link=link->next)
			printf ("->%d ", (int)link->item);
		printf ("-> NULL\n");
	}
	printf ("Current\n");
	for (i=0;i<hashTable->currentNumOfSlots;i++)
	{
		printf ("Slot %d", i);
		for (link=hashTable->currentSlots[i];link!=NULL;link=link->next)
			printf ("->%d ", (int)link->item);
		printf ("-> NULL\n\n");
	}
}

void clearHashSlots (HashLinkPtr *slots, int numOfSlots, HashKeyFree hashKeyFree)
{
	int i;
	HashLinkPtr link, nextLink;


	for (i=0;i<numOfSlots;i++)
	{
		for (link=slots[i];link!=NULL;)
		{
			nextLink = link->next;
			hashKeyFree (link->item);
			link->item = NULL;
			free (link);
			link = nextLink;
		}
		slots[i] = NULL;
	}
}
void clearHashTable (HashTablePtr hashTable)
{
	//printHashTable (hashTable);
	clearHashSlots (hashTable->previousSlots, hashTable->previousNumOfSlots, hashTable->hashKeyFree);
	hashTable->previousNumOfKeys = 0;
	clearHashSlots (hashTable->currentSlots, hashTable->currentNumOfSlots, hashTable->hashKeyFree);
	hashTable->currentNumOfKeys = 0;
	//printHashTable (hashTable);
}

HashLinkPtr newHashLink()
{
	HashLinkPtr result;

	result = (HashLinkPtr) checkMalloc (sizeof (HashLink),"Hash link");

	return result;
}

void hashTableMigrate (HashTablePtr hashTable)
{
	int i, slotNum;
	HashLinkPtr link;

	//printHashTable (hashTable);
	if (hashTable->previousNumOfKeys == 0 && hashTable->currentNumOfKeys >= 0.6*hashTable->currentNumOfSlots)
	{
		free (hashTable->previousSlots);
		hashTable->previousSlots = hashTable->currentSlots;
		hashTable->previousNumOfSlots = hashTable->currentNumOfSlots;
		hashTable->previousNumOfKeys = hashTable->currentNumOfKeys;
		hashTable->currentNumOfSlots = 2*hashTable->currentNumOfSlots + 1;
		hashTable->currentNumOfKeys = 0;
		//printf ("nodes %d\n", hashTable->currentNumOfSlots );
		hashTable->currentSlots = (HashLinkPtr *)checkMalloc (hashTable->currentNumOfSlots*sizeof(HashLinkPtr),"Hash slots");
		for (i=0;i<hashTable->currentNumOfSlots;i++)
			hashTable->currentSlots[i] = NULL;
		hashTable->currentRehashSlot = 0;
		hashTable->currentRehashLink = NULL;
	}
	else
	{
		if (hashTable->previousNumOfKeys > 0)
		{
			while (hashTable->previousSlots[hashTable->currentRehashSlot] == NULL)
			{
				hashTable->currentRehashSlot++;
			}
			link = hashTable->previousSlots[hashTable->currentRehashSlot];
			hashTable->previousSlots[hashTable->currentRehashSlot] = link->next;
			hashTable->previousNumOfKeys--;
			if (hashTable->hashKeyActive == NULL || hashTable->hashKeyActive (link->item))
			{
				slotNum = link->hashValue % hashTable->currentNumOfSlots;
				link->next = hashTable->currentSlots[slotNum];
				hashTable->currentSlots[slotNum] = link;
				hashTable->currentNumOfKeys++;
			}
			else
			{
				hashTable->hashKeyFree (link->item);
				free (link);
			}
		 }
	}
	//printHashTable (hashTable);
}

void hashTableInsert (HashTablePtr hashTable, void *k)
{
	unsigned long hashValue, slotNum;
	HashLinkPtr link;


	hashTableMigrate (hashTable);

	hashValue = hashTable->hashFunction (k);
	slotNum = hashValue % hashTable->currentNumOfSlots;

	link = newHashLink();
	link->item = k;
	link->hashValue = hashValue;
	link->next = hashTable->currentSlots[slotNum];
	hashTable->currentSlots[slotNum] = link;
	hashTable->currentNumOfKeys++;
}

void *oneSearch (unsigned long hashValue, HashTablePtr hashTable, HashLinkPtr *slots, int numOfSlots, void *k)
{
	HashLinkPtr link;
	void *result = NULL;


	link = slots[hashValue % numOfSlots];
	while (link != NULL && !hashTable->hashKeyEqual (link->item, k))
	{
		link = link->next;
	}

	if (link != NULL)
		result = link->item;

	//printf ("search return %d\n", result);
	return result;
}

void *hashTableSearchInsert (HashTablePtr hashTable, void *k)
{
	unsigned long hashValue, slotNum;
	HashLinkPtr link;
	void *result;

	//printf ("searchinsert %d\n", k);
	hashTableMigrate (hashTable);

	hashValue = hashTable->hashFunction (k);

	result = oneSearch (hashValue, hashTable, hashTable->currentSlots, hashTable->currentNumOfSlots, k);
	if (result == NULL)
		result = oneSearch (hashValue, hashTable, hashTable->previousSlots, hashTable->previousNumOfSlots, k);

	if (result == NULL)
	{
		link = newHashLink();
		link->item = k;
		link->hashValue = hashValue;
		slotNum = hashValue % hashTable->currentNumOfSlots;
		link->next = hashTable->currentSlots[slotNum];
		hashTable->currentSlots[slotNum] = link;
		hashTable->currentNumOfKeys++;
	}

	return result;

}


void *hashTableSearch (HashTablePtr hashTable, void *k)
{
	unsigned long hashValue;

	void *result;

	hashValue = hashTable->hashFunction (k);

	result = oneSearch (hashValue, hashTable, hashTable->currentSlots, hashTable->currentNumOfSlots, k);
	if (result == NULL)
		result = oneSearch (hashValue, hashTable, hashTable->previousSlots, hashTable->previousNumOfSlots, k);

	return result;
}

void hashTablePrintContents (HashTablePtr hashTable)
{
	int i;
	HashLinkPtr link;

	for (i=0;i<hashTable->previousNumOfSlots;i++)
		for (link=hashTable->previousSlots[i];link!=NULL;link=link->next)
			hashTable->hashKeyPrint (link->item);
	for (i=0;i<hashTable->currentNumOfSlots;i++)
		for (link=hashTable->currentSlots[i];link!=NULL;link=link->next)
			hashTable->hashKeyPrint (link->item);
}

void hashTableTraverse (HashTablePtr hashTable, HashKeyPrint f)
{
	int i;
	HashLinkPtr link;

	for (i=0;i<hashTable->previousNumOfSlots;i++)
		for (link=hashTable->previousSlots[i];link!=NULL;link=link->next)
			f (link->item);
	for (i=0;i<hashTable->currentNumOfSlots;i++)
		for (link=hashTable->currentSlots[i];link!=NULL;link=link->next)
			f (link->item);
}
