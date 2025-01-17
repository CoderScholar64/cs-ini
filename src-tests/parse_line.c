#include <stddef.h>
#include <stdint.h>

void *test_malloc(unsigned linePos, size_t size);
void test_free(unsigned linePos, void *pointer);

#define CS64_INI_MALLOC(size)  test_malloc(__LINE__, size)
#define CS64_INI_FREE(pointer) test_free(__LINE__, pointer)

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
int mallocPagesLeft = 0;

#define UNIT_TEST_DETAIL_ASSERT(LOOP_INDEX, EXP, ON_FAILURE)\
    if(!(EXP)) {\
        printf("Line %d, Loop %d: Statement (%s) failed\n", __LINE__, LOOP_INDEX, #EXP);\
        ON_FAILURE\
        exit(__LINE__);\
    }

#define UNIT_TEST_ASSERT(LOOP_INDEX, EXP)\
    UNIT_TEST_DETAIL_ASSERT(LOOP_INDEX, EXP, {})

#define UNIT_TEST_ASSERT_EQ(LOOP_INDEX, VALUE, EXPECT, FORMAT)\
    UNIT_TEST_DETAIL_ASSERT(LOOP_INDEX, VALUE == EXPECT, printf(#VALUE " = " FORMAT "\n", VALUE);)

#define UNIT_TEST_ASSERT_NEQ(LOOP_INDEX, VALUE, EXPECT, FORMAT)\
    UNIT_TEST_DETAIL_ASSERT(LOOP_INDEX, VALUE != EXPECT, printf(#VALUE " = " FORMAT "\n", VALUE);)

#define SET_AVAILABLE_MEM_PAGES(x)\
    mallocPagesLeft = x;

#define UNIT_TEST_MEM_CHECK_ASSERT\
    UNIT_TEST_DETAIL_ASSERT(0, pointerTrackAmount == 0, printf("pointerTrackAmount is supposed to be zero after test not be %i.\n", pointerTrackAmount);)

// Prototypes here.
void cs64_ini_last_comment_test();

int main() {
    cs64_ini_last_comment_test();

    return 0;
}

void cs64_ini_last_comment_test() {
    CS64UTF8 buffer[1024];

    CS64INIParserContext parserContext;

    parserContext.stringBufferLimit = sizeof(buffer) / 2;
    parserContext.pStringBuffer = buffer;
    parserContext.pValueBuffer  = buffer + parserContext.stringBufferLimit;

    SET_AVAILABLE_MEM_PAGES(2)
    parserContext.pData = cs64_ini_data_alloc();
    parserContext.tokenOffset = 0;
    parserContext.pSection = NULL;

    CS64UTF8 fileData[] = "; First Test";
    CS64Size fileDataSize = sizeof(fileData) / sizeof(fileData[0]) - 1;

    SET_AVAILABLE_MEM_PAGES(1)
    CS64INITokenResult tokenResult = cs64_ini_lexer(fileData, fileDataSize);

    UNIT_TEST_ASSERT_NEQ(0, tokenResult.state, CS64_INI_LEXER_NO_MEMORY_ERROR, "%d");

    parserContext.pSource = fileData;
    parserContext.pTokenResult = &tokenResult;

    cs64_ini_data_free(parserContext.pData);
    cs64_ini_lexer_free(parserContext.pTokenResult);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void *test_malloc(unsigned linePos, size_t size) {
    if(mallocPagesLeft <= 0)
        return NULL;
    mallocPagesLeft--;

    printf("Log: Line %u Allocating pointer of size 0x%zx", linePos, size);
    void *pointer = malloc(size);
    printf(". Address %p\n", pointer);

    memset(pointer, 0x5c, size);

    if(pointer == NULL) {
        printf("Error: Pointer for size 0x%zx cannot be allocated!\n", size);
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

void test_free(unsigned linePos, void *pPointer) {
    printf("Log: Line %u Freeing %p\n", linePos, pPointer);
    unsigned pointerTrackIndex = 0;
    while(pointerTrackIndex < pointerTrackAmount && pPointerTrackArray[pointerTrackIndex] != pPointer) {
        pointerTrackIndex++;
    }

    if(pointerTrackIndex == pointerTrackAmount) {
        printf("Error: Failed to locate %p within the track array. This is most likely a double free test_malloc. Array Limit = %d. Array Amount = %d \n", pPointer, TACKER_ARRAY_LIMIT, pointerTrackAmount);
        exit(EXIT_FAILURE);
    }

    free(pPointer);

    // Remove the pointer from the track array.
    pPointerTrackArray[pointerTrackIndex] = pPointerTrackArray[pointerTrackAmount - 1];
    pointerTrackAmount--;
}
