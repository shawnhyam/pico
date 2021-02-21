#ifndef ALLOCATE_H_
#define ALLOCATE_H_

#include <stdint.h>
#include <stddef.h>

#define NEW(p,a) ((p) = allocate(sizeof *(p), (a)))
#define NEWO(p,a) memset(NEW((p),(a)), 0, sizeof *(p))
#define roundup(x,n) (((x)+((n)-1))&(-((n)-1)))

void *allocate(size_t num_bytes, uint16_t arena);
void *newarray(size_t num_elements, size_t element_size, uint16_t arena);
void deallocate(uint16_t arena);

#endif
