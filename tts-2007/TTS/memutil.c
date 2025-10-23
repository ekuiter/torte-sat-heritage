#include <string.h>

#include "memutil.h"
#include "dimacs.h"


static char *buffer1, *buffer2;

void initMemUtil()
{
	buffer1 = checkMalloc(1000,"initial bufffer");
	buffer2 = checkMalloc(1000,"initial bufffer");
}

void *checkMalloc (size_t size, char *message)
{
	void *result;
	result = malloc (size);
	if (result == NULL)
	{
		free (buffer1);
		sprintf (buffer2,"Out of memory (malloc) <%s>\n", message);
		dimacsComment (buffer2);
		exit  (dimacsUNKNOWN);
	}
	return result;
}

void *checkRealloc (void *ptr, size_t size, char *message)
{
	void *result;
	result = realloc (ptr, size);
	if (result == NULL)
	{
		free (buffer1);
		sprintf (buffer2,"Out of memory (malloc) <%s>\n", message);
		dimacsComment (buffer2);
		exit  (dimacsUNKNOWN);
	}
	return result;
}

int ptrCompare (const void *p1, const void *p2)
{
		void *r1,*r2;
		int result;
		r1 = *((void **)p1);
		r2 = *((void **)p2);
		if (r1 < r2)
			result = -1;
		else if (r1 == r2)
			result =  0;
		else result = 1;
		return result;
}

void **twoDimArraySized (int rows, int columns, int size)
{
	int row;
	void **result;
	result = checkMalloc (rows*sizeof (void *),"2d-rows");
	for (row=0;row<rows;row++)
		result[row] = checkMalloc (columns*size,"2d-cols");
	return result;
}

void **twoDimArray (int rows, int columns)
{
	int row;
	void **result;
	result = checkMalloc (rows*sizeof (void *),"2d-rows");
	for (row=0;row<rows;row++)
		result[row] = checkMalloc (columns*sizeof (void *),"2d-cols");
	return result;
}

char *addExtension (char *filename, char *extension)
{
	char *result;
	int resultLength;
	
	resultLength = strlen (filename) + strlen (extension) + 1;
	result = (char *) checkMalloc (sizeof(char)*(resultLength+1),"");
	strcpy (result, filename);
	strcat (result, ".");
	strcat (result, extension);
	
	return result;
}

char *changeExtension (char *filename, char *extension)
{
	char *result;
	int resultLength;
	int i;
	
	for (i = strlen(filename)-1;i>=0 && filename[i] != '.'; i--);
	if (i < 0)
		i = strlen(filename);

	resultLength = i + strlen (extension) + 1;
	result = (char *) checkMalloc (sizeof(char)*(resultLength+1),"");
	strncpy (result, filename, i);
	result[i] = '\0';
	strcat (result, ".");
	strcat (result, extension);
	
	return result;

}
