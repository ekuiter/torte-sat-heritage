#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef unsigned long HashFunction(void *key);
typedef int HashKeyEqual (void *k1, void *k2);
typedef void HashKeyFree (void *k);
typedef void HashKeyPrint (void *k);
typedef int HashKeyActive (void *k);
typedef struct HashLinkStruct
{
	void * item;
	unsigned long hashValue;
	struct HashLinkStruct *next;
}   HashLink, *HashLinkPtr;

typedef struct HashTableStruct
{
	int currentNumOfSlots, previousNumOfSlots;
	HashLinkPtr *currentSlots, *previousSlots, currentRehashLink;
	HashFunction *hashFunction;
	HashKeyEqual *hashKeyEqual;
	HashKeyFree *hashKeyFree;
	HashKeyPrint *hashKeyPrint;
	HashKeyActive *hashKeyActive;
	int currentNumOfKeys, previousNumOfKeys, currentRehashSlot;
	
} HashTable,*HashTablePtr;


extern HashTablePtr newHashTable (HashFunction hashFunction, HashKeyEqual hashKeyEqual, HashKeyFree hashKeyFree,
									HashKeyPrint hashKeyPrint, HashKeyActive hashKeyActive);

extern void clearHashTable (HashTablePtr hashTable);
extern void freeJustHashSlots (HashTablePtr hashTable);

extern void hashTableInsert (HashTablePtr hashTable, void *k);
extern void *hashTableSearchInsert (HashTablePtr hashTable, void *k);
extern void *hashTableSearch (HashTablePtr hashTable, void *k);
extern void hashTablePrintContents (HashTablePtr hashTable);
extern void hashTableTraverse (HashTablePtr hashTable, HashKeyPrint f);
extern void hashTableRemove (HashTablePtr hashTable, void *k);

#endif
