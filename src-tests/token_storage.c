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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TACKER_ARRAY_LIMIT 64

void *pPointerTrackArray[TACKER_ARRAY_LIMIT];
unsigned pointerTrackAmount = 0;
int disableTestMalloc = 0;

int empty_alloc_test();
int fill_element_test();

int main() {
    int testResult;

    testResult = empty_alloc_test();
    if(testResult != 0)
        return testResult;

    testResult = fill_element_test();
    if(testResult != 0)
        return testResult;

    return testResult;
}

int empty_alloc_test() {
    disableTestMalloc = 1;

    CS64INITokenData *pTokenData = cs64_ini_token_data_alloc();

    if(pTokenData != NULL) {
        printf("Error: cs64_ini_token_data_alloc was supposed to fail for empty_alloc_test case. Not return %p.\n", pTokenData);
        return 1;
    }

    disableTestMalloc = 0;

    pTokenData = cs64_ini_token_data_alloc();

    if(pTokenData == NULL) {
        printf("Error empty_alloc_test: pTokenData failed to allocate! There is a very slight chance that the program ran out of memory.\n");
        return 2;
    }

    if(pTokenData->tokenAmount != 0) {
        printf("Error empty_alloc_test: expected 0, but go pTokenData->tokenAmount %i.\n", pTokenData->tokenAmount);
        return 3;
    }

    if(pTokenData->pLastPage != &pTokenData->firstPage) {
        printf("Error empty_alloc_test: pTokenData->pLastPage %p != &pTokenData->firstPage %p.\n", pTokenData->pLastPage, &pTokenData->firstPage);
        return 4;
    }

    if(pTokenData->firstPage.pNext != NULL) {
        printf("Error empty_alloc_test: pTokenData->firstPage.pNext is supposed to be NULL not %p.\n", pTokenData->firstPage.pNext);
        return 5;
    }

    CS64INIToken *pToken = cs64_ini_token_data_last_token(pTokenData);

    if(pToken != NULL) {
        printf("Error empty_alloc_test: pToken for cs64_ini_token_data_last_token is supposed to be empty not %p.\n", pToken);
        return 6;
    }

    unsigned a = 0;
    while(a < 12) {
        pToken = cs64_ini_token_data_get_token(pTokenData, a);

        if(pToken != NULL) {
            printf("Error empty_alloc_test: pToken for cs64_ini_token_data_get_token(%i) is supposed to be empty not %p.\n", a, pToken);
            return 7 + a;
        }
        a++;
    }

    cs64_ini_token_data_free(pTokenData);

    if(pointerTrackAmount != 0) {
        printf("Error empty_alloc_test: pointerTrackAmount is supposed to be zero after test not be %i.\n", pointerTrackAmount);
        return 19;
    }

    return 0;
}

int fill_element_test() {
    CS64INIToken tokens[12] = {
        {CS64_INI_TOKEN_ENTRY,          0, 12},
        {CS64_INI_TOKEN_VALUE,         12,  5},
        {CS64_INI_TOKEN_COMMENT,       16,  4},
        {CS64_INI_TOKEN_END,           20,  1},
        {CS64_INI_TOKEN_SECTION_START, 21,  1}, // 4
        {CS64_INI_TOKEN_VALUE,         22,  6},
        {CS64_INI_TOKEN_SECTION_END,   28,  1},
        {CS64_INI_TOKEN_END,           29,  1},
        {CS64_INI_TOKEN_ENTRY,         30,  6}, // 8
        {CS64_INI_TOKEN_VALUE,         36,  4},
        {CS64_INI_TOKEN_COMMENT,       41, 10},
        {CS64_INI_TOKEN_END,           51,  1}
    };

    return 0;
}

void *test_malloc(size_t size) {
    if(disableTestMalloc)
        return NULL;

    printf("Log: Allocating pointer of size 0x%x", size);
    void *pointer = malloc(size);
    printf(". Address %p\n", pointer);

    memset(pointer, 0x5c, size);

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
