#include <stdio.h>

#include "my_mem.h"

// run at the beginning of main, before any other allocations
int testCases(){
    // allocate a lot smaller than globalMem
    unsigned char* allocated = (unsigned char*)my_malloc(globalMem / 4);
    mem_stats_struct mem_stats;
    mem_get_stats(&mem_stats);
    // test that allocation 1 split block in half
    if (mem_stats.num_blocks_free != 1 && mem_stats.num_blocks_used != 1){
        printf("test failed in normal allocate!\n");
        return 0;
    }
    my_free(allocated);
    mem_get_stats(&mem_stats);
    // test that can free regular allocation
    if (mem_stats.num_blocks_free != 1 && mem_stats.num_blocks_used != 0){
        printf("test failed in normal free!\n");
        return 0;
    }

    mem_reset();

    // test cant allocate more than max memory
    allocated = (unsigned char *)my_malloc(globalMem* 2);
    if (allocated != NULL){
        printf("test failed- we arent supposed to be able to allocate more than max size\n");
        return 0;
    }

    // test cant allocate more than available free blocks
    allocated = (unsigned char*)my_malloc(globalMem -5);
    unsigned char *cantAllocate = (unsigned char *)my_malloc(10);
    if (cantAllocate != NULL) {
        printf("test failed- we arent supposed to be able to allocate more than available\n");
        return 0;
    }

    // reset arr for remaining tests
    mem_reset();

    // test ability to merge
    unsigned char *ptr_array[10];
    unsigned int sizes[] = {50, 20, 20, 20, 50, 0};

    for (int i = 0; sizes[i] != 0; i++) {
        char buf[1024];
        ptr_array[i] = (unsigned char *)my_malloc(sizes[i]);
    
        sprintf(buf, "after iteration %d size %d", i, sizes[i]);

        print_stats(buf);

  }

  int currentFree = freeNum; 

  my_free(ptr_array[1]);  print_stats("after free #1");

  my_free(ptr_array[3]);  print_stats("after free #3");

  my_free(ptr_array[2]);  print_stats("after free #2");

  if (currentFree + 1 != freeNum) {
      printf("test failed - merge didnt occur\n");
      return 0;
  }
  return 1;
}
