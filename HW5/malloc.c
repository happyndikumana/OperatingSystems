#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct block *)(ptr) - 1)
#define SINGLE_BLOCK_SIZE 512


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct block 
{
   size_t      size;  /* Size of the allocated block of memory in bytes */
   struct block *next;  /* Pointer to the next block of allcated memory   */
   bool        free;  /* Is this block free?                     */
};


struct block *FreeList = NULL; /* Free list to track the blocks available */
struct block *Last = NULL;

/*
void printBlocks(void) {
   struct block *curr = FreeList;
   
   while (curr) {

      printf("\nAddress: \t%p\n", curr);
      printf("Size: \t\t%d\n", curr->size);
      printf("Free: \t\t%d\n", curr->free);

      curr = curr->next;
   }
}
*/


/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free blocks
 * \param size size of the block needed in bytes 
 *
 * \return a block that fits the request or NULL if no free block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct block *findFreeBlock(struct block **last, size_t size) 
{
   struct block *curr = FreeList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
   /* Best fit */
   struct block *idealBlock = NULL;
   
   while (curr) {

      *last = curr;
      
       if (curr->free && curr->size >= size) {

          if (!idealBlock) idealBlock = curr;/* first free block with the right size if the first ideal */

          if (idealBlock && curr->size < idealBlock->size) idealBlock = curr;
          /* a new ideal is a block that has smaller size than first ideal but that is still large enough to hold the block */
       }
       curr = curr->next;
   }

   if (idealBlock) curr = idealBlock; /* if an ideal block was found, replace the curr to be returned with it */

#endif

#if defined WORST && WORST == 0
   /* Worst fit */
   struct block *worstBlock = NULL;
   
   while (curr) {

      *last = curr;
      
       if (curr->free && curr->size >= size) {

          if (!worstBlock) worstBlock = curr; /* first free block with the right size if the first worst */

          if (worstBlock && curr->size > worstBlock->size) worstBlock = curr;
          /* a new worst is a block that has larger size than first worst */
       }
       curr = curr->next;
   }

   if (worstBlock) curr = worstBlock; /* if a worse block was found, replace the curr to be returned with it */

#endif

#if defined NEXT && NEXT == 0
   /* Next fit */ 
   if (FreeList && !Last) Last = FreeList; 
   /*
      if head of heap exists and the pointer last is pointing to NULL,
      meaning no other allocation has went through the heap to give the variable last a value
      then make Last  = head

   */

   while (Last && !(Last->free && Last->size >= size)) 
   {
      *last = Last;
      Last  = Last->next;
   }
   /*
      if last != null, then from the address last is pointing at, start your search there.
   */
   curr = Last;

#endif

   return curr;
}



/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated block of NULL if failed
 */
struct block *growHeap(struct block *last, size_t size) 
{
   /* Request more space from OS */
   struct block *curr = (struct block *)sbrk(0);
   struct block *prev = (struct block *)sbrk(sizeof(struct block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct block *)-1) 
   {
      return NULL;
   }

   /* Update FreeList if not set */
   if (FreeList == NULL) 
   {
      FreeList = curr;
   }

   /* Attach new block to prev block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;

   num_grows++;
   num_blocks++;
   max_heap += size;

   return curr;
}
/*
      check if passed in block has remaining space
      check if that remanining size is bigger than struct block + 4 bytes
      if it does, split the block at requested size mark
*/

struct block *splitFreeBlock(struct block *freeBlock) {

   if (freeBlock->size <= SINGLE_BLOCK_SIZE || freeBlock->size % 2 != 0) {
      return freeBlock;
   }
   /* split right part */
   struct block *rightFreeBlock = (struct block *)( (char *)freeBlock + (freeBlock->size / 2) );
   rightFreeBlock->next = freeBlock->next;
   rightFreeBlock->size = freeBlock->size / 2;
   rightFreeBlock->free = freeBlock->free;

   /* split left part */
   struct block *leftFreeBlock = freeBlock;
   leftFreeBlock->next = rightFreeBlock;
   leftFreeBlock->size = freeBlock->size / 2;
   leftFreeBlock->free = freeBlock->free;

   num_splits++;

   return splitFreeBlock(leftFreeBlock);
   return splitFreeBlock(rightFreeBlock);

}

/*
 * \brief malloc
 *
 * finds a free block of heap memory for the calling process.
 * if there is no free block that satisfies the request then grows the 
 * heap and returns a new block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
    //   atexit( printBlocks );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   num_requested+= size;

   /* Look for free block */
   struct block *last = FreeList;
   struct block *next = findFreeBlock(&last, size);

   if (next != NULL) num_reuses++; /* if a free block is found, then it was reused */

   /* Split free block if possible */
   if (next != NULL) next = splitFreeBlock(next);

   /* Could not find free block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
   }

   /* Could not find free block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark block as in use */
   next->free = false;

   num_mallocs++;

//    printBlocks(); /* print blocks for debugging */

   /* Return data address associated with block */
   return BLOCK_DATA(next);
}

void *realloc(void *ptr, size_t size) {

   struct block *currPtr = BLOCK_HEADER(ptr);

   void *newPtr;
   newPtr = malloc(size);

   memcpy(newPtr, ptr, currPtr->size);
   free(ptr);

   return newPtr;
}

void *calloc(size_t nitems, size_t size) {
   size_t actualSize = nitems * size;

   void *ptr = malloc(actualSize);

   return memset(ptr, 0, actualSize); /* not ideal because OS might set memory from malloc to 0 and this might be unnecessary. But Oh well.... */
}

/*
 * \brief free
 *
 * frees the memory block pointed to by pointer. if the block is adjacent
 * to another block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make block as free */
   struct block *currPtr = BLOCK_HEADER(ptr);
   assert(currPtr->free == 0);
   currPtr->free = true;

   /* Coalescing free blocks  */

   struct block *curr = FreeList;
   
   while (curr) {

      struct block *nextBlock = curr->next;

      if (curr->free && nextBlock && nextBlock->free) {

         curr->next = nextBlock->next; /* join them by having the pointer before have its next point to the next next */
         curr->size += nextBlock->size; /* combine their sizes */
         num_coalesces++;

      } 

      curr = curr->next;
   }

   num_frees++;
}


/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
