#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include "myalloc.h"

#define MAP_ANONYMOUS 0x20
//keeps track of start of memory arena
node_t* _arena_start = NULL;
int statusno = 0;


/* Initializes memory allocatory
* size: desired size for memory arena 
* returns: size of page
*/
extern int myinit(size_t size) {

    //Checks for invalid size argument, throwing error
    if (size == 0 || size > MAX_ARENA_SIZE) {
        statusno = ERR_BAD_ARGUMENTS;
        return ERR_BAD_ARGUMENTS;
    }

    printf("Initializing arena:\n");
    printf("...requested size %ld bytes\n", size);

    //Fetch system page size
    size_t pageSize = getpagesize();
    printf("...pagesize is %ld bytes\n", pageSize);


    printf("...adjusting size with page boundaries\n");
    //Adjust size to the nearest multiple of system page size
    if (size % pageSize != 0) {
      size_t pages_needed = (size + pageSize - 1) / pageSize;
      size = pages_needed * pageSize;
    }
    printf("...adjusted size is %ld bytes\n", size);

    printf("...mapping arena with mmap()\n");
    // Request block of memory from the OS using mmap
    _arena_start = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    //throw error if mmap fails
    if (_arena_start == MAP_FAILED) {
      statusno = ERR_SYSCALL_FAILED;
      return ERR_SYSCALL_FAILED;
    }

      // Initialize the first block to represent the entire free space
    _arena_start->size = size - sizeof(node_t); //Exclude header size
    _arena_start->is_free = 1; //Mark block as free
    _arena_start->fwd = NULL; //no forward block since this is the only block
    _arena_start->bwd = NULL; // no backward block since this is the only block

    statusno = 0;

    printf("...arena starts at %p\n", _arena_start);
    printf("...arena ends at %p\n", (char*)_arena_start + size);

    return (int) size;
}

/* 
*  Deallocates memory arena and resets allocator
*  returns: 0 indicating successful operation 
*/
extern int mydestroy() {
  
  //Check if memory error is already not initialized and return an error if so
  if (!_arena_start) {
    return ERR_UNINITIALIZED;
  }

  //Release the memory block back to the OS using munmap
  if (munmap(_arena_start, _arena_start->size) == -1) {
    return ERR_SYSCALL_FAILED;
  }
  
  // Reset the _arena_start pointer
  _arena_start = NULL;

  printf("Destroying Arena: ...unmapping arena with munmap()\n");

  return 0;
}

extern void* myalloc(size_t size) {
  // TODO: Implement
  return NULL;
}

extern void myfree(void *ptr) {
  // TODO: Implement
}