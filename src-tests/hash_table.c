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
void cs64_ini_data_alloc_test();
void cs64_ini_data_reserve_empty_test();
void cs64_ini_single_global_variable_test();
void cs64_ini_variable_declarations_test();
void cs64_ini_variable_capacity_test();
void cs64_ini_variable_change_test();
void cs64_ini_variable_rehash_test();
void cs64_ini_4_data_test();
void cs64_ini_display_entry(const CS64INIEntry *const pEntry);
void cs64_ini_display_data(const CS64INIData *const pData);

int main() {
    cs64_ini_data_alloc_test();
    cs64_ini_data_reserve_empty_test();
    cs64_ini_single_global_variable_test();
    cs64_ini_variable_declarations_test();
    cs64_ini_variable_capacity_test();
    cs64_ini_variable_change_test();
    cs64_ini_variable_rehash_test();
    cs64_ini_4_data_test();
    return 0;
}

// Definitions here
void cs64_ini_data_alloc_test() {
    CS64INIData* pData = cs64_ini_data_alloc();

    UNIT_TEST_ASSERT(0, pData == NULL);

    SET_AVAILABLE_MEM_PAGES(1)
    pData = cs64_ini_data_alloc();

    UNIT_TEST_ASSERT(0, pData == NULL);

    SET_AVAILABLE_MEM_PAGES(2)
    pData = cs64_ini_data_alloc();

    UNIT_TEST_ASSERT(0, pData != NULL);
    UNIT_TEST_ASSERT(0, pData->lastCommentSize == 0);
    UNIT_TEST_ASSERT(0, pData->pLastComment == NULL);
    UNIT_TEST_ASSERT(0, pData->pFirstSection == NULL);
    UNIT_TEST_ASSERT(0, pData->pLastSection == NULL);
    UNIT_TEST_ASSERT(0, pData->hashTable.pEntries != NULL);
    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount     ==  0);
    UNIT_TEST_ASSERT(0, pData->hashTable.entryCapacity          == 16);
    UNIT_TEST_ASSERT(0, pData->hashTable.entryCapacityUpLimit   == 13);
    UNIT_TEST_ASSERT(0, pData->hashTable.entryCapacityDownLimit ==  0);
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == NULL);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == NULL);

    cs64_ini_data_free(pData);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_data_reserve_empty_test() {
    int returnResult;

    returnResult = cs64_ini_data_reserve(NULL, 32);
    UNIT_TEST_ASSERT_EQ(0, returnResult, -1, "%d");

    // This CS64INIData structure is built to be invalid on purpose!
    CS64INIData badData;
    badData.hashTable.pEntries = NULL;
    badData.hashTable.entryCapacity = 64;
    badData.hashTable.currentEntryAmount = 32;
    badData.hashTable.entryCapacityUpLimit = 52;
    badData.hashTable.entryCapacityDownLimit = 26;
    badData.lastCommentSize = 0;
    badData.pLastComment    = NULL;
    badData.globals.pFirstValue = NULL;
    badData.globals.pLastValue  = NULL;
    badData.pFirstSection = NULL;
    badData.pLastSection  = NULL;

    // Just in case pEntries end up as a NULL.
    returnResult = cs64_ini_data_reserve(&badData, 32);
    UNIT_TEST_ASSERT_EQ(0, returnResult, -2, "%d");

    // Cancel if the amount for reserving if the amount of reserving if the entry amounts do not increase.
    badData.hashTable.pEntries = (CS64INIEntry*)&returnResult;
    returnResult = cs64_ini_data_reserve(&badData, 32);
    UNIT_TEST_ASSERT_EQ(0, returnResult, -3, "%d");

    SET_AVAILABLE_MEM_PAGES(2)
    CS64INIData* pData = cs64_ini_data_alloc();
    UNIT_TEST_ASSERT(0, pData != NULL);

    CS64INIData backupData = *pData;

    // Malloc failure case
    returnResult = cs64_ini_data_reserve(pData, 32);
    UNIT_TEST_ASSERT_EQ(0, returnResult, -4, "%d");
    UNIT_TEST_ASSERT(0, memcmp(&backupData, pData, sizeof(CS64INIData)) == 0);

    // Malloc okay on empty case
    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 17);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,      0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          32, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   26, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit, 13, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 26);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,      0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          32, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   26, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit, 13, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 0);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,      0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          16, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   13, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit,  0, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 27);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,      0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          64, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   52, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit, 26, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 13);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,      0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          16, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   13, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit,  0, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 52);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,      0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          64, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   52, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit, 26, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 53);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,       0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          128, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   104, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit,  52, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 104);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,       0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          128, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   104, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit,  52, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 105);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,       0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          256, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   208, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit, 104, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 208);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,       0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          256, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   208, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit, 104, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 209);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,       0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          512, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   416, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit, 208, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 416);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,       0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          512, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   416, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit, 208, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 417);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,        0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          1024, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,    832, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit,  416, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 832);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,        0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          1024, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,    832, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit,  416, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 833);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,        0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          1536, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   1248, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit,  832, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 1248);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,        0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          1536, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   1248, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit,  832, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 1249);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,        0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          2048, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   1664, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit, 1248, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 1664);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,        0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          2048, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   1664, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit, 1248, "%zd");
    backupData = *pData;

    SET_AVAILABLE_MEM_PAGES(1)
    returnResult = cs64_ini_data_reserve(pData, 1665);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_NEQ(0, pData->hashTable.pEntries, backupData.hashTable.pEntries, "%p");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.currentEntryAmount,        0, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity,          2560, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityUpLimit,   2080, "%zd");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacityDownLimit, 1664, "%zd");
    backupData = *pData;

    cs64_ini_data_free(pData);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_single_global_variable_test() {
    SET_AVAILABLE_MEM_PAGES(2)
    CS64INIData* pData = cs64_ini_data_alloc();
    UNIT_TEST_ASSERT(0, pData != NULL);

    CS64INIEntry* pEntry = NULL;
    const CS64UTF8 key[] = "Key";

    CS64INIEntryState state = cs64_ini_add_variable(pData, NULL, key, (const CS64UTF8*)"Value", &pEntry);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pEntry->pNext == NULL);
    UNIT_TEST_ASSERT(0, pEntry->pPrev == NULL);

    UNIT_TEST_ASSERT(0, pEntry->type.value.pSection == NULL);

    UNIT_TEST_ASSERT_EQ(0, pEntry->type.value.nameByteSize, 4, "%zd");
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)pEntry->type.value.data.fixed, (const char*)key) == 0, printf("Actually (%s) \n", pEntry->type.value.data.fixed););
    UNIT_TEST_ASSERT_EQ(0, cs64_ini_get_entry_name(pEntry), pEntry->type.value.data.fixed, "%s");

    UNIT_TEST_ASSERT_EQ(0, pEntry->type.value.valueByteSize, 6, "%zd");
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)pEntry->type.value.data.fixed + pEntry->type.value.nameByteSize, "Value") == 0, printf("Actually (%s) \n", pEntry->type.value.data.fixed + pEntry->type.value.nameByteSize););
    UNIT_TEST_ASSERT_EQ(0, cs64_ini_get_entry_value(pEntry), pEntry->type.value.data.fixed + pEntry->type.value.nameByteSize, "%s");

    UNIT_TEST_ASSERT_EQ(0, cs64_ini_get_entry_type(pEntry), CS64_INI_ENTRY_VALUE, "%d");
    UNIT_TEST_ASSERT(0, cs64_ini_get_next_entry(pEntry) == NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pEntry) == NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pEntry) == NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section_name(pEntry) == NULL);

    // Test setters and getters for entry comments.
    UNIT_TEST_ASSERT(0, pEntry->commentSize == 0);
    UNIT_TEST_ASSERT(0, pEntry->pComment == NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_comment(pEntry) == NULL);

    const CS64UTF8 entryComment[] = "This is an entry comment\nmultilines can be done with this kind of comment!\n";
    SET_AVAILABLE_MEM_PAGES(1)
    state = cs64_ini_set_entry_comment(pEntry, entryComment);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    const CS64UTF8* retrievedEntryComment = cs64_ini_get_entry_comment(pEntry);
    UNIT_TEST_ASSERT(0, strcmp((const char*)retrievedEntryComment, (const char*)entryComment) == 0);
    UNIT_TEST_ASSERT_EQ(0, pEntry->commentSize, sizeof(entryComment) / sizeof(entryComment[0]), "%zd");
    UNIT_TEST_ASSERT(0, pEntry->pComment != NULL);

    state = cs64_ini_set_entry_comment(pEntry, NULL);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    state = cs64_ini_set_entry_comment(pEntry, NULL);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT(0, pEntry->pComment == NULL);
    UNIT_TEST_ASSERT_EQ(0, pEntry->commentSize, 0, "%zd");
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_comment(pEntry) == NULL);

    SET_AVAILABLE_MEM_PAGES(1)
    state = cs64_ini_set_entry_comment(pEntry, entryComment);

    state = cs64_ini_set_entry_comment(pEntry, (const CS64UTF8*)"");
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    state = cs64_ini_set_entry_comment(pEntry, (const CS64UTF8*)"");
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT(0, pEntry->pComment == NULL);
    UNIT_TEST_ASSERT_EQ(0, pEntry->commentSize, 0, "%zd");
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_comment(pEntry) == NULL);

    // Test setters and getters for inline comments.
    UNIT_TEST_ASSERT(0, pEntry->inlineCommentSize == 0);
    UNIT_TEST_ASSERT(0, pEntry->pInlineComment == NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_inline_comment(pEntry) == NULL);

    const CS64UTF8 inlineComment[] = "This is an inline comment";
    SET_AVAILABLE_MEM_PAGES(1)
    state = cs64_ini_set_entry_inline_comment(pEntry, inlineComment);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    const CS64UTF8* retrievedInlineComment = cs64_ini_get_entry_inline_comment(pEntry);
    UNIT_TEST_ASSERT(0, strcmp((const char*)retrievedInlineComment, (const char*)inlineComment) == 0);
    UNIT_TEST_ASSERT(0, pEntry->inlineCommentSize == sizeof(inlineComment) / sizeof(inlineComment[0]));
    UNIT_TEST_ASSERT(0, pEntry->pInlineComment != NULL);

    state = cs64_ini_set_entry_inline_comment(pEntry, NULL);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    state = cs64_ini_set_entry_inline_comment(pEntry, NULL);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT(0, pEntry->pInlineComment == NULL);
    UNIT_TEST_ASSERT_EQ(0, pEntry->inlineCommentSize, 0, "%zd");
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_comment(pEntry) == NULL);

    SET_AVAILABLE_MEM_PAGES(1)
    state = cs64_ini_set_entry_inline_comment(pEntry, inlineComment);

    state = cs64_ini_set_entry_inline_comment(pEntry, (const CS64UTF8*)"");
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    state = cs64_ini_set_entry_inline_comment(pEntry, (const CS64UTF8*)"");
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT(0, pEntry->pInlineComment == NULL);
    UNIT_TEST_ASSERT_EQ(0, pEntry->inlineCommentSize, 0, "%zd");
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_comment(pEntry) == NULL);

    CS64INIEntry *pEntryReceived = cs64_ini_get_variable(pData, NULL, key);

    UNIT_TEST_ASSERT(0, pEntry == pEntryReceived);

    state = cs64_ini_add_variable(pData, NULL, key, (const CS64UTF8*)"This is not supposed to work!", &pEntry);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_ERROR_ENTRY_EXISTS, "%d");

    cs64_ini_data_free(pData);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_variable_declarations_test() {
    SET_AVAILABLE_MEM_PAGES(2)
    CS64INIData* pData = cs64_ini_data_alloc();
    UNIT_TEST_ASSERT(0, pData != NULL);

    CS64UTF8 value[CS64_INI_IMP_DETAIL_VALUE_SIZE];

    int i = 0;
    while(i < CS64_INI_IMP_DETAIL_VALUE_SIZE) {
        value[i] = 'b';
        i++;
    }
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 1] = '\0';

    CS64INIEntryState state;
    CS64INIEntry* pEntry = NULL;

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 4] = '\0';
    state = cs64_ini_add_variable(pData, NULL, (const CS64UTF8*)"0", value, &pEntry);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 4] = 'b';

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 3] = '\0';
    state = cs64_ini_add_variable(pData, NULL, (const CS64UTF8*)"1", value, &pEntry);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 3] = 'b';

    // Test lack of memory case
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = '\0';
    state = cs64_ini_add_variable(pData, NULL, (const CS64UTF8*)"2", value, &pEntry);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_ERROR_OUT_OF_SPACE);
    UNIT_TEST_ASSERT(0, pEntry == NULL);
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = 'b';

    SET_AVAILABLE_MEM_PAGES(2)
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = '\0';
    state = cs64_ini_add_variable(pData, NULL, (const CS64UTF8*)"2", value, &pEntry);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_DYNAMIC_VALUE, "TOO big for static RAM usage %d");
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = 'b';

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 1] = '\0';
    state = cs64_ini_add_variable(pData, NULL, (const CS64UTF8*)"3", value, &pEntry);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_DYNAMIC_VALUE, "TOO big for static RAM usage %d");

    // Now, value will be used as the key.

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 4] = '\0';
    state = cs64_ini_add_variable(pData, NULL, value, (const CS64UTF8*)"v", &pEntry);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 4] = 'b';

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 3] = '\0';
    state = cs64_ini_add_variable(pData, NULL, value, (const CS64UTF8*)"v", &pEntry);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 3] = 'b';

    // Test lack of memory case
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = '\0';
    state = cs64_ini_add_variable(pData, NULL, value, (const CS64UTF8*)"v", &pEntry);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_ERROR_OUT_OF_SPACE);
    UNIT_TEST_ASSERT(0, pEntry == NULL);
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = 'b';

    SET_AVAILABLE_MEM_PAGES(2)
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = '\0';
    state = cs64_ini_add_variable(pData, NULL, value, (const CS64UTF8*)"v", &pEntry);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_DYNAMIC_VALUE, "TOO big for static RAM usage %d");
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = 'b';

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 1] = '\0';
    state = cs64_ini_add_variable(pData, NULL, value, (const CS64UTF8*)"v", &pEntry);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_DYNAMIC_VALUE, "TOO big for static RAM usage %d");

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = '\0';

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 3] = 'A';
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    state = cs64_ini_add_variable(pData, NULL, value, (const CS64UTF8*)"", &pEntry);
    UNIT_TEST_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_name(pEntry), (const char*)value) == 0);
    UNIT_TEST_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_value(pEntry), (const char*)"") == 0);
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 3] = 'B';
    state = cs64_ini_add_variable(pData, NULL, value, NULL, &pEntry);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_name(pEntry), (const char*)value) == 0);
    UNIT_TEST_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_value(pEntry), (const char*)"") == 0);
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 1] = '\0';
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = 'b';

    SET_AVAILABLE_MEM_PAGES(2)
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 3] = 'C';
    state = cs64_ini_add_variable(pData, NULL, value, (const CS64UTF8*)"", &pEntry);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_name(pEntry), (const char*)value) == 0);
    UNIT_TEST_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_value(pEntry), (const char*)"") == 0);
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_DYNAMIC_VALUE, "TOO big for static RAM usage %d");

    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 3] = 'D';
    state = cs64_ini_add_variable(pData, NULL, value, NULL, &pEntry);
    UNIT_TEST_ASSERT(0, state == CS64_INI_ENTRY_SUCCESS);
    UNIT_TEST_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_name(pEntry), (const char*)value) == 0);
    UNIT_TEST_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_value(pEntry), (const char*)"") == 0);
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_DYNAMIC_VALUE, "TOO big for static RAM usage %d");

    cs64_ini_data_free(pData);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_variable_capacity_test() {
    SET_AVAILABLE_MEM_PAGES(2)
    CS64INIData* pData = cs64_ini_data_alloc();
    UNIT_TEST_ASSERT(0, pData != NULL);

    CS64UTF8 name[2] = "A";

    CS64INIEntryState state;
    CS64INIEntry* pEntry = NULL;

    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity, 16, "%zd");

    int loop = 0;
    while(loop < pData->hashTable.entryCapacity) {
        name[0] = 'A' + loop;

        state = cs64_ini_add_variable(pData, NULL, name, (const CS64UTF8*)"val", &pEntry);
        UNIT_TEST_ASSERT(loop, state == CS64_INI_ENTRY_SUCCESS);
        UNIT_TEST_ASSERT_EQ(loop, pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");

        state = cs64_ini_add_variable(pData, NULL, name, (const CS64UTF8*)"val", &pEntry);
        if(pData->hashTable.currentEntryAmount != pData->hashTable.entryCapacity) {
            UNIT_TEST_ASSERT_EQ(loop, state, CS64_INI_ENTRY_ERROR_ENTRY_EXISTS, "%d");
        }
        else {
            UNIT_TEST_ASSERT_EQ(loop, state, CS64_INI_ENTRY_ERROR_OUT_OF_SPACE, "%d");
        }

        loop++;
    }

    SET_AVAILABLE_MEM_PAGES(1)
    int returnResult = cs64_ini_data_reserve(pData, pData->hashTable.entryCapacity + 1);
    UNIT_TEST_ASSERT_EQ(0, returnResult, 0, "%d");
    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity, 32, "%zd");

    int amount = loop;

    loop = 0;

    CS64INIEntry *pGlobalEntry = cs64_ini_get_first_global_value(pData);
    CS64INIEntry *pLastEntry = NULL;

    while(loop < amount) {
        name[0] = 'A' + loop;

        pEntry = cs64_ini_get_variable(pData, NULL, name);

        UNIT_TEST_ASSERT(loop, pEntry != NULL);
        UNIT_TEST_ASSERT(loop, pEntry == pGlobalEntry);
        UNIT_TEST_ASSERT(loop, cs64_ini_get_prev_entry(pEntry) == pLastEntry);

        loop++;
        pLastEntry = pEntry;
        pGlobalEntry = cs64_ini_get_next_entry(pGlobalEntry);
    }

    cs64_ini_data_free(pData);
    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_variable_rehash_test() {
    SET_AVAILABLE_MEM_PAGES(3)
    CS64INIData* pData = cs64_ini_data_alloc();
    UNIT_TEST_ASSERT(0, pData != NULL);

    CS64UTF8 name[2] = " ";

    CS64INIEntryState state;
    CS64INIEntry* pEntry = NULL;

    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity, 16, "%zd");

    int loop = 0;
    while(loop < pData->hashTable.entryCapacity) {
        name[0] = ' ' + loop;

        state = cs64_ini_add_variable(pData, NULL, name, (const CS64UTF8*)"val", &pEntry);
        UNIT_TEST_ASSERT(loop, state == CS64_INI_ENTRY_SUCCESS);
        UNIT_TEST_ASSERT_EQ(loop, pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");

        state = cs64_ini_add_variable(pData, NULL, name, (const CS64UTF8*)"val", &pEntry);
        if(pData->hashTable.currentEntryAmount != pData->hashTable.entryCapacity) {
            UNIT_TEST_ASSERT_EQ(loop, state, CS64_INI_ENTRY_ERROR_ENTRY_EXISTS, "%d");
        }
        else {
            UNIT_TEST_ASSERT_EQ(loop, state, CS64_INI_ENTRY_ERROR_OUT_OF_SPACE, "%d");
        }

        loop++;
    }

    UNIT_TEST_ASSERT_EQ(0, pData->hashTable.entryCapacity, 32, "%zd");

    CS64INIEntry *pGlobalEntry = cs64_ini_get_first_global_value(pData);
    CS64INIEntry *pLastEntry = NULL;

    loop = 0;
    while(loop < pData->hashTable.entryCapacity) {
        name[0] = ' ' + loop;

        pEntry = cs64_ini_get_variable(pData, NULL, name);

        UNIT_TEST_ASSERT(loop, pEntry != NULL);
        UNIT_TEST_ASSERT(loop, pEntry == pGlobalEntry);
        UNIT_TEST_ASSERT(loop, cs64_ini_get_prev_entry(pEntry) == pLastEntry);

        loop++;
        pLastEntry = pEntry;
        pGlobalEntry = cs64_ini_get_next_entry(pGlobalEntry);
    }

    cs64_ini_data_free(pData);
    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_variable_change_test() {
    SET_AVAILABLE_MEM_PAGES(2)
    CS64INIData* pData = cs64_ini_data_alloc();
    UNIT_TEST_ASSERT(0, pData != NULL);

    CS64UTF8 value[CS64_INI_IMP_DETAIL_VALUE_SIZE];

    int i = 0;
    while(i < CS64_INI_IMP_DETAIL_VALUE_SIZE) {
        value[i] = 'v';
        i++;
    }
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 1] = '\0';
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 2] = '\0';
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 3] = '\0';
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 4] = '\0';
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 5] = '\0';

    CS64INIEntryState state;
    CS64INIEntry* pEntry = NULL;

    CS64INIEntry wrongEntryType;
    wrongEntryType.pNext = NULL;
    wrongEntryType.pPrev = NULL;
    wrongEntryType.commentSize = 0;
    wrongEntryType.pComment = NULL;
    wrongEntryType.inlineCommentSize = 0;
    wrongEntryType.pInlineComment = NULL;
    wrongEntryType.type.section.header.pFirstValue = NULL;
    wrongEntryType.type.section.header.pLastValue = NULL;
    wrongEntryType.type.section.nameByteSize = 4;
    wrongEntryType.type.section.name.fixed[0] = 'N';
    wrongEntryType.type.section.name.fixed[1] = '/';
    wrongEntryType.type.section.name.fixed[2] = 'A';
    wrongEntryType.type.section.name.fixed[3] = '\0';

    state = cs64_ini_set_entry_value(NULL, value);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_ERROR_DATA_NULL, "%d");
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_value(NULL) == NULL);

    wrongEntryType.entryType = CS64_INI_ENTRY_EMPTY;
    state = cs64_ini_set_entry_value(&wrongEntryType, value);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_ERROR_DATA_NULL, "%d");
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_value(NULL) == NULL);

    wrongEntryType.entryType = CS64_INI_ENTRY_WAS_OCCUPIED;
    state = cs64_ini_set_entry_value(&wrongEntryType, value);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_ERROR_DATA_NULL, "%d");
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_value(NULL) == NULL);

    wrongEntryType.entryType = CS64_INI_ENTRY_DYNAMIC_SECTION;
    state = cs64_ini_set_entry_value(&wrongEntryType, value);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_ERROR_DATA_NULL, "%d");
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_value(NULL) == NULL);

    state = cs64_ini_add_variable(pData, NULL, (const CS64UTF8*)"null", NULL, &pEntry);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT(0, pEntry->type.value.pSection      == NULL);
    UNIT_TEST_ASSERT(0, pEntry->type.value.nameByteSize  == 5);
    UNIT_TEST_ASSERT(0, pEntry->type.value.valueByteSize == 1);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_value(pEntry) != NULL);
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_value(pEntry), (const char*)"") == 0,
        printf(" a = %s\n b = %s\n", (const char*)cs64_ini_get_entry_value(pEntry), (const char*)""););

    state = cs64_ini_add_variable(pData, NULL, (const CS64UTF8*)"key", (const CS64UTF8*)"", &pEntry);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT(0, pEntry->type.value.pSection      == NULL);
    UNIT_TEST_ASSERT(0, pEntry->type.value.nameByteSize  == 4);
    UNIT_TEST_ASSERT(0, pEntry->type.value.valueByteSize == 1);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_value(pEntry) != NULL);
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_value(pEntry), (const char*)"") == 0,
        printf(" a = %s\n b = %s\n", (const char*)cs64_ini_get_entry_value(pEntry), (const char*)""););

    // static to static case.
    state = cs64_ini_set_entry_value(pEntry, value);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO small for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pEntry->type.value.pSection      == NULL);
    UNIT_TEST_ASSERT(0, pEntry->type.value.nameByteSize  ==  4);
    UNIT_TEST_ASSERT(0, pEntry->type.value.valueByteSize == 12);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_value(pEntry) != NULL);
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_value(pEntry), (const char*)value) == 0,
        printf(" a = %s\n b = %s\n", (const char*)cs64_ini_get_entry_value(pEntry), (const char*)value););

    // static to dynamic case.
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 5] = 'a';
    state = cs64_ini_set_entry_value(pEntry, value);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_ERROR_OUT_OF_SPACE, "%d");
    SET_AVAILABLE_MEM_PAGES(1)

    state = cs64_ini_set_entry_value(pEntry, value);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_DYNAMIC_VALUE, "TOO big for static RAM usage %d");
    UNIT_TEST_ASSERT(0, pEntry->type.value.pSection      == NULL);
    UNIT_TEST_ASSERT(0, pEntry->type.value.nameByteSize  ==  4);
    UNIT_TEST_ASSERT(0, pEntry->type.value.valueByteSize == 13);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_value(pEntry) != NULL);
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_value(pEntry), (const char*)value) == 0,
        printf(" a = %s\n b = %s\n", (const char*)cs64_ini_get_entry_value(pEntry), (const char*)value););

    // dynamic to dynamic case.
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 5] = 'b';
    state = cs64_ini_set_entry_value(pEntry, value);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_ERROR_OUT_OF_SPACE, "%d");
    SET_AVAILABLE_MEM_PAGES(1)

    state = cs64_ini_set_entry_value(pEntry, value);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_DYNAMIC_VALUE, "TOO big for static RAM usage %d");
    UNIT_TEST_ASSERT(0, pEntry->type.value.pSection      == NULL);
    UNIT_TEST_ASSERT(0, pEntry->type.value.nameByteSize  ==  4);
    UNIT_TEST_ASSERT(0, pEntry->type.value.valueByteSize == 13);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_value(pEntry) != NULL);
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_value(pEntry), (const char*)value) == 0,
        printf(" a = %s\n b = %s\n", (const char*)cs64_ini_get_entry_value(pEntry), (const char*)value););

    // dynamic to static case.
    value[CS64_INI_IMP_DETAIL_VALUE_SIZE - 5] = '\0';
    state = cs64_ini_set_entry_value(pEntry, value);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pEntry->entryType, CS64_INI_ENTRY_VALUE, "TOO small for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pEntry->type.value.pSection      == NULL);
    UNIT_TEST_ASSERT(0, pEntry->type.value.nameByteSize  ==  4);
    UNIT_TEST_ASSERT(0, pEntry->type.value.valueByteSize == 12);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_value(pEntry) != NULL);
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)cs64_ini_get_entry_value(pEntry), (const char*)value) == 0,
        printf(" a = %s\n b = %s\n", (const char*)cs64_ini_get_entry_value(pEntry), (const char*)value););

    cs64_ini_data_free(pData);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_4_data_test() {
    SET_AVAILABLE_MEM_PAGES(2)
    CS64INIData* pData = cs64_ini_data_alloc();
    UNIT_TEST_ASSERT(0, pData != NULL);

    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == NULL);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue == NULL);
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 0);

    CS64INIEntry* pEntry[] = {NULL, NULL, NULL, NULL};
    CS64INIEntryState state;

    state = cs64_ini_add_variable(pData, NULL, (const CS64UTF8*)"key_0", (const CS64UTF8*)"Value", &pEntry[0]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pEntry[0]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pEntry[0]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pEntry[0]->pPrev == NULL);

    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == pEntry[0]);
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 1);

    state = cs64_ini_add_variable(pData, NULL, (const CS64UTF8*)"key_1", (const CS64UTF8*)"Value", &pEntry[1]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pEntry[1]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pEntry[1]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pEntry[1]->pPrev == pEntry[0]);

    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == pEntry[1]);
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 2);

    state = cs64_ini_add_variable(pData, NULL, (const CS64UTF8*)"key_2", (const CS64UTF8*)"Value", &pEntry[2]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pEntry[2]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pEntry[2]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pEntry[2]->pPrev == pEntry[1]);

    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == pEntry[2]);
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 3);

    state = cs64_ini_add_variable(pData, NULL, (const CS64UTF8*)"key_3", (const CS64UTF8*)"Value", &pEntry[3]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pEntry[3]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pEntry[3]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pEntry[3]->pPrev == pEntry[2]);

    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == pEntry[3]);
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 4);

    // Check if the chain is correct.

    UNIT_TEST_ASSERT(0, pEntry[0]->pNext == pEntry[1]);
    UNIT_TEST_ASSERT(0, pEntry[0]->pPrev == NULL);
    UNIT_TEST_ASSERT(0, pEntry[0]->pNext == cs64_ini_get_next_entry(pEntry[0]));
    UNIT_TEST_ASSERT(0, pEntry[0]->pPrev == cs64_ini_get_prev_entry(pEntry[0]));

    UNIT_TEST_ASSERT(0, pEntry[1]->pNext == pEntry[2]);
    UNIT_TEST_ASSERT(0, pEntry[1]->pPrev == pEntry[0]);
    UNIT_TEST_ASSERT(0, pEntry[1]->pNext == cs64_ini_get_next_entry(pEntry[1]));
    UNIT_TEST_ASSERT(0, pEntry[1]->pPrev == cs64_ini_get_prev_entry(pEntry[1]));

    UNIT_TEST_ASSERT(0, pEntry[2]->pNext == pEntry[3]);
    UNIT_TEST_ASSERT(0, pEntry[2]->pPrev == pEntry[1]);
    UNIT_TEST_ASSERT(0, pEntry[2]->pNext == cs64_ini_get_next_entry(pEntry[2]));
    UNIT_TEST_ASSERT(0, pEntry[2]->pPrev == cs64_ini_get_prev_entry(pEntry[2]));

    UNIT_TEST_ASSERT(0, pEntry[3]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pEntry[3]->pPrev == pEntry[2]);
    UNIT_TEST_ASSERT(0, pEntry[3]->pNext == cs64_ini_get_next_entry(pEntry[3]));
    UNIT_TEST_ASSERT(0, pEntry[3]->pPrev == cs64_ini_get_prev_entry(pEntry[3]));

    // Check if the variables can be found!

    int loop = 0;
    while(loop < sizeof(pEntry) / sizeof(pEntry[0])) {
        UNIT_TEST_ASSERT(loop, cs64_ini_get_variable(pData, NULL, cs64_ini_get_entry_name(pEntry[loop])) == pEntry[loop])
        UNIT_TEST_ASSERT(loop, cs64_ini_get_variable(pData, (const CS64UTF8*)"", cs64_ini_get_entry_name(pEntry[loop])) == pEntry[loop])
        loop++;
    }

    // Sections

    CS64INIEntry* pSectionEntry[] = {NULL, NULL, NULL, NULL};

    UNIT_TEST_ASSERT(0, pData->pFirstSection == NULL);
    UNIT_TEST_ASSERT(0, pData->pLastSection  == NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section(pData) == pData->pFirstSection);

    state = cs64_ini_add_section(pData, (const CS64UTF8*)"s0", &pSectionEntry[0]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionEntry[0]->entryType, CS64_INI_ENTRY_SECTION, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pSectionEntry[0]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pSectionEntry[0]->pPrev == NULL);
    UNIT_TEST_ASSERT(0, pSectionEntry[0]->pNext == cs64_ini_get_next_entry(pSectionEntry[0]));
    UNIT_TEST_ASSERT(0, pSectionEntry[0]->pPrev == cs64_ini_get_prev_entry(pSectionEntry[0]));

    UNIT_TEST_ASSERT(0, pData->pFirstSection == pSectionEntry[0]);
    UNIT_TEST_ASSERT(0, pData->pLastSection  == pSectionEntry[0]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section(pData) == pData->pFirstSection);

    state = cs64_ini_add_section(pData, (const CS64UTF8*)"s1", &pSectionEntry[1]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionEntry[1]->entryType, CS64_INI_ENTRY_SECTION, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pSectionEntry[1]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pSectionEntry[1]->pPrev == pSectionEntry[0]);
    UNIT_TEST_ASSERT(0, pSectionEntry[1]->pNext == cs64_ini_get_next_entry(pSectionEntry[1]));
    UNIT_TEST_ASSERT(0, pSectionEntry[1]->pPrev == cs64_ini_get_prev_entry(pSectionEntry[1]));

    UNIT_TEST_ASSERT(0, pData->pFirstSection == pSectionEntry[0]);
    UNIT_TEST_ASSERT(0, pData->pLastSection  == pSectionEntry[1]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section(pData) == pData->pFirstSection);

    state = cs64_ini_add_section(pData, (const CS64UTF8*)"s2", &pSectionEntry[2]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionEntry[2]->entryType, CS64_INI_ENTRY_SECTION, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pSectionEntry[2]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pSectionEntry[2]->pPrev == pSectionEntry[1]);
    UNIT_TEST_ASSERT(0, pSectionEntry[2]->pNext == cs64_ini_get_next_entry(pSectionEntry[2]));
    UNIT_TEST_ASSERT(0, pSectionEntry[2]->pPrev == cs64_ini_get_prev_entry(pSectionEntry[2]));

    UNIT_TEST_ASSERT(0, pData->pFirstSection == pSectionEntry[0]);
    UNIT_TEST_ASSERT(0, pData->pLastSection  == pSectionEntry[2]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section(pData) == pData->pFirstSection);

    state = cs64_ini_add_section(pData, (const CS64UTF8*)"s3", &pSectionEntry[3]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionEntry[3]->entryType, CS64_INI_ENTRY_SECTION, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->pPrev == pSectionEntry[2]);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->pNext == cs64_ini_get_next_entry(pSectionEntry[3]));
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->pPrev == cs64_ini_get_prev_entry(pSectionEntry[3]));

    UNIT_TEST_ASSERT(0, pData->pFirstSection == pSectionEntry[0]);
    UNIT_TEST_ASSERT(0, pData->pLastSection  == pSectionEntry[3]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section(pData) == pData->pFirstSection);

    // Section variables

    CS64INIEntry* pSectionVarEntry[] = {
        NULL,
        NULL, NULL, NULL,
        NULL, NULL, NULL, NULL};

    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section_value(pSectionEntry[0]) == NULL);

    state = cs64_ini_add_variable(pData, (const CS64UTF8*)"s1", (const CS64UTF8*)"key_0", (const CS64UTF8*)"Value", &pSectionVarEntry[0]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[0]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pSectionVarEntry[0]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[0]->pPrev == NULL);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[0]->pNext == cs64_ini_get_next_entry(pSectionVarEntry[0]));
    UNIT_TEST_ASSERT(0, pSectionVarEntry[0]->pPrev == cs64_ini_get_prev_entry(pSectionVarEntry[0]));
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section_value(pSectionEntry[1]) == pSectionVarEntry[0]);

    state = cs64_ini_add_variable(pData, (const CS64UTF8*)"s2", (const CS64UTF8*)"key_0", (const CS64UTF8*)"Value", &pSectionVarEntry[1]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[1]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pSectionVarEntry[1]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[1]->pPrev == NULL);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[1]->pNext == cs64_ini_get_next_entry(pSectionVarEntry[1]));
    UNIT_TEST_ASSERT(0, pSectionVarEntry[1]->pPrev == cs64_ini_get_prev_entry(pSectionVarEntry[1]));
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section_value(pSectionEntry[2]) == pSectionVarEntry[1]);

    state = cs64_ini_add_variable(pData, (const CS64UTF8*)"s2", (const CS64UTF8*)"key_1", (const CS64UTF8*)"Value", &pSectionVarEntry[2]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[2]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pSectionVarEntry[2]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[2]->pPrev == pSectionVarEntry[1]);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[2]->pNext == cs64_ini_get_next_entry(pSectionVarEntry[2]));
    UNIT_TEST_ASSERT(0, pSectionVarEntry[2]->pPrev == cs64_ini_get_prev_entry(pSectionVarEntry[2]));
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section_value(pSectionEntry[2]) == pSectionVarEntry[1]);

    state = cs64_ini_add_variable(pData, (const CS64UTF8*)"s2", (const CS64UTF8*)"key_2", (const CS64UTF8*)"Value", &pSectionVarEntry[3]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[3]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pSectionVarEntry[3]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[3]->pPrev == pSectionVarEntry[2]);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[3]->pNext == cs64_ini_get_next_entry(pSectionVarEntry[3]));
    UNIT_TEST_ASSERT(0, pSectionVarEntry[3]->pPrev == cs64_ini_get_prev_entry(pSectionVarEntry[3]));
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section_value(pSectionEntry[2]) == pSectionVarEntry[1]);

    state = cs64_ini_add_variable(pData, (const CS64UTF8*)"s3", (const CS64UTF8*)"key_0", (const CS64UTF8*)"Value", &pSectionVarEntry[4]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[4]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pSectionVarEntry[4]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[4]->pPrev == NULL);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[4]->pNext == cs64_ini_get_next_entry(pSectionVarEntry[4]));
    UNIT_TEST_ASSERT(0, pSectionVarEntry[4]->pPrev == cs64_ini_get_prev_entry(pSectionVarEntry[4]));
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section_value(pSectionEntry[3]) == pSectionVarEntry[4]);

    state = cs64_ini_add_variable(pData, (const CS64UTF8*)"s3", (const CS64UTF8*)"key_1", (const CS64UTF8*)"Value", &pSectionVarEntry[5]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[5]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pSectionVarEntry[5]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[5]->pPrev == pSectionVarEntry[4]);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[5]->pNext == cs64_ini_get_next_entry(pSectionVarEntry[5]));
    UNIT_TEST_ASSERT(0, pSectionVarEntry[5]->pPrev == cs64_ini_get_prev_entry(pSectionVarEntry[5]));
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section_value(pSectionEntry[3]) == pSectionVarEntry[4]);

    state = cs64_ini_add_variable(pData, (const CS64UTF8*)"s3", (const CS64UTF8*)"key_2", (const CS64UTF8*)"Value", &pSectionVarEntry[6]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[6]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pSectionVarEntry[6]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[6]->pPrev == pSectionVarEntry[5]);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[6]->pNext == cs64_ini_get_next_entry(pSectionVarEntry[6]));
    UNIT_TEST_ASSERT(0, pSectionVarEntry[6]->pPrev == cs64_ini_get_prev_entry(pSectionVarEntry[6]));
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section_value(pSectionEntry[3]) == pSectionVarEntry[4]);

    state = cs64_ini_add_variable(pData, (const CS64UTF8*)"s3", (const CS64UTF8*)"key_3", (const CS64UTF8*)"Value", &pSectionVarEntry[7]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[7]->entryType, CS64_INI_ENTRY_VALUE, "TOO short for dynamic RAM usage %d");
    UNIT_TEST_ASSERT(0, pSectionVarEntry[7]->pNext == NULL);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[7]->pPrev == pSectionVarEntry[6]);
    UNIT_TEST_ASSERT(0, pSectionVarEntry[7]->pNext == cs64_ini_get_next_entry(pSectionVarEntry[7]));
    UNIT_TEST_ASSERT(0, pSectionVarEntry[7]->pPrev == cs64_ini_get_prev_entry(pSectionVarEntry[7]));
    UNIT_TEST_ASSERT(0, cs64_ini_get_first_section_value(pSectionEntry[3]) == pSectionVarEntry[4]);

    // Relational deletion test!

    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 16);

     // Remove middle case section variable case.

    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pSectionVarEntry[6]) != NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pSectionVarEntry[6]) == pSectionEntry[3]);

    state = cs64_ini_del_entry(pData, pSectionVarEntry[6]);

    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[6]->entryType, CS64_INI_ENTRY_WAS_OCCUPIED, "This entry should have been deleted %d");
    UNIT_TEST_ASSERT(0, pSectionVarEntry[6]->type.value.pSection != NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pSectionVarEntry[6]) == NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, cs64_ini_get_entry_section_name(pSectionVarEntry[6]), cs64_ini_get_entry_name(pSectionVarEntry[6])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pSectionVarEntry[7]) != pSectionVarEntry[6]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pSectionVarEntry[5]) != pSectionVarEntry[6]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pSectionVarEntry[7]) == pSectionVarEntry[5]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_next_entry(pSectionVarEntry[5]) == pSectionVarEntry[7]);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->type.section.header.pFirstValue == pSectionVarEntry[4]);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->type.section.header.pLastValue  == pSectionVarEntry[7]);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->type.section.header.pFirstValue == cs64_ini_get_first_section_value(pSectionEntry[3]));
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == pEntry[3]);

    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 15);

    // Remove empty right section variable case.

    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pSectionVarEntry[7]) != NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pSectionVarEntry[7]) == pSectionEntry[3]);

    state = cs64_ini_del_entry(pData, pSectionVarEntry[7]);

    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[7]->entryType, CS64_INI_ENTRY_WAS_OCCUPIED, "This entry should have been deleted %d");
    UNIT_TEST_ASSERT(0, pSectionVarEntry[7]->type.value.pSection != NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pSectionVarEntry[7]) == NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, cs64_ini_get_entry_section_name(pSectionVarEntry[7]), cs64_ini_get_entry_name(pSectionVarEntry[7])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_next_entry(pSectionVarEntry[5]) != pSectionVarEntry[7]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_next_entry(pSectionVarEntry[5]) == NULL);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->type.section.header.pFirstValue == pSectionVarEntry[4]);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->type.section.header.pLastValue  == pSectionVarEntry[5]);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->type.section.header.pFirstValue == cs64_ini_get_first_section_value(pSectionEntry[3]));
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == pEntry[3]);

    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 14);

    // Remove empty left section variable case.

    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pSectionVarEntry[4]) != NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pSectionVarEntry[4]) == pSectionEntry[3]);

    state = cs64_ini_del_entry(pData, pSectionVarEntry[4]);

    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[4]->entryType, CS64_INI_ENTRY_WAS_OCCUPIED, "This entry should have been deleted %d");
    UNIT_TEST_ASSERT(0, pSectionVarEntry[4]->type.value.pSection != NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pSectionVarEntry[4]) == NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, cs64_ini_get_entry_section_name(pSectionVarEntry[4]), cs64_ini_get_entry_name(pSectionVarEntry[4])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_next_entry(pSectionVarEntry[5]) != pSectionVarEntry[4]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_next_entry(pSectionVarEntry[5]) == NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pSectionVarEntry[5]) != pSectionVarEntry[4]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pSectionVarEntry[5]) == NULL);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->type.section.header.pFirstValue == pSectionVarEntry[5]);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->type.section.header.pLastValue  == pSectionVarEntry[5]);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->type.section.header.pFirstValue == cs64_ini_get_first_section_value(pSectionEntry[3]));
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == pEntry[3]);

    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 13);

    // Remove no child section variable case.

    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pSectionVarEntry[5]) != NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pSectionVarEntry[5]) == pSectionEntry[3]);

    state = cs64_ini_del_entry(pData, pSectionVarEntry[5]);

    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[5]->entryType, CS64_INI_ENTRY_WAS_OCCUPIED, "This entry should have been deleted %d");
    UNIT_TEST_ASSERT(0, pSectionVarEntry[5]->type.value.pSection != NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_entry_section(pSectionVarEntry[5]) == NULL);
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, cs64_ini_get_entry_section_name(pSectionVarEntry[5]), cs64_ini_get_entry_name(pSectionVarEntry[5])) == NULL)
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->type.section.header.pFirstValue == NULL);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->type.section.header.pLastValue  == NULL);
    UNIT_TEST_ASSERT(0, pSectionEntry[3]->type.section.header.pFirstValue == cs64_ini_get_first_section_value(pSectionEntry[3]));
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == pEntry[3]);

    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 12);

    // Sections removal

    // Remove middle section case.
    state = cs64_ini_del_entry(pData, pSectionEntry[2]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, NULL, cs64_ini_get_entry_name(pSectionEntry[2])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, (const CS64UTF8*)"", cs64_ini_get_entry_name(pSectionEntry[2])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pSectionEntry[3]) != pSectionEntry[2]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pSectionEntry[1]) != pSectionEntry[2]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pSectionEntry[3]) == pSectionEntry[1]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_next_entry(pSectionEntry[1]) == pSectionEntry[3]);
    UNIT_TEST_ASSERT(0, pData->pFirstSection == cs64_ini_get_first_section(pData));
    UNIT_TEST_ASSERT(0, pData->pFirstSection == pSectionEntry[0]);
    UNIT_TEST_ASSERT(0, pData->pLastSection  == pSectionEntry[3]);

    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 8);
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[1]->entryType, CS64_INI_ENTRY_WAS_OCCUPIED, "This entry should have been removed with the section %d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[2]->entryType, CS64_INI_ENTRY_WAS_OCCUPIED, "This entry should have been removed with the section %d");
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[3]->entryType, CS64_INI_ENTRY_WAS_OCCUPIED, "This entry should have been removed with the section %d");

    // Remove empty section right.
    state = cs64_ini_del_entry(pData, pSectionEntry[3]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, NULL, cs64_ini_get_entry_name(pSectionEntry[3])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, (const CS64UTF8*)"", cs64_ini_get_entry_name(pSectionEntry[3])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pSectionEntry[1]) != pSectionEntry[3]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_next_entry(pSectionEntry[1]) == NULL);
    UNIT_TEST_ASSERT(0, pData->pFirstSection == cs64_ini_get_first_section(pData));
    UNIT_TEST_ASSERT(0, pData->pFirstSection == pSectionEntry[0]);
    UNIT_TEST_ASSERT(0, pData->pLastSection  != pSectionEntry[3]);
    UNIT_TEST_ASSERT(0, pData->pLastSection  == pSectionEntry[1]);

    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 7);

    // Remove empty section right.
    state = cs64_ini_del_entry(pData, pSectionEntry[0]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, NULL, cs64_ini_get_entry_name(pSectionEntry[0])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, (const CS64UTF8*)"", cs64_ini_get_entry_name(pSectionEntry[0])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pSectionEntry[1]) != pSectionEntry[0]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_next_entry(pSectionEntry[1]) == NULL);
    UNIT_TEST_ASSERT(0, pData->pFirstSection == cs64_ini_get_first_section(pData));
    UNIT_TEST_ASSERT(0, pData->pFirstSection != pSectionEntry[0]);
    UNIT_TEST_ASSERT(0, pData->pFirstSection == pSectionEntry[1]);
    UNIT_TEST_ASSERT(0, pData->pLastSection  == pSectionEntry[1]);

    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 6);

    // Remove empty section no child case.
    state = cs64_ini_del_entry(pData, pSectionEntry[1]);
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, NULL, cs64_ini_get_entry_name(pSectionEntry[1])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, (const CS64UTF8*)"", cs64_ini_get_entry_name(pSectionEntry[1])) == NULL)
    UNIT_TEST_ASSERT(0, pData->pFirstSection == cs64_ini_get_first_section(pData));
    UNIT_TEST_ASSERT(0, pData->pFirstSection != pSectionEntry[1]);
    UNIT_TEST_ASSERT(0, pData->pFirstSection == NULL);
    UNIT_TEST_ASSERT(0, pData->pFirstSection != pSectionEntry[1]);
    UNIT_TEST_ASSERT(0, pData->pLastSection  == NULL);

    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 4);
    UNIT_TEST_ASSERT_EQ(0, pSectionVarEntry[0]->entryType, CS64_INI_ENTRY_WAS_OCCUPIED, "This entry should have been removed with the section %d");

    // Globals removal

    state = cs64_ini_del_entry(pData, pEntry[2]); // Remove middle case.
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, NULL, cs64_ini_get_entry_name(pEntry[2])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, (const CS64UTF8*)"", cs64_ini_get_entry_name(pEntry[2])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pEntry[3]) != pEntry[2]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pEntry[1]) != pEntry[2]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pEntry[3]) == pEntry[1]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_next_entry(pEntry[1]) == pEntry[3]);
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == pEntry[3]);
    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 3);

    state = cs64_ini_del_entry(pData, pEntry[3]); // Remove empty right.
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, NULL, cs64_ini_get_entry_name(pEntry[3])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_variable(pData, (const CS64UTF8*)"", cs64_ini_get_entry_name(pEntry[3])) == NULL)
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pEntry[1]) != pEntry[3]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_next_entry(pEntry[1]) == NULL);
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == pEntry[0]);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  != pEntry[3]); // the third element is supposed to be removed!
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == pEntry[1]);
    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 2);

    state = cs64_ini_del_entry(pData, pEntry[0]); // Remove empty left.
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT(0, cs64_ini_get_prev_entry(pEntry[1]) != pEntry[0]);
    UNIT_TEST_ASSERT(0, cs64_ini_get_next_entry(pEntry[1]) == NULL);
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == pEntry[1]);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == pEntry[1]);
    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 1);

    state = cs64_ini_del_entry(pData, pEntry[1]); // Remove no child case.
    UNIT_TEST_ASSERT_EQ(0, state, CS64_INI_ENTRY_SUCCESS, "%d");
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == cs64_ini_get_first_global_value(pData));
    UNIT_TEST_ASSERT(0, pData->globals.pFirstValue == NULL);
    UNIT_TEST_ASSERT(0, pData->globals.pLastValue  == NULL);
    UNIT_TEST_ASSERT(0, pData->hashTable.currentEntryAmount == 0);

    cs64_ini_data_free(pData);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_display_entry(const CS64INIEntry *const pEntry) {
    if(pEntry == NULL) {
        printf("ENTRY NULL!\n");
        return;
    }

    switch(pEntry->entryType) {
        case CS64_INI_ENTRY_EMPTY:
            printf("ENTRY Always Empty!\n");
            return;
        case CS64_INI_ENTRY_WAS_OCCUPIED:
            printf("ENTRY Was Occupied!\n");
            return;
        case CS64_INI_ENTRY_SECTION:
        case CS64_INI_ENTRY_DYNAMIC_SECTION:
            printf("ENTRY IS A SECTION!\n");
            printf("\tfirst    value = %p\n",  pEntry->type.section.header.pFirstValue);
            printf("\tlast     value = %p\n",  pEntry->type.section.header.pLastValue);
            printf("\tname byte size = %zd\n", pEntry->type.section.nameByteSize);

            if(pEntry->entryType == CS64_INI_ENTRY_DYNAMIC_SECTION) {
                printf("\tname = %s\n", pEntry->type.section.name.pDynamic);
            }
            else {
                printf("\tname = %s\n", pEntry->type.section.name.fixed);
            }

            break;
        case CS64_INI_ENTRY_VALUE:
        case CS64_INI_ENTRY_DYNAMIC_VALUE:
            printf("ENTRY IS A VALUE!\n");
            printf("\tparent  section = %p\n",  pEntry->type.value.pSection);
            printf("\tname  byte size = %zd\n", pEntry->type.value.nameByteSize);
            printf("\tvalue byte size = %zd\n", pEntry->type.value.valueByteSize);
            printf("\tname  = %s\n", cs64_ini_get_entry_name(pEntry));
            printf("\tvalue = %s\n", cs64_ini_get_entry_value(pEntry));
            break;
        default:
            printf("ENTRY is corrupted with a value %d!\n", pEntry->entryType);
            return;
    }
}

void cs64_ini_display_data(const CS64INIData *const pData) {
    if(pData == NULL) {
        printf("INI DATA is NULL!\n");
        return;
    }
    printf("INI DATA DUMP!\n");

    printf("\tentries                   = %p\n",  pData->hashTable.pEntries);
    printf("\tcurrent entry amount      = %zd\n", pData->hashTable.currentEntryAmount);
    printf("\tentry capacity            = %zd\n", pData->hashTable.entryCapacity);
    printf("\tentry capacity up limit   = %zd\n", pData->hashTable.entryCapacityUpLimit);
    printf("\tentry capacity down limit = %zd\n", pData->hashTable.entryCapacityDownLimit);

    CS64Size l = 0;
    while(l < pData->hashTable.entryCapacity) {
        cs64_ini_display_entry( &pData->hashTable.pEntries[l] );
        l++;
    }

    printf("\tlast comment size    = %zd\n", pData->lastCommentSize);
    printf("\tlast comment pointer = %p\n",  pData->pLastComment);
    if(pData->pLastComment != NULL) {
        printf("\tlast comment = %s\n",  pData->pLastComment);
    }

    printf("\tglobals first value = %p\n", pData->globals.pFirstValue);
    printf("\tglobals last  value = %p\n", pData->globals.pLastValue);
    printf("\tfirst section = %p\n", pData->pFirstSection);
    printf("\tlast  section = %p\n", pData->pLastSection);
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
