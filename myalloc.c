#include <stddef.h>
#include <sys/mman.h>
#include "myalloc.h"

#define MAP_ANONYMOUS 0x20
node_t* _arena_start = NULL;

extern int myinit(size_t size) {



    if (size == 0 || size > MAX_ARENA_SIZE) {
        statusno = ERR_BAD_ARGUMENTS;
        return ERR_BAD_ARGUMENTS;
    }

    printf("Initializing arena:\n");
    printf("...requested size %d bytes\n", size);
    printf("...pagesize is %d bytes\n", getpagesize());
    printf("...adjusting size with page boundaries\n");
    size = getpagesize();
    printf("...adjusted size is %d bytes\n", size);

    printf("...mapping arena with mmap()\n");
    _arena_start = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);


      // Initialize the first block to represent the entire free space
    _arena_start->size = size;
    _arena_start->is_free = 1;
    _arena_start->fwd = NULL;
    _arena_start->bwd = NULL;

    statusno = 0;

    printf("...arena starts at %p\n", _arena_start);
    printf("...arena ends at %p\n", _arena_start + size);

}

extern int mydestroy() {

}

extern void* myalloc(size_t size) {

}

extern void myfree(void *ptr) {

}