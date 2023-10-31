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

// Allocate memory function
extern void *myalloc(size_t size) {
    // Check if myinit has been called
    if (!_arena_start) {
        statusno = ERR_UNINITIALIZED;
        return NULL;
    }

    printf("Allocating memory:\n");
    printf(" - Looking for free chunk of >= %li bytes\n", size);

    // Find a free block of memory
    node_t *current = _arena_start;
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            printf(" - Found free chunk of %li bytes with header at %p\n", size, current);
            printf(" - Free chunk-fwd currently points to %p\n", current->fwd);
            printf(" - Free chunk-bwd currently points to %p\n", current->bwd);
            printf(" - Checking if splitting is required\n");

            // Split the block if it's too big
            if (current->size > size + sizeof(node_t)) {
                // Create a new block after the current block
                node_t *new_block = (node_t *)((char *)current + sizeof(node_t) + size);
                new_block->size = current->size - size - sizeof(node_t);
                new_block->is_free = 1;
                new_block->fwd = current->fwd;
                new_block->bwd = current;

                if (current->fwd != NULL) {
                    current->fwd->bwd = new_block;
                }
                current->fwd = new_block;

                // Update size of the current block to match the requested size
                current->size = size;
            }
            printf(current->size > size + sizeof(node_t) ? " - Splitting is required\n" : " - Splitting is not required\n");

            // Mark the block as allocated and update status
            current->is_free = 0;
            statusno = 0;

            printf(" - Updating chunk header at %p\n", current);
            printf(" - Being careful with my pointer arithmetic and void pointer casting\n");
            printf(" - Allocation starts at %p\n", (current + 1));

            // Return pointer to the user data in the block
            return (void *)(current + 1);
        }
        current = current->fwd;
    }

    // No free block found
    statusno = ERR_OUT_OF_MEMORY;
    return NULL;
}

// Free memory function
extern void myfree(void *ptr) {
    printf("Freeing allocated memory:\n");
    printf(" - Supplied pointer %p:\n", ptr);

    // Calculate the header pointer from the user data pointer
    node_t *header = (node_t *)((char *)ptr - sizeof(node_t));

    printf(" - Being careful with my pointer arithmetic and void pointer casting\n");
    printf(" - Accessing chunk header at %p\n", header);
    printf(" - Chunk of size %li bytes\n", header->size);

    // Mark the block as free
    header->is_free = 1;

    // Coalesce adjacent free blocks
    node_t *prev = header->bwd;
    node_t *next = header->fwd;
    printf(" - Checking if coalescing is needed\n");
    int coalescing = 0;

    // Check if the block before the current block is free
    if (prev != NULL && prev->is_free) {
        coalescing = 1;

        // Update the size of the previous block to include the size of the current block and the next block (if necessary)
        prev->size += sizeof(node_t) + header->size;

        // Update the forward and backward pointers of the previous block to skip over the current block
        prev->fwd = next;
        if (next != NULL) {
            next->bwd = prev;
        }

        // Update the header pointer to point to the previous block
        header = prev;
    }

    // Check if the block after the current is free
    if (next != NULL && next->is_free) {
        coalescing = 1;

        // Update the size and pointers
        header->size += sizeof(node_t) + next->size;
        header->fwd = next->fwd;
        if (next->fwd != NULL) {
            next->fwd->bwd = header;
        }
    }
    (coalescing) ? printf(" - Coalescing is needed\n") : printf(" - Coalescing is not needed\n");
}