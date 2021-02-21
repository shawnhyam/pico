#include "allocate/allocate.h"
#include <stdlib.h>  // ? do I need this
#include "pico/stdlib.h"

#define ARENA_SIZE_KB 16

typedef struct block {
    struct block *next;
    char *limit;
    char *avail;
} block_t;


static block_t
    first[] = { { NULL}, {NULL}, {NULL} },
    *arenas[] = { &first[0], &first[1], &first[2] };

static struct block *freeblocks;

union align {
    long l;
    char *p;
    double d;
    int (*f)(void);
};

union header {
    struct block b;
    union align a;
};

void *allocate(size_t num_bytes, uint16_t arena) {
    block_t *ap = arenas[arena];
    num_bytes = roundup(num_bytes, sizeof (union align));
    while (ap->avail + num_bytes > ap->limit) {
        // get a new block
        if ((ap->next = freeblocks) != NULL) {
            freeblocks = freeblocks->next;
            ap = ap->next;
        } else {
            // allocate a new block
            unsigned m = sizeof (union header) + num_bytes + ARENA_SIZE_KB*1024;
            ap->next = malloc(m);
            ap = ap->next;
            if (ap == NULL) {
                //error("insufficient memory\n");
                exit(1);
            }
            ap->limit = (char *)ap + m;
        }
        ap->avail = (char *)((union header *)ap + 1);
        ap->next = NULL;
        arenas[arena] = ap;
    }
    ap->avail += num_bytes;
    return ap->avail - num_bytes;
}

void *newarray(size_t num_elements, size_t element_size, uint16_t arena) {
    return allocate(num_elements*element_size, arena);
}

void deallocate(uint16_t arena) {
    arenas[arena]->next = freeblocks;
    freeblocks = first[arena].next;
    first[arena].next =NULL;
    arenas[arena] = &first[arena];
}
