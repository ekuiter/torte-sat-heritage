#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void* my_malloc(size_t size)
{
  void* ans = malloc(size);

  if(ans==NULL){
    printf("c Insufficient memory in malloc\n");
    exit(0);
  }

  return ans;
}

void* my_calloc(size_t num,size_t size)
{
  
  void* ans = calloc(num,size);

  if(ans==NULL){
    printf("c Insufficient memory in calloc\n");
    exit(0);
  }

  return ans;
}
