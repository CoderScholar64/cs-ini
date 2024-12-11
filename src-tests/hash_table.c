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
void cs64_ini_single_global_variable_test();
void cs64_ini_static_to_dynamic_variable_test();
void cs64_ini_4_global_variables_test();

int main() {
    cs64_ini_data_alloc_test();
    cs64_ini_single_global_variable_test();
    cs64_ini_static_to_dynamic_variable_test();
    cs64_ini_4_global_variables_test();
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

void cs64_ini_single_global_variable_test() {
    SET_AVAILABLE_MEM_PAGES(2)
    CS64INIData* pData = cs64_ini_data_alloc();
    UNIT_TEST_ASSERT(pData != NULL);

    CS64INIEntry* pEntry = NULL;
    const CS64UTF8 key[] = "Key";

    CS64INIEntryState state = cs64_ini_add_value(pData, NULL, key, (const CS64UTF8*)"Value", &pEntry);
    UNIT_TEST_ASSERT_EQ(state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(pEntry->pNext == NULL);
    UNIT_TEST_ASSERT(pEntry->pPrev == NULL);

    UNIT_TEST_ASSERT(pEntry->type.value.pSection == NULL);

    UNIT_TEST_ASSERT_EQ(pEntry->type.value.nameByteSize, 4, "%zd");
    UNIT_TEST_DETAIL_ASSERT(strcmp((const char*)pEntry->type.value.data.fixed, (const char*)key) == 0, printf("Actually (%s) \n", pEntry->type.value.data.fixed););
    UNIT_TEST_ASSERT_EQ(cs64_ini_get_entry_name(pEntry), pEntry->type.value.data.fixed, "%s");

    UNIT_TEST_ASSERT_EQ(pEntry->type.value.valueByteSize, 6, "%zd");
    UNIT_TEST_DETAIL_ASSERT(strcmp((const char*)pEntry->type.value.data.fixed + pEntry->type.value.nameByteSize, "Value") == 0, printf("Actually (%s) \n", pEntry->type.value.data.fixed + pEntry->type.value.nameByteSize););
    UNIT_TEST_ASSERT_EQ(cs64_ini_get_entry_value(pEntry), pEntry->type.value.data.fixed + pEntry->type.value.nameByteSize, "%s");

    UNIT_TEST_ASSERT_EQ(cs64_ini_get_entry_type(pEntry), CS64_INI_ENTRY_VALUE, "%d");
    UNIT_TEST_ASSERT(cs64_ini_get_next_entry(pEntry) == NULL);
    UNIT_TEST_ASSERT(cs64_ini_get_prev_entry(pEntry) == NULL);
    UNIT_TEST_ASSERT(cs64_ini_get_entry_section(pEntry) == NULL);
    UNIT_TEST_ASSERT(cs64_ini_get_entry_section_name(pEntry) == NULL);

    CS64INIEntry *pEntryReceived = cs64_ini_get_variable(pData, NULL, key);

    UNIT_TEST_ASSERT(pEntry == pEntryReceived);

    state = cs64_ini_add_value(pData, NULL, key, (const CS64UTF8*)"This is not supposed to work!", &pEntry);
    UNIT_TEST_ASSERT_EQ(state, CS64_INI_ENTRY_ERROR_ENTRY_EXISTS, "%d");

    // Test setters and getters for entry comments.
    UNIT_TEST_ASSERT(pEntry->commentSize == 0);
    UNIT_TEST_ASSERT(pEntry->pComment == NULL);
    UNIT_TEST_ASSERT(cs64_ini_get_entry_comment(pEntry) == NULL);

    const CS64UTF8 entryComment[] = "This is an entry comment\nmultilines can be done with this kind of comment!\n";
    SET_AVAILABLE_MEM_PAGES(1)
    state = cs64_ini_set_entry_comment(pEntry, entryComment);
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    const CS64UTF8* retrievedEntryComment = cs64_ini_get_entry_comment(pEntry);
    UNIT_TEST_ASSERT(strcmp((const char*)retrievedEntryComment, (const char*)entryComment) == 0);
    UNIT_TEST_ASSERT_EQ(pEntry->commentSize, sizeof(entryComment) / sizeof(entryComment[0]), "%zd");
    UNIT_TEST_ASSERT(pEntry->pComment != NULL);

    state = cs64_ini_set_entry_comment(pEntry, NULL);
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    state = cs64_ini_set_entry_comment(pEntry, NULL);
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT(pEntry->pComment == NULL);
    UNIT_TEST_ASSERT_EQ(pEntry->commentSize, 0, "%zd");
    UNIT_TEST_ASSERT(cs64_ini_get_entry_comment(pEntry) == NULL);

    SET_AVAILABLE_MEM_PAGES(1)
    state = cs64_ini_set_entry_comment(pEntry, entryComment);

    state = cs64_ini_set_entry_comment(pEntry, (const CS64UTF8*)"");
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    state = cs64_ini_set_entry_comment(pEntry, (const CS64UTF8*)"");
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT(pEntry->pComment == NULL);
    UNIT_TEST_ASSERT_EQ(pEntry->commentSize, 0, "%zd");
    UNIT_TEST_ASSERT(cs64_ini_get_entry_comment(pEntry) == NULL);

    // Test setters and getters for inline comments.
    UNIT_TEST_ASSERT(pEntry->inlineCommentSize == 0);
    UNIT_TEST_ASSERT(pEntry->pInlineComment == NULL);
    UNIT_TEST_ASSERT(cs64_ini_get_entry_inline_comment(pEntry) == NULL);

    const CS64UTF8 inlineComment[] = "This is an inline comment";
    SET_AVAILABLE_MEM_PAGES(1)
    state = cs64_ini_set_entry_inline_comment(pEntry, inlineComment);
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    const CS64UTF8* retrievedInlineComment = cs64_ini_get_entry_inline_comment(pEntry);
    UNIT_TEST_ASSERT(strcmp((const char*)retrievedInlineComment, (const char*)inlineComment) == 0);
    UNIT_TEST_ASSERT(pEntry->inlineCommentSize == sizeof(inlineComment) / sizeof(inlineComment[0]));
    UNIT_TEST_ASSERT(pEntry->pInlineComment != NULL);

    state = cs64_ini_set_entry_inline_comment(pEntry, NULL);
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    state = cs64_ini_set_entry_inline_comment(pEntry, NULL);
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT(pEntry->pInlineComment == NULL);
    UNIT_TEST_ASSERT_EQ(pEntry->inlineCommentSize, 0, "%zd");
    UNIT_TEST_ASSERT(cs64_ini_get_entry_comment(pEntry) == NULL);

    SET_AVAILABLE_MEM_PAGES(1)
    state = cs64_ini_set_entry_inline_comment(pEntry, inlineComment);

    state = cs64_ini_set_entry_inline_comment(pEntry, (const CS64UTF8*)"");
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    state = cs64_ini_set_entry_inline_comment(pEntry, (const CS64UTF8*)"");
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT(pEntry->pInlineComment == NULL);
    UNIT_TEST_ASSERT_EQ(pEntry->inlineCommentSize, 0, "%zd");
    UNIT_TEST_ASSERT(cs64_ini_get_entry_comment(pEntry) == NULL);

    cs64_ini_data_free(pData);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_static_to_dynamic_variable_test() {
    SET_AVAILABLE_MEM_PAGES(2)
    CS64INIData* pData = cs64_ini_data_alloc();
    UNIT_TEST_ASSERT(pData != NULL);

    CS64UTF8 value[CS64_INI_IMP_DETAIL_VALUE_SIZE];

    int i = 0;
    while(i < CS64_INI_IMP_DETAIL_VALUE_SIZE) {
        value[i] = 'b';
        i++;
    }
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 1] = '\0';

    CS64INIEntryState state;
    CS64INIEntry* pEntry = NULL;

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 3] = '\0';
    state = cs64_ini_add_value(pData, NULL, (const CS64UTF8*)"0", value, &pEntry);
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT_EQ(pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 3] = 'b';

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = '\0';
    state = cs64_ini_add_value(pData, NULL, (const CS64UTF8*)"1", value, &pEntry);
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT_EQ(pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = 'b';

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 1] = '\0';
    state = cs64_ini_add_value(pData, NULL, (const CS64UTF8*)"2", value, &pEntry);
    UNIT_TEST_ASSERT(state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT_EQ(pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 1] = 'b';

    cs64_ini_data_free(pData);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_4_global_variables_test() {
    SET_AVAILABLE_MEM_PAGES(2)
    CS64INIData* pData = cs64_ini_data_alloc();
    UNIT_TEST_ASSERT(pData != NULL);

    UNIT_TEST_ASSERT(pData->globals.pFirstValue == NULL);
    UNIT_TEST_ASSERT(pData->globals.pLastValue == NULL);
    UNIT_TEST_ASSERT(pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));

    CS64INIEntry* pEntry[] = {NULL, NULL, NULL, NULL};
    CS64INIEntryState state;

    state = cs64_ini_add_value(pData, NULL, (const CS64UTF8*)"key_0", (const CS64UTF8*)"Value", &pEntry[0]);
    UNIT_TEST_ASSERT_EQ(state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(pEntry[0]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(pEntry[0]->pNext == NULL);
    UNIT_TEST_ASSERT(pEntry[0]->pPrev == NULL);

    UNIT_TEST_ASSERT(pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(pData->globals.pLastValue  == pEntry[0]);
    UNIT_TEST_ASSERT(pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));

    state = cs64_ini_add_value(pData, NULL, (const CS64UTF8*)"key_1", (const CS64UTF8*)"Value", &pEntry[1]);
    UNIT_TEST_ASSERT_EQ(state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(pEntry[1]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(pEntry[1]->pNext == NULL);
    UNIT_TEST_ASSERT(pEntry[1]->pPrev == pEntry[0]);

    UNIT_TEST_ASSERT(pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(pData->globals.pLastValue  == pEntry[1]);
    UNIT_TEST_ASSERT(pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));

    state = cs64_ini_add_value(pData, NULL, (const CS64UTF8*)"key_2", (const CS64UTF8*)"Value", &pEntry[2]);
    UNIT_TEST_ASSERT_EQ(state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(pEntry[2]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(pEntry[2]->pNext == NULL);
    UNIT_TEST_ASSERT(pEntry[2]->pPrev == pEntry[1]);

    UNIT_TEST_ASSERT(pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(pData->globals.pLastValue  == pEntry[2]);
    UNIT_TEST_ASSERT(pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));

    state = cs64_ini_add_value(pData, NULL, (const CS64UTF8*)"key_3", (const CS64UTF8*)"Value", &pEntry[3]);
    UNIT_TEST_ASSERT_EQ(state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(pEntry[3]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(pEntry[3]->pNext == NULL);
    UNIT_TEST_ASSERT(pEntry[3]->pPrev == pEntry[2]);

    UNIT_TEST_ASSERT(pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(pData->globals.pLastValue  == pEntry[3]);
    UNIT_TEST_ASSERT(pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));

    // Check if the chain is correct.

    UNIT_TEST_ASSERT(pEntry[0]->pNext == pEntry[1]);
    UNIT_TEST_ASSERT(pEntry[0]->pPrev == NULL);
    UNIT_TEST_ASSERT(pEntry[0]->pNext == cs64_ini_get_next_entry(pEntry[0]));
    UNIT_TEST_ASSERT(pEntry[0]->pPrev == cs64_ini_get_prev_entry(pEntry[0]));

    UNIT_TEST_ASSERT(pEntry[1]->pNext == pEntry[2]);
    UNIT_TEST_ASSERT(pEntry[1]->pPrev == pEntry[0]);
    UNIT_TEST_ASSERT(pEntry[1]->pNext == cs64_ini_get_next_entry(pEntry[1]));
    UNIT_TEST_ASSERT(pEntry[1]->pPrev == cs64_ini_get_prev_entry(pEntry[1]));

    UNIT_TEST_ASSERT(pEntry[2]->pNext == pEntry[3]);
    UNIT_TEST_ASSERT(pEntry[2]->pPrev == pEntry[1]);
    UNIT_TEST_ASSERT(pEntry[2]->pNext == cs64_ini_get_next_entry(pEntry[2]));
    UNIT_TEST_ASSERT(pEntry[2]->pPrev == cs64_ini_get_prev_entry(pEntry[2]));

    UNIT_TEST_ASSERT(pEntry[3]->pNext == NULL);
    UNIT_TEST_ASSERT(pEntry[3]->pPrev == pEntry[2]);
    UNIT_TEST_ASSERT(pEntry[3]->pNext == cs64_ini_get_next_entry(pEntry[3]));
    UNIT_TEST_ASSERT(pEntry[3]->pPrev == cs64_ini_get_prev_entry(pEntry[3]));

    // Check if the variables can be found!

    UNIT_TEST_ASSERT(cs64_ini_get_variable(pData, NULL, cs64_ini_get_entry_name(pEntry[0])) == pEntry[0])
    UNIT_TEST_ASSERT(cs64_ini_get_variable(pData, NULL, cs64_ini_get_entry_name(pEntry[1])) == pEntry[1])
    UNIT_TEST_ASSERT(cs64_ini_get_variable(pData, NULL, cs64_ini_get_entry_name(pEntry[2])) == pEntry[2])
    UNIT_TEST_ASSERT(cs64_ini_get_variable(pData, NULL, cs64_ini_get_entry_name(pEntry[3])) == pEntry[3])
    UNIT_TEST_ASSERT(cs64_ini_get_variable(pData, (const CS64UTF8*)"", cs64_ini_get_entry_name(pEntry[0])) == pEntry[0])
    UNIT_TEST_ASSERT(cs64_ini_get_variable(pData, (const CS64UTF8*)"", cs64_ini_get_entry_name(pEntry[1])) == pEntry[1])
    UNIT_TEST_ASSERT(cs64_ini_get_variable(pData, (const CS64UTF8*)"", cs64_ini_get_entry_name(pEntry[2])) == pEntry[2])
    UNIT_TEST_ASSERT(cs64_ini_get_variable(pData, (const CS64UTF8*)"", cs64_ini_get_entry_name(pEntry[3])) == pEntry[3])

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
