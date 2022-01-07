#include<stdio.h>
#include<stdlib.h>
#include <errno.h>
#include "my_mem.h"

unsigned char *original_memory;
unsigned int original_size;

int freeNum;
int usedNum;

struct storage {
    int blockSize;
    unsigned char *location;
    int freeZeroUsedOne;
};

struct storage *arr = NULL;
int globalMem;


void mem_init(unsigned char *my_memory, unsigned int my_mem_size){
    original_memory = my_memory;
    original_size = my_mem_size;

    if (arr != NULL) {
        free(arr);
    }

    // all structs are zeroed by calloc
    arr = (struct storage *)calloc(my_mem_size, sizeof(struct storage));
    globalMem = my_mem_size;

    // array is initialized to one free block of globalMem size
    arr[0].blockSize = globalMem;
    arr[0].freeZeroUsedOne = 0;
    arr[0].location = my_memory;

    freeNum = 1;
    usedNum = 0;
}

void mem_reset() {
    //reset array to how it is after init
    mem_init(original_memory, original_size);
}


void *my_malloc(unsigned size){
    int i;
    int j;
    int localSize = size;

    // if size of allocation is 0
    if (localSize == 0) {
        printf("cant allocate size 0 of memory\n");
        return NULL;
    }
    if (localSize > globalMem) {
        printf("cant allocate larger than originally allocated memory\n");
        return NULL;

    }

    for (i = 0; i < globalMem; i++){
        // save pointer of arr[i] 
        struct storage *block = &arr[i];

        // if current block size is larger than what we want to allocate and it is free
        if (block->blockSize >= localSize && block->freeZeroUsedOne == 0){
            // if it is a LOT larger
            if (block->blockSize / 2 > localSize){
                // divide in half, one part to new block, other to give to user
                block->blockSize /= 2;
                block->freeZeroUsedOne = 1;

                // need to shift everything by one to insert other half of block in next slot of array
                for (j = (globalMem - 2); j > i; j--) {
                    arr[j+1] = arr[j];
                }

                // create the new block at the space we just created after shifting
                struct storage *new_block = &arr[i + 1];

                new_block->location = block->location + block->blockSize;
                new_block->blockSize = block->blockSize;
                new_block->freeZeroUsedOne = 0;

                // used increases by 1, free doesnt decrease bc of the split we are adding new block that is free instead
                usedNum = usedNum + 1;

                return block->location;
            }

            // else if the block is not much bigger just allocate the whole block 
            else {
                block->freeZeroUsedOne = 1;

                freeNum = freeNum - 1;
                usedNum = usedNum + 1;

                return block->location;
            }
        }
    }
    // IF IVE GONE THROUGH FULL FOR LOOP AND HAVENT FOUND ANY MEMORY BIG ENOUGH
    printf("No available memory for this size\n");
    return NULL;
}


// merges the block at this index with the block after it
void merge_blocks(int index) {
    int j;
    struct storage *block = &arr[index];
    struct storage *next = &arr[index+1];

    // blockSize becomes the size of blocks merging
    block->blockSize += next->blockSize;

    // merge means one less free block (2 free blocks now 1)
    freeNum = freeNum - 1;

    for (j = (index + 1); j < (globalMem - 1); j++) {
        // shift to the left
        arr[j] = arr[j+1];
    }
}

void my_free(void *mem_pointer){
    int i;
    for (i = 0; i < globalMem; i ++) {
        struct storage *block = &arr[i];

        // if we found the block with the pointer we want to free

        if (block->location == mem_pointer) {
            if (block->freeZeroUsedOne == 0) {
                printf("this memory is already free\n");
                return;
            }

            block->freeZeroUsedOne = 0;
            freeNum = freeNum + 1;
            usedNum = usedNum - 1;

            // merge if one after is free and if not end of array
            if (i != (globalMem -1) && arr[i + 1].freeZeroUsedOne == 0) {
                merge_blocks(i);
            }

            // merge if one before is free and if not beginning of array
            if (i != 0 && arr[i-1].freeZeroUsedOne == 0) {
                merge_blocks(i-1);
            }
            return;
        }
    }

    // reached end of for loop

    printf("sorry this memory pointer cant be found\n");
}

void mem_get_stats(mem_stats_ptr mem_stats_ptr){
    mem_stats_ptr->num_blocks_free = freeNum;
    mem_stats_ptr->num_blocks_used = usedNum;

    // if there are used blocks
    if (usedNum > 0){
        int i;
        // if they are all allocated
        if (freeNum == 0) {
            mem_stats_ptr->smallest_block_free = 0;
            mem_stats_ptr->largest_block_free = 0;
            mem_stats_ptr->smallest_block_used = globalMem;
            mem_stats_ptr->largest_block_used = 0;

        }
        else {
            mem_stats_ptr->smallest_block_free = globalMem;
            mem_stats_ptr->largest_block_free = 0;
            mem_stats_ptr->smallest_block_used = globalMem;
            mem_stats_ptr->largest_block_used = 0;
        } 

        for (i = 0; i < usedNum + freeNum; i ++){
            // stats about free blocks
            if (arr[i].freeZeroUsedOne == 0){
                // if current block is less than minFree set new min
                if (arr[i].blockSize < mem_stats_ptr->smallest_block_free) {
                    mem_stats_ptr->smallest_block_free = arr[i].blockSize;

                }
                // if current block is more than maxFree set new max
                if (arr[i].blockSize > mem_stats_ptr->largest_block_free) {
                    mem_stats_ptr->largest_block_free = arr[i].blockSize;
                }
            }
            // stats about allocated blocks
            else {
                // if current block is less than minUsed set new min
                if (arr[i].blockSize < mem_stats_ptr->smallest_block_used) {
                    mem_stats_ptr->smallest_block_used = arr[i].blockSize;

                }
                // if current block is more than maxUsed set new max
                if (arr[i].blockSize > mem_stats_ptr->largest_block_used) {
                    mem_stats_ptr->largest_block_used = arr[i].blockSize;
                }

            }


        }
    }
    else {
        // one free block (all freed or right after mem_init)
        mem_stats_ptr->smallest_block_free = globalMem;
        mem_stats_ptr->largest_block_free = globalMem;
        mem_stats_ptr->smallest_block_used = 0;
        mem_stats_ptr->largest_block_used = 0;
    }
}

void print_stats(char *prefix) {
  mem_stats_struct mem_stats;

  mem_get_stats(&mem_stats);
  printf("mem stats: %s: %d free blocks, %d used blocks, free blocks: smallest=%d largest=%d, used blocks: smallest=%d largest=%d\n",
	 prefix,
	 mem_stats.num_blocks_free,
	 mem_stats.num_blocks_used,
	 mem_stats.smallest_block_free,
	 mem_stats.largest_block_free,
	 mem_stats.smallest_block_used,
	 mem_stats.largest_block_used);
}
