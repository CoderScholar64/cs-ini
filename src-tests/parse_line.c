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
void cs64_ini_variable_test();
void cs64_ini_last_comment_test();

void display_parser_context(CS64INIParserContext *pParserContext);
void display_parser_result(CS64INIParserResult *pParserResult);

int main() {
    cs64_ini_section_test();
    cs64_ini_variable_test();
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
    /* Configure section names here */
    #define SECTION_0 "sect0"
    #define SECTION_1 "sect1"
    #define SECTION_2 "sect2"
    #define SECTION_3 "sect3"
    #define SECTION_4 "section_long_name_case__zero"
    #define SECTION_5 "section_long_name_case___one"
    #define SECTION_6 "section_long_name_case___two"
    #define SECTION_7 "section_long_name_case_three"

    /* Configure inline comments here */
    #define INLINE_COMMENT " inline"
    #define COMMENT " comment"

    static const CS64UTF8 section[8][32] = {
        SECTION_0,
        SECTION_1,
        SECTION_2,
        SECTION_3,
        SECTION_4,
        SECTION_5,
        SECTION_6,
        SECTION_7};

    static const int SECTION_MEM_REQUIRED[8] = {
        0,
        0,
        0,
        0,
        1,
        1,
        1,
        1
    };

    static const int INLINE_MEM_REQUIRED[8] = {
        0,
        1,
        0,
        1,
        0,
        1,
        0,
        1
    };

    static const int COMMENT_MEM_REQUIRED[8] = {
        0,
        0,
        1,
        1,
        0,
        0,
        1,
        1
    };

    PARSE_LINE_SETUP(1024,
                      "[" SECTION_0 "]\n"                     /* 0 malloc */
                      "[" SECTION_1 "];" INLINE_COMMENT "\n"  /* 1 malloc */
        ";" COMMENT "\n[" SECTION_2 "]\n"                     /* 1 malloc */
        ";" COMMENT "\n[" SECTION_3 "];" INLINE_COMMENT "\n"  /* 2 malloc */
                      "[" SECTION_4 "]\n"                     /* 1 malloc */
                      "[" SECTION_5 "];" INLINE_COMMENT "\n"  /* 2 malloc */
        ";" COMMENT "\n[" SECTION_6 "]\n"                     /* 2 malloc */
        ";" COMMENT "\n[" SECTION_7 "];" INLINE_COMMENT "\n", /* 3 malloc */
        12)

    static const int START_OFFSETS[8] = {
         0,
         4,
         9,
        15,
        22,
        26,
        31,
        37
    };

    static const int END_OFFSETS[8] = {
        3,
        8,
        14,
        21,
        25,
        30,
        36,
        43
    };

    CS64INIParserResult result;
    CS64INIEntry *pEntry;

    int testIndex;

    /* Successful cases */
    testIndex = 0;
    while(testIndex < 8) {
        SET_AVAILABLE_MEM_PAGES(SECTION_MEM_REQUIRED[testIndex] + INLINE_MEM_REQUIRED[testIndex] + COMMENT_MEM_REQUIRED[testIndex])
        parserContext.tokenOffset = START_OFFSETS[testIndex];
        result = cs64_ini_parse_line(&parserContext);
        UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result); display_parser_context(&parserContext););
        UNIT_TEST_ASSERT_EQ(testIndex, parserContext.tokenOffset, END_OFFSETS[testIndex], "%zd");
        pEntry = cs64_ini_get_section(parserContext.pData, section[testIndex]);
        UNIT_TEST_ASSERT_EQ(testIndex, parserContext.pSection, pEntry, "%p");

        if(INLINE_MEM_REQUIRED[testIndex]) {
            UNIT_TEST_ASSERT_NEQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)cs64_ini_get_entry_inline_comment(pEntry), INLINE_COMMENT) == 0, printf("Actually (%s) \n", cs64_ini_get_entry_inline_comment(pEntry)););
        }
        else {
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
        }

        if(COMMENT_MEM_REQUIRED[testIndex]) {
            UNIT_TEST_ASSERT_NEQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)cs64_ini_get_entry_comment(pEntry), COMMENT) == 0, printf("Actually (%s) \n", cs64_ini_get_entry_comment(pEntry)););
        }
        else {
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");
        }

        cs64_ini_del_entry(parserContext.pData, pEntry);

        /* End of Test */
        testIndex++;
    }

    /* No mem cases */
    testIndex = 0;
    while(testIndex < 8) {
        SET_AVAILABLE_MEM_PAGES(0)
        parserContext.tokenOffset = START_OFFSETS[testIndex];

        if(SECTION_MEM_REQUIRED[testIndex] + INLINE_MEM_REQUIRED[testIndex] + COMMENT_MEM_REQUIRED[testIndex] == 0) {
            /* This function should succeed since there is no memory required. */
            result = cs64_ini_parse_line(&parserContext);

            UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result); display_parser_context(&parserContext););
            UNIT_TEST_ASSERT_EQ(testIndex, parserContext.tokenOffset, END_OFFSETS[testIndex], "%zd");
            pEntry = cs64_ini_get_section(parserContext.pData, section[testIndex]);
            UNIT_TEST_ASSERT_EQ(testIndex, parserContext.pSection, pEntry, "%p");

            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }
        else if(SECTION_MEM_REQUIRED[testIndex] == 1) {
            result = cs64_ini_parse_line(&parserContext);

            UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););

            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_add_section") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
            UNIT_TEST_ASSERT_EQ(testIndex, result.status.data_error.functionStatus, CS64_INI_ENTRY_NO_MEMORY_ERROR, "%d");
        }
        else {
            /* This function should succeed in creating an entry with a short name. The short name sections do not need malloc. */
            result = cs64_ini_parse_line(&parserContext);

            /* It should still error out though. */
            UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););

            /* Inline comments are called before the normal comments. Just the way the parser works. */
            if(INLINE_MEM_REQUIRED[testIndex] == 1) {
                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););
                UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_inline_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
            }
            else {
                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););
                UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
            }

            /* The entry and the section should still be set. */
            pEntry = cs64_ini_get_section(parserContext.pData, section[testIndex]);
            UNIT_TEST_ASSERT_EQ(testIndex, parserContext.pSection, pEntry, "%p");

            /* The comments in this case should fail. */
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry),        NULL, "%p");

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }

        /* End of Test */
        testIndex++;
    }

    /* One mem cases */
    testIndex = 0;
    while(testIndex < 8) {
        SET_AVAILABLE_MEM_PAGES(1)
        parserContext.tokenOffset = START_OFFSETS[testIndex];

        if(SECTION_MEM_REQUIRED[testIndex] == 1) {
            /* SECTION = 1 INLINE = X COMMENT = Y */

            result = cs64_ini_parse_line(&parserContext);

            if(INLINE_MEM_REQUIRED[testIndex] == 1) {
                /* SECTION = 1 INLINE = 1 COMMENT = Y */

                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););
                UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_inline_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
            }
            else if(COMMENT_MEM_REQUIRED[testIndex] == 1) {
                /* SECTION = 1 INLINE = 0 COMMENT = 1 */

                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););
                UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
            }
            else {
                /* SECTION = 1 INLINE = 0 COMMENT = 0 */
                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result); display_parser_context(&parserContext););
            }

            /* The entry and the section should still be set. */
            pEntry = cs64_ini_get_section(parserContext.pData, section[testIndex]);
            UNIT_TEST_ASSERT_EQ(testIndex, parserContext.pSection, pEntry, "%p");

            /* The comments in this case should fail. */
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry),        NULL, "%p");

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }
        else if(INLINE_MEM_REQUIRED[testIndex] == 1) {
            /* SECTION = 0 INLINE = 1 COMMENT = Y */

            result = cs64_ini_parse_line(&parserContext);

            if(COMMENT_MEM_REQUIRED[testIndex] == 1) {
                /* SECTION = 0 INLINE = 1 COMMENT = 1 */
                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););
                UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
            }
            else {
                /* SECTION = 0 INLINE = 1 COMMENT = 0 */
                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result); display_parser_context(&parserContext););
            }

            /* The entry and the section should still be set. */
            pEntry = cs64_ini_get_section(parserContext.pData, section[testIndex]);
            UNIT_TEST_ASSERT_EQ(testIndex, parserContext.pSection, pEntry, "%p");

            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");

            UNIT_TEST_ASSERT_NEQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)cs64_ini_get_entry_inline_comment(pEntry), INLINE_COMMENT) == 0, printf("Actually (%s) \n", cs64_ini_get_entry_inline_comment(pEntry)););

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }
        else if(COMMENT_MEM_REQUIRED[testIndex] == 1) {
            /* SECTION = 0 INLINE = 0 COMMENT = 1 */

            result = cs64_ini_parse_line(&parserContext);

            UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result); display_parser_context(&parserContext););

            /* The entry and the section should still be set. */
            pEntry = cs64_ini_get_section(parserContext.pData, section[testIndex]);
            UNIT_TEST_ASSERT_EQ(testIndex, parserContext.pSection, pEntry, "%p");

            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");

            UNIT_TEST_ASSERT_NEQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)cs64_ini_get_entry_comment(pEntry), COMMENT) == 0, printf("Actually (%s) \n", cs64_ini_get_entry_comment(pEntry)););

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }
        else {
            /* I am sure that the case would always succeed. */
        }

        /* End of Test */
        testIndex++;
    }

    /* Two mem case */
    testIndex = 0;
    while(testIndex < 8) {
        SET_AVAILABLE_MEM_PAGES(2)
        parserContext.tokenOffset = START_OFFSETS[testIndex];

        if(SECTION_MEM_REQUIRED[testIndex] == 1 && INLINE_MEM_REQUIRED[testIndex] == 1 && COMMENT_MEM_REQUIRED[testIndex] == 1) {
            result = cs64_ini_parse_line(&parserContext);

            UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););

            /* The entry and the section should still be set. */
            pEntry = cs64_ini_get_section(parserContext.pData, section[testIndex]);
            UNIT_TEST_ASSERT_EQ(testIndex, parserContext.pSection, pEntry, "%p");

            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");

            UNIT_TEST_ASSERT_NEQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)cs64_ini_get_entry_inline_comment(pEntry), INLINE_COMMENT) == 0, printf("Actually (%s) \n", cs64_ini_get_entry_inline_comment(pEntry)););

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }

        /* End of Test */
        testIndex++;
    }

    cs64_ini_data_free(parserContext.pData);
    cs64_ini_lexer_free(parserContext.pTokenResult);

    UNIT_TEST_MEM_CHECK_ASSERT
}

void cs64_ini_variable_test() {
    /* Configure section names here */
    #define KEY(N) "key_" #N
    #define SHORT_VALUE "val"
    #define  LONG_VALUE "This is an intentially long value"

    static const int VALUE_MEM_REQUIRED[8] = {
        0,
        0,
        0,
        0,
        1,
        1,
        1,
        1
    };

    static const int INLINE_MEM_REQUIRED[8] = {
        0,
        1,
        0,
        1,
        0,
        1,
        0,
        1
    };

    static const int COMMENT_MEM_REQUIRED[8] = {
        0,
        0,
        1,
        1,
        0,
        0,
        1,
        1
    };

    PARSE_LINE_SETUP(1024,
                         KEY(0) " = " SHORT_VALUE "\n"                     /* 0 malloc */
                         KEY(1) " = " SHORT_VALUE ";" INLINE_COMMENT "\n"  /* 1 malloc */
        ";" COMMENT "\n" KEY(2) " = " SHORT_VALUE "\n"                     /* 1 malloc */
        ";" COMMENT "\n" KEY(3) " = " SHORT_VALUE ";" INLINE_COMMENT "\n"  /* 2 malloc */
                         KEY(4) " = "  LONG_VALUE "\n"                     /* 1 malloc */
                         KEY(5) " = "  LONG_VALUE ";" INLINE_COMMENT "\n"  /* 2 malloc */
        ";" COMMENT "\n" KEY(6) " = "  LONG_VALUE "\n"                     /* 2 malloc */
        ";" COMMENT "\n" KEY(7) " = "  LONG_VALUE ";" INLINE_COMMENT "\n", /* 3 malloc */
        12)

    static const int START_OFFSETS[8] = {
         0,
         4,
         9,
        15,
        22,
        26,
        31,
        37
    };

    static const int END_OFFSETS[8] = {
        3,
        8,
        14,
        21,
        25,
        30,
        36,
        43
    };

    CS64INIParserResult result;
    CS64INIEntry *pEntry;

    int testIndex;

    /* Successful cases */
    testIndex = 0;
    while(testIndex < 8) {
        SET_AVAILABLE_MEM_PAGES(VALUE_MEM_REQUIRED[testIndex] + INLINE_MEM_REQUIRED[testIndex] + COMMENT_MEM_REQUIRED[testIndex])
        parserContext.tokenOffset = START_OFFSETS[testIndex];
        result = cs64_ini_parse_line(&parserContext);
        UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result); display_parser_context(&parserContext););
        UNIT_TEST_ASSERT_EQ(testIndex, parserContext.tokenOffset, END_OFFSETS[testIndex], "%zd");

        /* The entry should still be there. */
        pEntry = cs64_ini_get_variable(parserContext.pData, NULL, (CS64UTF8*)KEY(testIndex));

        if(INLINE_MEM_REQUIRED[testIndex]) {
            UNIT_TEST_ASSERT_NEQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)cs64_ini_get_entry_inline_comment(pEntry), INLINE_COMMENT) == 0, printf("Actually (%s) \n", cs64_ini_get_entry_inline_comment(pEntry)););
        }
        else {
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
        }

        if(COMMENT_MEM_REQUIRED[testIndex]) {
            UNIT_TEST_ASSERT_NEQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)cs64_ini_get_entry_comment(pEntry), COMMENT) == 0, printf("Actually (%s) \n", cs64_ini_get_entry_comment(pEntry)););
        }
        else {
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");
        }

        cs64_ini_del_entry(parserContext.pData, pEntry);

        /* End of Test */
        testIndex++;
    }

    /* No mem cases */
    testIndex = 0;
    while(testIndex < 8) {
        SET_AVAILABLE_MEM_PAGES(0)
        parserContext.tokenOffset = START_OFFSETS[testIndex];

        if(VALUE_MEM_REQUIRED[testIndex] + INLINE_MEM_REQUIRED[testIndex] + COMMENT_MEM_REQUIRED[testIndex] == 0) {
            /* This function should succeed since there is no memory required. */
            result = cs64_ini_parse_line(&parserContext);

            UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result); display_parser_context(&parserContext););
            UNIT_TEST_ASSERT_EQ(testIndex, parserContext.tokenOffset, END_OFFSETS[testIndex], "%zd");

            /* The entry should still be there. */
            pEntry = cs64_ini_get_variable(parserContext.pData, NULL, (CS64UTF8*)KEY(testIndex));

            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }
        else if(VALUE_MEM_REQUIRED[testIndex] == 1) {
            result = cs64_ini_parse_line(&parserContext);

            UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););

            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_add_section") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
            UNIT_TEST_ASSERT_EQ(testIndex, result.status.data_error.functionStatus, CS64_INI_ENTRY_NO_MEMORY_ERROR, "%d");
        }
        else {
            /* This function should succeed in creating an entry with a short name. The short name sections do not need malloc. */
            result = cs64_ini_parse_line(&parserContext);

            /* It should still error out though. */
            UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););

            /* Inline comments are called before the normal comments. Just the way the parser works. */
            if(INLINE_MEM_REQUIRED[testIndex] == 1) {
                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););
                UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_inline_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
            }
            else {
                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););
                UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
            }

            /* The entry should still be there. */
            pEntry = cs64_ini_get_variable(parserContext.pData, NULL, (CS64UTF8*)KEY(testIndex));

            /* The comments in this case should fail. */
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry),        NULL, "%p");

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }

        /* End of Test */
        testIndex++;
    }

    /* One mem cases */
    testIndex = 0;
    while(testIndex < 8) {
        SET_AVAILABLE_MEM_PAGES(1)
        parserContext.tokenOffset = START_OFFSETS[testIndex];

        if(VALUE_MEM_REQUIRED[testIndex] == 1) {
            /* SECTION = 1 INLINE = X COMMENT = Y */

            result = cs64_ini_parse_line(&parserContext);

            if(INLINE_MEM_REQUIRED[testIndex] == 1) {
                /* SECTION = 1 INLINE = 1 COMMENT = Y */

                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););
                UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_inline_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
            }
            else if(COMMENT_MEM_REQUIRED[testIndex] == 1) {
                /* SECTION = 1 INLINE = 0 COMMENT = 1 */

                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););
                UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
            }
            else {
                /* SECTION = 1 INLINE = 0 COMMENT = 0 */
                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result); display_parser_context(&parserContext););
            }

            /* The entry should still be there. */
            pEntry = cs64_ini_get_variable(parserContext.pData, NULL, (CS64UTF8*)KEY(testIndex));

            /* The comments in this case should fail. */
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry),        NULL, "%p");

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }
        else if(INLINE_MEM_REQUIRED[testIndex] == 1) {
            /* SECTION = 0 INLINE = 1 COMMENT = Y */

            result = cs64_ini_parse_line(&parserContext);

            if(COMMENT_MEM_REQUIRED[testIndex] == 1) {
                /* SECTION = 0 INLINE = 1 COMMENT = 1 */
                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););
                UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
            }
            else {
                /* SECTION = 0 INLINE = 1 COMMENT = 0 */
                UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result); display_parser_context(&parserContext););
            }

            /* The entry should still be there. */
            pEntry = cs64_ini_get_variable(parserContext.pData, NULL, (CS64UTF8*)KEY(testIndex));

            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");

            UNIT_TEST_ASSERT_NEQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)cs64_ini_get_entry_inline_comment(pEntry), INLINE_COMMENT) == 0, printf("Actually (%s) \n", cs64_ini_get_entry_inline_comment(pEntry)););

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }
        else if(COMMENT_MEM_REQUIRED[testIndex] == 1) {
            /* SECTION = 0 INLINE = 0 COMMENT = 1 */

            result = cs64_ini_parse_line(&parserContext);

            UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_SUCCESS, display_parser_result(&result); display_parser_context(&parserContext););

            /* The entry should still be there. */
            pEntry = cs64_ini_get_variable(parserContext.pData, NULL, (CS64UTF8*)KEY(testIndex));

            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");

            UNIT_TEST_ASSERT_NEQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)cs64_ini_get_entry_comment(pEntry), COMMENT) == 0, printf("Actually (%s) \n", cs64_ini_get_entry_comment(pEntry)););

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }
        else {
            /* I am sure that the case would always succeed. */
        }

        /* End of Test */
        testIndex++;
    }

    /* Two mem case */
    testIndex = 0;
    while(testIndex < 8) {
        SET_AVAILABLE_MEM_PAGES(2)
        parserContext.tokenOffset = START_OFFSETS[testIndex];

        if(VALUE_MEM_REQUIRED[testIndex] == 1 && INLINE_MEM_REQUIRED[testIndex] == 1 && COMMENT_MEM_REQUIRED[testIndex] == 1) {
            result = cs64_ini_parse_line(&parserContext);

            UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_set_entry_comment") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););

            /* The entry should still be there. */
            pEntry = cs64_ini_get_variable(parserContext.pData, NULL, (CS64UTF8*)KEY(testIndex));

            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");

            UNIT_TEST_ASSERT_NEQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)cs64_ini_get_entry_inline_comment(pEntry), INLINE_COMMENT) == 0, printf("Actually (%s) \n", cs64_ini_get_entry_inline_comment(pEntry)););

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }

        /* End of Test */
        testIndex++;
    }

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

void display_parser_context(CS64INIParserContext *pParserContext) {
    if(pParserContext == NULL) {
        printf("CS64INIParserContext is NULL\n");
        return;
    }
    printf("CS64INIParserContext is %p\n", pParserContext);

    printf("  pStringBuffer is %p\n", pParserContext->pStringBuffer);
    if(pParserContext->pStringBuffer != NULL) {
        printf("  pStringBuffer is %.512s\n", pParserContext->pStringBuffer);
    }
    printf("  pValueBuffer is %p\n",  pParserContext->pValueBuffer);
    if(pParserContext->pValueBuffer != NULL) {
        printf("  pStringBuffer is %.512s\n", pParserContext->pValueBuffer);
    }
    printf("  stringBufferLimit is %zu\n",  pParserContext->stringBufferLimit);
    printf("  pData is %p\n",  pParserContext->pData);
    printf("  pSection is %p\n",  pParserContext->pSection);
    printf("  pSource is %p\n",  pParserContext->pSource);
    printf("  tokenOffset is %zu\n",  pParserContext->tokenOffset);
    printf("  pTokenResult is %p\n",  pParserContext->pTokenResult);

}

void display_parser_result(CS64INIParserResult *pParserResult) {
    if(pParserResult == NULL) {
        printf("CS64INIParserResult is NULL\n");
        return;
    }
    printf("CS64INIParserResult is %p\n", pParserResult);

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
