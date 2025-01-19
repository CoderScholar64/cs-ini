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

void display_parser_result(CS64INIParserResult *pParserResult);

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
    UNIT_TEST_DETAIL_ASSERT(0, tokenResult.sectionBeginCount == tokenResult.sectionEndCount, printf("These sections should be equal %zu != %zu\n", tokenResult.sectionBeginCount, tokenResult.sectionEndCount);)\
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
    PARSE_LINE_SETUP(1024, "[SectionWithTOOmuchData]\n; DNE\n[SectA]\n; comment\n[SectB]", 5)

    CS64INIParserResult result;

    /* Failure test. */
    result = cs64_ini_parse_line(&parserContext);
    UNIT_TEST_ASSERT_EQ(0, result.state, CS64_INI_PARSER_INI_DATA_ERROR, "%d");
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)result.status.data_error.pFunctionName, "cs64_ini_add_section") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
    UNIT_TEST_ASSERT_EQ(0, result.status.data_error.functionStatus, CS64_INI_ENTRY_NO_MEMORY_ERROR, "%d");

    CS64INIEntry *pEntry;

    /* SectionWithTOOmuchData Success test. */
    SET_AVAILABLE_MEM_PAGES(1)
    parserContext.tokenOffset = 0;
    result = cs64_ini_parse_line(&parserContext);
    UNIT_TEST_DETAIL_ASSERT(0, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result););
    UNIT_TEST_ASSERT_EQ(0, parserContext.tokenOffset, 3, "%zd");
    pEntry = cs64_ini_get_section(parserContext.pData, "SectionWithTOOmuchData");
    UNIT_TEST_ASSERT_NEQ(0, pEntry, NULL, "%p");
    UNIT_TEST_ASSERT_EQ(0, parserContext.pSection, pEntry, "%p");

    SET_AVAILABLE_MEM_PAGES(0);
    parserContext.tokenOffset = 4;
    result = cs64_ini_parse_line(&parserContext);
    UNIT_TEST_DETAIL_ASSERT(0, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result););
    UNIT_TEST_ASSERT_EQ(0, parserContext.tokenOffset, 9, "%zd");
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
    UNIT_TEST_ASSERT_EQ(0, result.status.data_error.functionStatus, CS64_INI_ENTRY_NO_MEMORY_ERROR, "%d");
    pEntry = cs64_ini_get_section(parserContext.pData, "SectA");
    UNIT_TEST_ASSERT_EQ(0, cs64_ini_get_entry_comment(pEntry), NULL, "%p");
    UNIT_TEST_ASSERT_EQ(0, parserContext.pSection, pEntry, "%p");

    SET_AVAILABLE_MEM_PAGES(1)
    parserContext.tokenOffset = 10;
    result = cs64_ini_parse_line(&parserContext);
    UNIT_TEST_DETAIL_ASSERT(0, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result););
    UNIT_TEST_ASSERT_EQ(0, parserContext.tokenOffset, 15, "%zd");
    pEntry = cs64_ini_get_section(parserContext.pData, "SectB");
    UNIT_TEST_ASSERT_NEQ(0, cs64_ini_get_entry_comment(pEntry), NULL, "%p");
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((char*)cs64_ini_get_entry_comment(pEntry), " comment") == 0, printf("Actually (%s) \n", cs64_ini_get_entry_comment(pEntry)););
    UNIT_TEST_ASSERT_EQ(0, parserContext.pSection, pEntry, "%p");

    cs64_ini_data_free(parserContext.pData);
    cs64_ini_lexer_free(parserContext.pTokenResult);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_last_comment_test() {
    PARSE_LINE_SETUP(1024, "; First Test", 2)

    CS64INIParserResult result;

    result = cs64_ini_parse_line(&parserContext);
    UNIT_TEST_DETAIL_ASSERT(0, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result););
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)result.status.data_error.pFunctionName, "cs64_ini_set_last_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
    UNIT_TEST_ASSERT_EQ(0, result.status.data_error.functionStatus, CS64_INI_ENTRY_NO_MEMORY_ERROR, "%d");

    SET_AVAILABLE_MEM_PAGES(1)
    parserContext.tokenOffset = 0;
    result = cs64_ini_parse_line(&parserContext);
    UNIT_TEST_DETAIL_ASSERT(0, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result););
    UNIT_TEST_ASSERT_EQ(0, parserContext.tokenOffset, 2, "%zd");
    UNIT_TEST_ASSERT_NEQ(0, cs64_ini_get_last_comment(parserContext.pData), NULL, "%p");
    UNIT_TEST_DETAIL_ASSERT(0, strcmp((const char*)cs64_ini_get_last_comment(parserContext.pData), " First Test") == 0, printf("Actually (%s) \n", cs64_ini_get_last_comment(parserContext.pData)););

    cs64_ini_data_free(parserContext.pData);
    cs64_ini_lexer_free(parserContext.pTokenResult);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void display_token_type(CS64INITokenType type) {
    switch(type) {
        case CS64_INI_TOKEN_DELEMETER:
            printf("    CS64_INI_TOKEN_DELEMETER\n");
            return;
        case CS64_INI_TOKEN_VALUE:
            printf("    CS64_INI_TOKEN_VALUE\n");
            return;
        case CS64_INI_TOKEN_COMMENT:
            printf("    CS64_INI_TOKEN_COMMENT\n");
            return;
        case CS64_INI_TOKEN_END:
            printf("    CS64_INI_TOKEN_END\n");
            return;
        case CS64_INI_TOKEN_SECTION_START:
            printf("    CS64_INI_TOKEN_SECTION_START\n");
            return;
        case CS64_INI_TOKEN_SECTION_END:
            printf("    CS64_INI_TOKEN_SECTION_END\n");
            return;
    }
}

void display_entry_state(CS64INIEntryState type) {
    switch(type) {
        case CS64_INI_ENTRY_SUCCESS:
            printf("    CS64_INI_ENTRY_SUCCESS\n");
            return;
        case CS64_INI_ENTRY_DATA_NULL_ERROR:
            printf("    CS64_INI_ENTRY_DATA_NULL_ERROR\n");
            return;
        case CS64_INI_ENTRY_SECTION_EMPTY_ERROR:
            printf("    CS64_INI_ENTRY_SECTION_EMPTY_ERROR\n");
            return;
        case CS64_INI_ENTRY_ENTRY_EMPTY_ERROR:
            printf("    CS64_INI_ENTRY_ENTRY_EMPTY_ERROR\n");
            return;
        case CS64_INI_ENTRY_ENTRY_EXISTS_ERROR:
            printf("    CS64_INI_ENTRY_ENTRY_EXISTS_ERROR\n");
            return;
        case CS64_INI_ENTRY_ENTRY_DNE_ERROR:
            printf("    CS64_INI_ENTRY_ENTRY_DNE_ERROR\n");
            return;
        case CS64_INI_ENTRY_NO_MEMORY_ERROR:
            printf("    CS64_INI_ENTRY_NO_MEMORY_ERROR\n");
            return;
        case CS64_INI_ENTRY_ILLEGAL_STRING_ERROR:
            printf("    CS64_INI_ENTRY_ILLEGAL_STRING_ERROR\n");
            return;
        case CS64_INI_ENTRY_INVALID_ENCODE_ERROR:
            printf("    CS64_INI_ENTRY_INVALID_ENCODE_ERROR\n");
            return;
    }
}

void display_parser_result(CS64INIParserResult *pParserResult) {
    if(pParserResult == NULL) {
        printf("CS64INIParserContext is NULL\n");
        return;
    }
    printf("CS64INIParserContext is %p\n", pParserResult);

    switch(pParserResult->state) {
        case CS64_INI_PARSER_SUCCESS:
            printf("  CS64_INI_PARSER_SUCCESS\n");
            return;
        case CS64_INI_PARSER_UNEXPECTED_ERROR:
            printf("  CS64_INI_PARSER_UNEXPECTED_ERROR\n");
            {
                printf("  Received Token\n");
                display_token_type(pParserResult->status.unexpected_token.receivedToken.type);

                CS64Size p = 0;

                while(p < pParserResult->status.unexpected_token.expectedTokenAmount) {
                    printf("  Other %zu\n", p);
                    display_token_type(pParserResult->status.unexpected_token.pExpectedTokens[p]);
                    p++;
                }
            }
            return;
        case CS64_INI_PARSER_REDECLARATION_ERROR:
            printf("  CS64_INI_PARSER_REDECLARATION_ERROR\n");
            return;
        case CS64_INI_PARSER_INI_DATA_ERROR:
            printf("  CS64_INI_PARSER_INI_DATA_ERROR\n");
            {
                printf("  Function Name\n    %s\n", pParserResult->status.data_error.pFunctionName);
                printf("  Function Status\n");
                display_entry_state(pParserResult->status.data_error.functionStatus);
            }
            return;
        case CS64_INI_PARSER_LEXER_MEM_ERROR:
            printf("  CS64_INI_PARSER_LEXER_MEM_ERROR\n");
            return;
    }
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
