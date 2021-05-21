#include <stdio.h>
#include <stdbool.h>

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};
struct _block *heapList = NULL;
int main()
{
    struct _block *curr = heapList;
    struct _block *send = heapList;
    struct _block **last = &send;
    size_t size = 100;
    while (curr && !(curr->free && curr->size >= size)) 
    {
      *last = curr;
      curr  = curr->next;
      printf("in loop\n");
    }

    printf("address = %p", curr);

    return 0;
}