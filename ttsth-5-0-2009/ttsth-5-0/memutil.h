#ifndef MEMUTIL_H

#define MEMUTIL_H

#include <stdio.h>
#include <stdlib.h>

#ifndef false
#define false 0
#define true 1
#endif



extern void initMemUtil();

extern void *checkMalloc (size_t size, char *message);

void *checkMallocContinue (size_t size, char *message);

extern void *checkRealloc (void *ptr, size_t size, char *message);

int ptrCompare (const void *p1, const void *p2);

int intCompare (const void *p1, const void *p2);

extern void **twoDimArray (int rows, int columns);

extern void **twoDimArraySized (int rows, int columns, int size);

extern char *addExtension (char *filename, char *extension);

extern char *changeExtension (char *filename, char *extension);


#endif
