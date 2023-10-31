// #include <stddef.h>
// #include <stdio.h>
// #include <unistd.h>
// #include <sys/mman.h>
// #include "myalloc.h"

// #define MAP_ANONYMOUS 0x20
// //keeps track of start of memory arena
// node_t* _arena_start = NULL;
// int statusno = 0;


// /* Initializes memory allocatory
// * size: desired size for memory arena 
// * returns: size of page
// */
// extern int myinit(size_t size) {

//     //Checks for invalid size argument, throwing error
//     if (size == 0 || size > MAX_ARENA_SIZE) {
//         statusno = ERR_BAD_ARGUMENTS;
//         return ERR_BAD_ARGUMENTS;
//     }

//     printf("Initializing arena:\n");
//     printf("...requested size %ld bytes\n", size);

//     //Fetch system page size
//     size_t pageSize = getpagesize();
//     printf("...pagesize is %ld bytes\n", pageSize);


//     printf("...adjusting size with page boundaries\n");
//     //Adjust size to the nearest multiple of system page size
//     if (size % pageSize != 0) {
//       size_t pages_needed = (size + pageSize - 1) / pageSize;
//       size = pages_needed * pageSize;
//     }
//     printf("...adjusted size is %ld bytes\n", size);

//     printf("...mapping arena with mmap()\n");
//     // Request block of memory from the OS using mmap
//     _arena_start = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

//     //throw error if mmap fails
//     if (_arena_start == MAP_FAILED) {
//       statusno = ERR_SYSCALL_FAILED;
//       return ERR_SYSCALL_FAILED;
//     }

//       // Initialize the first block to represent the entire free space
//     _arena_start->size = size - sizeof(node_t); //Exclude header size
//     _arena_start->is_free = 1; //Mark block as free
//     _arena_start->fwd = NULL; //no forward block since this is the only block
//     _arena_start->bwd = NULL; // no backward block since this is the only block

//     statusno = 0;

//     printf("...arena starts at %p\n", _arena_start);
//     printf("...arena ends at %p\n", (char*)_arena_start + size);

//     return (int) size;
// }

// /* 
// *  Deallocates memory arena and resets allocator
// *  returns: 0 indicating successful operation 
// */
// extern int mydestroy() {
  
//   //Check if memory error is already not initialized and return an error if so
//   if (!_arena_start) {
//     return ERR_UNINITIALIZED;
//   }

//   //Release the memory block back to the OS using munmap
//   if (munmap(_arena_start, _arena_start->size) == -1) {
//     return ERR_SYSCALL_FAILED;
//   }
  
//   // Reset the _arena_start pointer
//   _arena_start = NULL;

//   printf("Destroying Arena: ...unmapping arena with munmap()\n");

//   return 0;
// }

// extern void* myalloc(size_t size) {
 
//   //Check if memory arena is initialized
//   if (!_arena_start) {
//     statusno = ERR_UNINITIALIZED;
//     return NULL;
//   }

//   //Adjust size to include block header and ensure pointer is setup correctly
//   size_t adjusted_size = size + sizeof(node_t);
  
//   // if requested size is exactly one page, adjust size
//   if (adjusted_size == getpagesize()) {
//     size = getpagesize() - 2 * sizeof(node_t);
//     adjusted_size = size + sizeof(node_t);
//   }

//   node_t* current = _arena_start;

//   //Debug statements. use -v when running shell scripts to view
//   printf("Examining block at %p\n", current);
//   printf("Block size: %ld, Is free: %d\n", current->size, current->is_free);
//   printf("adjusted size is %ld", adjusted_size);

//   //Traverse the memory arena to find a suitable free block
//   while (current) {
//     //check if the current block is free and large enough
//     if (current->is_free && current->size >= adjusted_size) {
//       //If the current block size is exactly what we need or slightly bigger, allocate whole block
//       if(current->size <= adjusted_size + sizeof(node_t)) {
//         current->is_free = 0;
//         printf("Returning");
//         fflush(stdout);
//         return (void*)((char*)current + sizeof(node_t));
//       }
//       //otherwise split block
//       node_t* new_block = (node_t*)((char*)current + adjusted_size);
//       new_block->size = current->size - adjusted_size;
//       new_block->is_free = 1;
      
//       //update fwd and bwd pointers of new block to add to list properly
//       new_block->fwd = current->fwd;
//       new_block->bwd = current;

//       //update current blocks properties to reflect the allocation
//       current->size = adjusted_size;
//       current->is_free = 0;
//       current->fwd = new_block;

//       return (void*)((char*)current + sizeof(node_t));
//     }
//     current = current->fwd;
//   }

//   //if we reach this point, no suitable memory block was found
//   statusno = ERR_OUT_OF_MEMORY;
//   return NULL;
// }

// /* Frees a previously allocated block of memory
// *  ptr: Pointer to the block of memory to be freed
// */
// extern void myfree(void *ptr) {
//   //If the provided printer is null, simply return
//   if (!ptr) {
//     return;
//   }

//   //retrieve block header from pointer
//   node_t* block = (node_t*)((char*)ptr - sizeof(node_t));

//   //mark block as free
//   block->is_free = 1;

//   //check next block for coalescing
//   node_t *next_block = (node_t*)((char*)block + block->size);
//   //if next block is free, we can merge the current block with the next block
//   if (next_block->is_free) {
//     block->size += next_block->size; // increase size of current block to include next block
//     block->fwd = next_block->fwd; //update fwd pointer to point to the block after the next block
//     //if the next block has a block after it, update its previous pointer, removing nextblock effectively
//     if (next_block->fwd) {
//       next_block->fwd->bwd = block;
//     }
//   }

//   //check previous block for coalescing
//   //if current block has a previous block that is free, we can merge it
//   if (block->bwd && block->bwd->is_free) {
//     block->bwd->size += block->size; //Increase size of previous block to include size of current
//     block->bwd->fwd = block->fwd; //update fwd pointer of previous block to point to the block after the current block
//     //if current block has a block after it, update its pointer
//     //this effectively removes current block from the list since its been merged with the previous and we dont require it anymore
//     if (block->fwd) {
//       block->fwd->bwd = block->bwd;
//     }
//   }
// }







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
 
  //Check if memory arena is initialized
  if (!_arena_start) {
    statusno = ERR_UNINITIALIZED;
    return NULL;
  }

  // Check if size is valid
  if (size <= 0 || size > MAX_ARENA_SIZE)
  {
      statusno = ERR_BAD_ARGUMENTS;
      return NULL;
  }

//   //Adjust size to include block header and ensure pointer is setup correctly
//   size_t adjusted_size = size + sizeof(node_t);
  
//   // if requested size is exactly one page, adjust size
//   if (adjusted_size == getpagesize()) {
//     size = getpagesize() - 2 * sizeof(node_t);
//     adjusted_size = size + sizeof(node_t);
//   }

//   node_t* current = _arena_start;

//   //Debug statements. use -v when running shell scripts to view
//   printf("Examining block at %p\n", current);
//   printf("Block size: %ld, Is free: %d\n", current->size, current->is_free);
//   printf("adjusted size is %ld", adjusted_size);

//   //Traverse the memory arena to find a suitable free block
//   while (current) {
//     //check if the current block is free and large enough
//     if (current->is_free && current->size >= adjusted_size) {
//       //If the current block size is exactly what we need or slightly bigger, allocate whole block
//       if(current->size <= adjusted_size + sizeof(node_t)) {
//         current->is_free = 0;
//         printf("Returning");
//         fflush(stdout);
//         return (void*)((char*)current + sizeof(node_t));
//       }
//       //otherwise split block
//       node_t* new_block = (node_t*)((char*)current + adjusted_size);
//       new_block->size = current->size - adjusted_size;
//       new_block->is_free = 1;
      
//       //update fwd and bwd pointers of new block to add to list properly
//       new_block->fwd = current->fwd;
//       new_block->bwd = current;

//       //update current blocks properties to reflect the allocation
//       current->size = adjusted_size;
//       current->is_free = 0;
//       current->fwd = new_block;

//       return (void*)((char*)current + sizeof(node_t));
//     }
//     current = current->fwd;
//   }

//   //if we reach this point, no suitable memory block was found
//   statusno = ERR_OUT_OF_MEMORY;
//   return NULL;
// }
  printf("Allocating memory:\n");
  printf("...looking for free chunk of >= %li bytes", size);
  // Find a free block of memory
  node_t *current = _arena_start;
  while (current != NULL)
  {
      if (current->is_free && current->size >= size)
      {
          printf("...found free chunk of %li bytes with header at %p\n", size, current);
          printf("...free chunk-fwd currently points to %p\n", current->fwd);
          printf("...free chunk-bwd currently points to %p\n", current->bwd);
          printf("...checking if splitting is required\n");
          // Split the block if it's too big
          if (current->size > size + sizeof(node_t))
          {
              // create a new block after the current block
              node_t *new_block = (node_t *)((char *)current + sizeof(node_t) + size);
              new_block->size = current->size - size - sizeof(node_t);
              new_block->is_free = 1;
              new_block->fwd = current->fwd;
              new_block->bwd = current;
              if (current->fwd != NULL)
              {
                  current->fwd->bwd = new_block;
              }
              current->fwd = new_block;

              // update size of current block to match the requested size
              current->size = size;
          }
          current->size > size + sizeof(node_t) ? printf("...splitting is required\n") : printf("...splitting is not required\n");
          // mark block as allocated and update status
          current->is_free = 0;
          statusno = 0;

          printf("...updating chunk header at %p\n", current);
          printf("...being careful with my pointer arthimetic and void pointer casting\n");
          printf("...allocation starts at %p\n", (current + 1));
          // return pointer to the user data in the block
          return (void *)(current + 1);
      }
      current = current->fwd;
  }

  // No free block found
  statusno = ERR_OUT_OF_MEMORY;
  return NULL;
}
/* Frees a previously allocated block of memory
*  ptr: Pointer to the block of memory to be freed
*/
extern void myfree(void *ptr) {
  //If the provided printer is null, simply return
  if (!ptr) {
    return;
  }

  //retrieve block header from pointer
  node_t* block = (node_t*)((char*)ptr - sizeof(node_t));

  //mark block as free
  block->is_free = 1;

  //check next block for coalescing
  node_t *next_block = (node_t*)((char*)block + block->size);
  //if next block is free, we can merge the current block with the next block
  if (next_block->is_free) {
    block->size += next_block->size; // increase size of current block to include next block
    block->fwd = next_block->fwd; //update fwd pointer to point to the block after the next block
    //if the next block has a block after it, update its previous pointer, removing nextblock effectively
    if (next_block->fwd) {
      next_block->fwd->bwd = block;
    }
  }

  //check previous block for coalescing
  //if current block has a previous block that is free, we can merge it
  if (block->bwd && block->bwd->is_free) {
    block->bwd->size += block->size; //Increase size of previous block to include size of current
    block->bwd->fwd = block->fwd; //update fwd pointer of previous block to point to the block after the current block
    //if current block has a block after it, update its pointer
    //this effectively removes current block from the list since its been merged with the previous and we dont require it anymore
    if (block->fwd) {
      block->fwd->bwd = block->bwd;
    }
  }
}