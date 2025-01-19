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
void cs64_ini_section_test();
void cs64_ini_last_comment_test();

int main() {
    cs64_ini_section_test();
    cs64_ini_last_comment_test();

    return 0;
}

#define PARSE_LINE_SETUP(BUFFER_SIZE, FILE_DATA, TOKEN_DATA_PAGES) \
    CS64UTF8 buffer[BUFFER_SIZE];\
\
    CS64INIParserContext parserContext;\
\
    parserContext.stringBufferLimit = sizeof(buffer) / 2;\
    parserContext.pStringBuffer = buffer;\
    parserContext.pValueBuffer  = buffer + parserContext.stringBufferLimit;\
    parserContext.tokenOffset   = 0;\
    parserContext.pSection      = NULL;\
\
    CS64UTF8 fileData[] = FILE_DATA;\
\
    SET_AVAILABLE_MEM_PAGES(TOKEN_DATA_PAGES)\
    CS64INITokenResult tokenResult = cs64_ini_lexer(fileData, sizeof(fileData) / sizeof(fileData[0]) - 1);\
    UNIT_TEST_ASSERT_EQ(0, tokenResult.state, CS64_INI_LEXER_SUCCESS, "%d");\
    UNIT_TEST_DETAIL_ASSERT(0, tokenResult.sectionBeginCount == tokenResult.sectionEndCount, printf("These sections should be equal %zd != %zd\n", tokenResult.sectionBeginCount == tokenResult.sectionEndCount);)\
\
    SET_AVAILABLE_MEM_PAGES(2)\
    parserContext.pData = cs64_ini_data_alloc();\
    UNIT_TEST_ASSERT_NEQ(0, parserContext.pData, NULL, "%p");\
\
    if(parserContext.pData->hashTable.entryCapacityUpLimit >= tokenResult.delimeterCount + tokenResult.sectionBeginCount) {\
        SET_AVAILABLE_MEM_PAGES(1)\
\
        int reserveResult = cs64_ini_data_reserve(parserContext.pData, tokenResult.delimeterCount + tokenResult.sectionBeginCount);\
\
        UNIT_TEST_ASSERT_EQ(0, reserveResult, 0, "%d");\
    }\
\
    parserContext.pSource = fileData;\
    parserContext.pTokenResult = &tokenResult;

void cs64_ini_section_test() {
    PARSE_LINE_SETUP(1024, "[SectionWithTOOmuchData]", 2)

    CS64INIParserResult result;

    result = cs64_ini_parse_line(&parserContext);
    UNIT_TEST_ASSERT_EQ(0, result.state, CS64_INI_PARSER_INI_DATA_ERROR, "%d");
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)result.status.data_error.pFunctionName, "cs64_ini_add_section") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
    UNIT_TEST_ASSERT_EQ(0, result.status.data_error.functionStatus, CS64_INI_ENTRY_NO_MEMORY_ERROR, "%d");

    SET_AVAILABLE_MEM_PAGES(1)
    parserContext.tokenOffset = 0;
    result = cs64_ini_parse_line(&parserContext);
    UNIT_TEST_ASSERT_EQ(0, result.state, CS64_INI_PARSER_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, parserContext.tokenOffset, 3, "%zd");
    CS64INIEntry *pEntry = cs64_ini_get_section(parserContext.pData, "SectionWithTOOmuchData");
    UNIT_TEST_ASSERT_NEQ(0, pEntry, NULL, "%p");
    UNIT_TEST_ASSERT_EQ(0, parserContext.pSection, pEntry, "%p");

    cs64_ini_data_free(parserContext.pData);
    cs64_ini_lexer_free(parserContext.pTokenResult);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_last_comment_test() {
    PARSE_LINE_SETUP(1024, "; First Test", 2)

    CS64INIParserResult result;

    result = cs64_ini_parse_line(&parserContext);
    UNIT_TEST_ASSERT_EQ(0, result.state, CS64_INI_PARSER_INI_DATA_ERROR, "%d");
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)result.status.data_error.pFunctionName, "cs64_ini_set_last_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
    UNIT_TEST_ASSERT_EQ(0, result.status.data_error.functionStatus, CS64_INI_ENTRY_NO_MEMORY_ERROR, "%d");

    SET_AVAILABLE_MEM_PAGES(1)
    parserContext.tokenOffset = 0;
    result = cs64_ini_parse_line(&parserContext);
    UNIT_TEST_ASSERT_EQ(0, result.state, CS64_INI_PARSER_SUCCESS, "%d");
    UNIT_TEST_ASSERT_EQ(0, parserContext.tokenOffset, 1, "%zd");
    UNIT_TEST_ASSERT_NEQ(0, cs64_ini_get_last_comment(parserContext.pData), NULL, "%p");
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)cs64_ini_get_last_comment(parserContext.pData), " First Test") == 0, printf("Actually (%s) \n", cs64_ini_get_last_comment(parserContext.pData)););

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
