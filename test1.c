#include "malloc.h"
#include <assert.h>


int main(int argc, char **argv)
{
  size_t size = 34;
  size_t big = 2032;

	void *mem = malloc(size);
  printf("Successfully malloc'd block %p of size %zd \n", mem, size);
  assert(mem != NULL);

  void *mem1 = malloc(big);
  printf("Successfully malloc'd block %p of size %zd \n", mem1, big);
  assert(mem1 != NULL);

  free(mem);
  printf("Successfully free'd block %p of size %zd \n", mem, size);

  void *mem2 = malloc(size*2);
  printf("Successfully malloc'd block %p of size %zd \n", mem2, size*2);
  assert(mem2 != NULL);

  void *mem3 = malloc(size);
  printf("Successfully malloc'd block %p of size %zd \n", mem3, size);
  assert(mem3 != NULL);

  void *mem4 = malloc(big*12);
  printf("Successfully malloc'd block %p of size %zd \n", mem4, big*12);
  assert(mem4 != NULL);

  free(mem3);
  printf("Successfully free'd block %p of size %zd \n", mem3, size);

  void *mem5 = calloc(5, size);
  printf("Successfully calloc'd block %p of size %zd * 5\n", mem5, size*5);
  assert(mem5 != NULL);

  void *mem6 = NULL;
  int success = posix_memalign((void **)&mem6, 16, size*2);
  printf("Successfully posix alloc'd block %p of size %zd\n", mem6, size*2);
  assert(success == 0);

  void *mem7 = realloc(mem5, 5*size);
  printf("Successfully realloc'd block to %p of size %zd from %p\n", mem7, size*5, mem5);
  assert(mem7 != NULL);

  void *mem8 = realloc(mem7, 32*size);
  printf("Successfully realloc'd block to %p of size %zd from %p\n", mem8, size*32, mem7);
  assert(mem8 != NULL);

  void *mem9 = memalign(16, size * 20);
  printf("Successfully memaligned'd block to %p of size %zd\n", mem9, size * 20);
  assert(mem9 != NULL);

  void *mem10 = malloc(107360);
  printf("Successfully malloc'd block %p of size 107360 \n", mem10);
  assert(mem10 != NULL);

  return 0;
}
