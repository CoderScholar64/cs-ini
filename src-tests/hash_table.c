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
int mallocPagesLeft = 0;

#define UNIT_TEST_DETAIL_ASSERT(EXP, ON_FAILURE)\
    if(!(EXP)) {\
        printf("Line %d: Statement (%s) failed\n", __LINE__, #EXP);\
        ON_FAILURE\
        exit(__LINE__);\
    }

#define UNIT_TEST_ASSERT(EXP)\
    UNIT_TEST_DETAIL_ASSERT(EXP, {})

#define UNIT_TEST_ASSERT_EQ(VALUE, EXPECT, FORMAT)\
    UNIT_TEST_DETAIL_ASSERT(VALUE == EXPECT, printf(#VALUE " = " FORMAT "\n", VALUE);)

#define UNIT_TEST_ASSERT_NEQ(VALUE, EXPECT, FORMAT)\
    UNIT_TEST_DETAIL_ASSERT(VALUE != EXPECT, printf(#VALUE " = " FORMAT "\n", VALUE);)

#define SET_AVAILABLE_MEM_PAGES(x)\
    mallocPagesLeft = x;

#define UNIT_TEST_MEM_CHECK_ASSERT\
    UNIT_TEST_DETAIL_ASSERT(pointerTrackAmount == 0, printf("pointerTrackAmount is supposed to be zero after test not be %i.\n", pointerTrackAmount);)

// Prototypes here.
void cs64_ini_data_alloc_test();
void cs64_ini_global_variable_test();
void cs64_ini_entry_comment_test();

int main() {
    cs64_ini_data_alloc_test();
    cs64_ini_global_variable_test();
    cs64_ini_entry_comment_test();
    return 0;
}

// Definitions here
void cs64_ini_data_alloc_test() {
    CS64INIData* pData = cs64_ini_data_alloc();

    UNIT_TEST_ASSERT(pData == NULL);

    SET_AVAILABLE_MEM_PAGES(1)
    pData = cs64_ini_data_alloc();

    UNIT_TEST_ASSERT(pData == NULL);

    SET_AVAILABLE_MEM_PAGES(2)
    pData = cs64_ini_data_alloc();

    UNIT_TEST_ASSERT(pData != NULL);
    UNIT_TEST_ASSERT(pData->lastCommentSize == 0);
    UNIT_TEST_ASSERT(pData->pLastComment == NULL);
    UNIT_TEST_ASSERT(pData->pFirstSection == NULL);
    UNIT_TEST_ASSERT(pData->pLastSection == NULL);
    UNIT_TEST_ASSERT(pData->hashTable.pEntries != NULL);
    UNIT_TEST_ASSERT(pData->hashTable.currentEntryAmount     ==  0);
    UNIT_TEST_ASSERT(pData->hashTable.entryCapacity          == 16);
    UNIT_TEST_ASSERT(pData->hashTable.entryCapacityUpLimit   == 13);
    UNIT_TEST_ASSERT(pData->hashTable.entryCapacityDownLimit ==  0);
    UNIT_TEST_ASSERT(pData->globals.pFirstValue == NULL);
    UNIT_TEST_ASSERT(pData->globals.pLastValue  == NULL);

    cs64_ini_data_free(pData);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_global_variable_test() {
    SET_AVAILABLE_MEM_PAGES(2)
    CS64INIData* pData = cs64_ini_data_alloc();
    UNIT_TEST_ASSERT(pData != NULL);

    CS64INIEntry* pEntry = NULL;

    SET_AVAILABLE_MEM_PAGES(1)

    CS64INIEntryState state = cs64_ini_add_value(pData, NULL, (const CS64UTF8*)"Key", (const CS64UTF8*)"Value", &pEntry);
    UNIT_TEST_ASSERT_EQ(state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(pEntry->entryType, CS64_INI_ENTRY_VALUE, "%d"); // TOO short for dynamic RAM usage.

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_entry_comment_test() {
    SET_AVAILABLE_MEM_PAGES(2)
    CS64INIData* pData = cs64_ini_data_alloc();
    UNIT_TEST_ASSERT(pData != NULL);

    CS64INIEntry* pEntry = NULL;
    SET_AVAILABLE_MEM_PAGES(1)
    CS64INIEntryState state = cs64_ini_add_value(pData, NULL, (const CS64UTF8*)"Key", (const CS64UTF8*)"Value", &pEntry);
    UNIT_TEST_ASSERT_EQ(state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT(pEntry != NULL);

    // Test setters and getters for inline comments.
    const CS64UTF8 inlineComment[] = "This is an inline comment";
    SET_AVAILABLE_MEM_PAGES(1)
    state = cs64_ini_set_entry_inline_comment(pEntry, inlineComment);
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);

    const CS64UTF8* retrievedInlineComment = cs64_ini_get_entry_inline_comment(pEntry);
    UNIT_TEST_ASSERT(strcmp((const char*)retrievedInlineComment, (const char*)inlineComment) == 0);

    // Test setters and getters for entry comments.
    const CS64UTF8 entryComment[] = "This is an entry comment";
    SET_AVAILABLE_MEM_PAGES(1)
    state = cs64_ini_set_entry_comment(pEntry, entryComment);
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);

    const CS64UTF8* retrievedEntryComment = cs64_ini_get_entry_comment(pEntry);
    UNIT_TEST_ASSERT(strcmp((const char*)retrievedEntryComment, (const char*)entryComment) == 0);

    cs64_ini_data_free(pData);

    UNIT_TEST_MEM_CHECK_ASSERT
}


void *test_malloc(size_t size) {
    if(mallocPagesLeft <= 0)
        return NULL;
    mallocPagesLeft--;

    printf("Log: Allocating pointer of size 0x%zx", size);
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
