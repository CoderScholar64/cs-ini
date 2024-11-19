#include <stddef.h>
#include <stdint.h>

void *test_malloc(size_t size);
void test_free(void *pointer);

#define CS64_INI_MALLOC(size)  test_malloc(size)
#define CS64_INI_FREE(pointer) test_free(pointer)

// This limit would make this test easier to write.
#define CS64_INI_TOKEN_AMOUNT 4

#define CS64_INI_LIBRARY_IMP
#include "cs64_ini.h"

#include <stdlib.h>
#include <stdio.h>

#define TACKER_ARRAY_LIMIT 64

void *pPointerTrackArray[TACKER_ARRAY_LIMIT];
unsigned pointerTrackAmount = 0;
int disableTestMalloc = 0;

int valid_alloc_test();
int invalid_alloc_test();

int main() {
    int testResult;

    testResult = valid_alloc_test();
    if(testResult != 0)
        return testResult;

    testResult = invalid_alloc_test();
    if(testResult != 0)
        return testResult;

    return testResult;
}

int valid_alloc_test() {
    disableTestMalloc = 0;

    CS64INITokenData *pTokenData = cs64_ini_token_data_alloc();

    if(pTokenData == NULL) {
        printf("Error: pTokenData failed to allocatefor valid_alloc_test case! There is a very slight chance that the program ran out of memory.\n");
        return 1;
    }

    cs64_ini_token_data_free(pTokenData);

    if(pointerTrackAmount != 0) {
        printf("Error: pointerTrackAmount is supposed to be zero after test not be %i.\n", pointerTrackAmount);
        return 2;
    }

    return 0;
}

int invalid_alloc_test() {
    disableTestMalloc = 1;

    CS64INITokenData *pTokenData = cs64_ini_token_data_alloc();

    if(pTokenData != NULL) {
        printf("Error: cs64_ini_token_data_alloc was supposed to fail for invalid_alloc_test case. Not return %p.\n", pTokenData);
        return 1;
    }

    if(pointerTrackAmount != 0) {
        printf("Error: pointerTrackAmount is supposed to be zero after test not be %i.\n", pointerTrackAmount);
        return 2;
    }

    return 0;
}

void *test_malloc(size_t size) {
    if(disableTestMalloc)
        return NULL;

    printf("Log: Allocating pointer of size 0x%x", size);
    void *pointer = malloc(size);
    printf(". Address %p\n", pointer);

    if(pointer == NULL) {
        printf("Error: Pointer for size 0x%x cannot be allocated!\n", size);
        exit(EXIT_FAILURE); // Out of memory
    }

    if(pointerTrackAmount == TACKER_ARRAY_LIMIT) {
        printf("Error: Traker limit reached %i\n", TACKER_ARRAY_LIMIT);
        exit(EXIT_FAILURE); // Tracker ran out of memory.
    }

    pPointerTrackArray[pointerTrackAmount] = pointer;
    pointerTrackAmount++;

    return pointer;
}

void test_free(void *pPointer) {
    printf("Log: Freeing %p\n", pPointer);
    unsigned pointerTrackIndex = 0;
    while(pointerTrackIndex < pointerTrackAmount && pPointerTrackArray[pointerTrackIndex] != pPointer) {
        pointerTrackIndex++;
    }

    if(pointerTrackIndex == pointerTrackAmount) {
        printf("Error: Failed to locate %p within the track array. Probably a double free or a pointer not allocated by test_malloc\n", pPointer);
        exit(EXIT_FAILURE);
    }

    free(pPointer);

    // Remove the pointer from the track array.
    pPointerTrackArray[pointerTrackIndex] = pPointerTrackArray[pointerTrackAmount - 1];
    pointerTrackAmount--;
}
