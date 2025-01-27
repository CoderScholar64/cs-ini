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
void cs64_ini_section_memory_test();
void cs64_ini_variable_memory_test();
void cs64_ini_parser_expectation_test();
void cs64_ini_last_comment_test();

void display_parser_context(CS64INIParserContext *pParserContext);
void display_parser_result(CS64INIParserResult *pParserResult);
void cs64_ini_display_entry(const CS64INIEntry *const pEntry);
void cs64_ini_display_data(const CS64INIData *const pData);

int main() {
    cs64_ini_section_memory_test();
    cs64_ini_variable_memory_test();
    cs64_ini_parser_expectation_test();
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

void cs64_ini_section_memory_test() {
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

void cs64_ini_variable_memory_test() {
    /* Configure section names here */
    #define KEY(N) "key_" #N
    #define SHORT_VALUE "val"
    #define  LONG_VALUE "This is an intentially long value"

    static const CS64UTF8 key[8][6] = {
        KEY(0),
        KEY(1),
        KEY(2),
        KEY(3),
        KEY(4),
        KEY(5),
        KEY(6),
        KEY(7)};

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
                         KEY(0) " = "   SHORT_VALUE   "\n"                     /* 0 malloc */
                         KEY(1) " = "   SHORT_VALUE   ";" INLINE_COMMENT "\n"  /* 1 malloc */
        ";" COMMENT "\n" KEY(2) " = "   SHORT_VALUE   "\n"                     /* 1 malloc */
        ";" COMMENT "\n" KEY(3) " = "   SHORT_VALUE   ";" INLINE_COMMENT "\n"  /* 2 malloc */
                         KEY(4) " = \""  LONG_VALUE "\"\n"                     /* 1 malloc */
                         KEY(5) " = \""  LONG_VALUE "\";" INLINE_COMMENT "\n"  /* 2 malloc */
        ";" COMMENT "\n" KEY(6) " = \""  LONG_VALUE "\"\n"                     /* 2 malloc */
        ";" COMMENT "\n" KEY(7) " = \""  LONG_VALUE "\";" INLINE_COMMENT "\n", /* 3 malloc */
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
        pEntry = cs64_ini_get_variable(parserContext.pData, NULL, key[testIndex]);
        UNIT_TEST_ASSERT_NEQ(testIndex, pEntry, NULL, "%p");

        UNIT_TEST_ASSERT_NEQ(testIndex, cs64_ini_get_entry_name(pEntry), NULL, "%p");

        if(VALUE_MEM_REQUIRED[testIndex] == 1) {
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)cs64_ini_get_entry_value(pEntry), LONG_VALUE) == 0, printf("Actually (%s) \n", cs64_ini_get_entry_value(pEntry)););
        } else {
            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)cs64_ini_get_entry_value(pEntry), SHORT_VALUE) == 0, printf("Actually (%s) \n", cs64_ini_get_entry_value(pEntry)););
        }

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
            pEntry = cs64_ini_get_variable(parserContext.pData, NULL, key[testIndex]);
            UNIT_TEST_ASSERT_NEQ(testIndex, pEntry, NULL, "%p");

            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_inline_comment(pEntry), NULL, "%p");
            UNIT_TEST_ASSERT_EQ(testIndex, cs64_ini_get_entry_comment(pEntry), NULL, "%p");

            cs64_ini_del_entry(parserContext.pData, pEntry);
        }
        else if(VALUE_MEM_REQUIRED[testIndex] == 1) {
            result = cs64_ini_parse_line(&parserContext);

            UNIT_TEST_DETAIL_ASSERT(testIndex, result.state == CS64_INI_PARSER_INI_DATA_ERROR, display_parser_result(&result); display_parser_context(&parserContext););

            UNIT_TEST_DETAIL_ASSERT(testIndex, strcmp((char*)result.status.data_error.pFunctionName, "cs64_ini_add_variable") == 0, printf("Actually (%s) \n", result.status.data_error.pFunctionName););
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
            pEntry = cs64_ini_get_variable(parserContext.pData, NULL, key[testIndex]);
            UNIT_TEST_ASSERT_NEQ(testIndex, pEntry, NULL, "%p");

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
            pEntry = cs64_ini_get_variable(parserContext.pData, NULL, key[testIndex]);
            UNIT_TEST_ASSERT_NEQ(testIndex, pEntry, NULL, "%p");

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
            pEntry = cs64_ini_get_variable(parserContext.pData, NULL, key[testIndex]);
            UNIT_TEST_ASSERT_NEQ(testIndex, pEntry, NULL, "%p");

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
            pEntry = cs64_ini_get_variable(parserContext.pData, NULL, key[testIndex]);
            UNIT_TEST_ASSERT_NEQ(testIndex, pEntry, NULL, "%p");

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
            pEntry = cs64_ini_get_variable(parserContext.pData, NULL, key[testIndex]);
            UNIT_TEST_ASSERT_NEQ(testIndex, pEntry, NULL, "%p");

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


#define PARSE_LINE_TOKENLESS_SETUP(BUFFER_SIZE) \
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
    CS64INITokenResult tokenResult;\
    tokenResult.state = CS64_INI_LEXER_SUCCESS;\
    tokenResult.lineCount = 0;\
    tokenResult.linePosition = 0;\
    tokenResult.delimeterCount = 0;\
    tokenResult.sectionBeginCount = 0;\
    tokenResult.sectionEndCount = 0;\
    SET_AVAILABLE_MEM_PAGES(1)\
    tokenResult.pTokenStorage = cs64_ini_token_data_alloc();\
\
    SET_AVAILABLE_MEM_PAGES(2)\
    parserContext.pData = cs64_ini_data_alloc();\
    UNIT_TEST_ASSERT_NEQ(0, parserContext.pData, NULL, "%p");\
\
    parserContext.pSource = NULL;\
    parserContext.pTokenResult = &tokenResult;

#define UNEXPECTED_TOKEN_TEST(X)\
    pToken[X]->type = types[index];\
\
    parserContext.tokenOffset = 0;\
    result = cs64_ini_parse_line(&parserContext);\
    UNIT_TEST_DETAIL_ASSERT(index, result.state == CS64_INI_PARSER_UNEXPECTED_ERROR, display_parser_result(&result););\
    UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.receivedToken.type == types[index], display_parser_result(&result););\
    UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.receivedToken.index == 0, display_parser_result(&result););\
    UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.receivedToken.byteLength == 0, display_parser_result(&result););\
    UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens != NULL, display_parser_result(&result););

void cs64_ini_parser_expectation_test() {
    PARSE_LINE_TOKENLESS_SETUP(1024)

    CS64INIToken token;

    token.type       = CS64_INI_TOKEN_END;
    token.index      = 0;
    token.byteLength = 0;

    CS64INIToken *pToken[8];
    unsigned index = 0;

    SET_AVAILABLE_MEM_PAGES(1);
    while(index < sizeof(pToken) / sizeof(pToken[0])) {
        UNIT_TEST_ASSERT(index, cs64_ini_token_data_append_token(parserContext.pTokenResult->pTokenStorage, token));
        pToken[index] = cs64_ini_token_data_last_token(parserContext.pTokenResult->pTokenStorage);
        index++;
    }

    CS64INIParserResult result;

    {
        /*
         * Invalid Comment Test
         * X = 5 Combo = 1 Total = 5
         * Comment X{Delem, Value, Comment, SecE, SecB}
         */
        CS64INITokenType types[] = {
            CS64_INI_TOKEN_DELEMETER,
            CS64_INI_TOKEN_VALUE,
            CS64_INI_TOKEN_COMMENT,
            CS64_INI_TOKEN_SECTION_START,
            CS64_INI_TOKEN_SECTION_END};

        pToken[0]->type = CS64_INI_TOKEN_COMMENT;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(1);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 1, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_END, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_END;
        pToken[1]->type = CS64_INI_TOKEN_END;
    }

    {
        /*
         * Invalid Start Tests.
         * X = 2 Combo = 2 Total = 4
         * [Comment END] X{SecE, Delem}
         */
        CS64INITokenType types[] = {
            CS64_INI_TOKEN_DELEMETER,
            CS64_INI_TOKEN_SECTION_END};

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(0);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 4, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_SECTION_START, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[1] == CS64_INI_TOKEN_VALUE, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[2] == CS64_INI_TOKEN_COMMENT, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[3] == CS64_INI_TOKEN_END, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_COMMENT;
        pToken[1]->type = CS64_INI_TOKEN_END;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(2);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 4, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_SECTION_START, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[1] == CS64_INI_TOKEN_VALUE, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[2] == CS64_INI_TOKEN_COMMENT, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[3] == CS64_INI_TOKEN_END, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_END;
        pToken[1]->type = CS64_INI_TOKEN_END;
        pToken[2]->type = CS64_INI_TOKEN_END;
    }

    {
        /*
         * Section Tests.
         * X = 5 Combo = 2 Total = 10
         * [Comment END] SecB X{End, SecE, Comment, Delem, SecB}
         */
        CS64INITokenType types[] = {
            CS64_INI_TOKEN_DELEMETER,
            CS64_INI_TOKEN_COMMENT,
            CS64_INI_TOKEN_SECTION_START,
            CS64_INI_TOKEN_SECTION_END};

        pToken[0]->type = CS64_INI_TOKEN_SECTION_START;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(1);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 1, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_VALUE, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_COMMENT;
        pToken[1]->type = CS64_INI_TOKEN_END;
        pToken[2]->type = CS64_INI_TOKEN_SECTION_START;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(3);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 1, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_VALUE, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_END;
        pToken[1]->type = CS64_INI_TOKEN_END;
        pToken[2]->type = CS64_INI_TOKEN_END;
        pToken[3]->type = CS64_INI_TOKEN_END;
    }

    {
        /*
         * Section Tests.
         * X = 4 Combo = 2 Total =  8
         * [Comment END] SecB Values X{Delem, Comment, End, SecB}
         */
        CS64INITokenType types[] = {
            CS64_INI_TOKEN_DELEMETER,
            CS64_INI_TOKEN_COMMENT,
            CS64_INI_TOKEN_END,
            CS64_INI_TOKEN_SECTION_START};

        pToken[0]->type = CS64_INI_TOKEN_SECTION_START;
        pToken[1]->type = CS64_INI_TOKEN_VALUE;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(2);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 1, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_SECTION_END, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_COMMENT;
        pToken[1]->type = CS64_INI_TOKEN_END;
        pToken[2]->type = CS64_INI_TOKEN_SECTION_START;
        pToken[3]->type = CS64_INI_TOKEN_VALUE;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(4);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 1, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_SECTION_END, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_END;
        pToken[1]->type = CS64_INI_TOKEN_END;
        pToken[2]->type = CS64_INI_TOKEN_END;
        pToken[3]->type = CS64_INI_TOKEN_END;
        pToken[4]->type = CS64_INI_TOKEN_END;
    }

    {
        /*
         * Section Tests.
         * X = 4 Combo = 2 Total =  8
         * [Comment END] SecB Values SecE X{Delem, Value, SecB, SecE}
         */
        CS64INITokenType types[] = {
            CS64_INI_TOKEN_DELEMETER,
            CS64_INI_TOKEN_VALUE,
            CS64_INI_TOKEN_SECTION_START,
            CS64_INI_TOKEN_SECTION_END};

        pToken[0]->type = CS64_INI_TOKEN_SECTION_START;
        pToken[1]->type = CS64_INI_TOKEN_VALUE;
        pToken[2]->type = CS64_INI_TOKEN_SECTION_END;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(3);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 2, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_COMMENT, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[1] == CS64_INI_TOKEN_END, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_COMMENT;
        pToken[1]->type = CS64_INI_TOKEN_END;
        pToken[2]->type = CS64_INI_TOKEN_SECTION_START;
        pToken[3]->type = CS64_INI_TOKEN_VALUE;
        pToken[4]->type = CS64_INI_TOKEN_SECTION_END;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(5);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 2, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_COMMENT, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[1] == CS64_INI_TOKEN_END, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_END;
        pToken[1]->type = CS64_INI_TOKEN_END;
        pToken[2]->type = CS64_INI_TOKEN_END;
        pToken[3]->type = CS64_INI_TOKEN_END;
        pToken[4]->type = CS64_INI_TOKEN_END;
        pToken[5]->type = CS64_INI_TOKEN_END;
    }

    {
        /*
         * Section Tests.
         * X = 4 Combo = 2 Total =  8
         * [Comment END] SecB Values SecE Comment X{Delem, Value, SecB, SecE}
         */
        CS64INITokenType types[] = {
            CS64_INI_TOKEN_DELEMETER,
            CS64_INI_TOKEN_VALUE,
            CS64_INI_TOKEN_SECTION_START,
            CS64_INI_TOKEN_SECTION_END};

        pToken[0]->type = CS64_INI_TOKEN_SECTION_START;
        pToken[1]->type = CS64_INI_TOKEN_VALUE;
        pToken[2]->type = CS64_INI_TOKEN_SECTION_END;
        pToken[3]->type = CS64_INI_TOKEN_COMMENT;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(4);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 1, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_END, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_COMMENT;
        pToken[1]->type = CS64_INI_TOKEN_END;
        pToken[2]->type = CS64_INI_TOKEN_SECTION_START;
        pToken[3]->type = CS64_INI_TOKEN_VALUE;
        pToken[4]->type = CS64_INI_TOKEN_SECTION_END;
        pToken[5]->type = CS64_INI_TOKEN_COMMENT;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(6);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 1, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_END, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_END;
        pToken[1]->type = CS64_INI_TOKEN_END;
        pToken[2]->type = CS64_INI_TOKEN_END;
        pToken[3]->type = CS64_INI_TOKEN_END;
        pToken[4]->type = CS64_INI_TOKEN_END;
        pToken[5]->type = CS64_INI_TOKEN_END;
        pToken[6]->type = CS64_INI_TOKEN_END;
    }

    {
        /*
         * Variable Tests.
         * X = 4 Combo = 2 Total =  8
         * [Comment END] Values X{Comment, End, SecB, SecE}
         */
        CS64INITokenType types[] = {
            CS64_INI_TOKEN_COMMENT,
            CS64_INI_TOKEN_END,
            CS64_INI_TOKEN_SECTION_START,
            CS64_INI_TOKEN_SECTION_END};

        pToken[0]->type = CS64_INI_TOKEN_VALUE;
        pToken[2]->type = CS64_INI_TOKEN_END;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(1);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 1, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_DELEMETER, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_COMMENT;
        pToken[1]->type = CS64_INI_TOKEN_END;
        pToken[2]->type = CS64_INI_TOKEN_VALUE;
        pToken[4]->type = CS64_INI_TOKEN_END;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(3);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 1, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_DELEMETER, display_parser_result(&result););

            index++;
        }
    }

    {
        /*
         * Variable Tests.
         * X = 5 Combo = 2 Total = 10
         * [Comment END] Values Delem X{Delem, Comment, End, SecB, SecE}
         */
        CS64INITokenType types[] = {
            CS64_INI_TOKEN_DELEMETER,
            CS64_INI_TOKEN_COMMENT,
            CS64_INI_TOKEN_END,
            CS64_INI_TOKEN_SECTION_START,
            CS64_INI_TOKEN_SECTION_END};

        pToken[0]->type = CS64_INI_TOKEN_VALUE;
        pToken[1]->type = CS64_INI_TOKEN_DELEMETER;
        pToken[3]->type = CS64_INI_TOKEN_END;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(2);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 1, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_VALUE, display_parser_result(&result););

            index++;
        }

        pToken[0]->type = CS64_INI_TOKEN_COMMENT;
        pToken[1]->type = CS64_INI_TOKEN_END;
        pToken[2]->type = CS64_INI_TOKEN_VALUE;
        pToken[3]->type = CS64_INI_TOKEN_DELEMETER;
        pToken[5]->type = CS64_INI_TOKEN_END;

        index = 0;
        while(index < sizeof(types) / sizeof(types[0])) {
            UNEXPECTED_TOKEN_TEST(4);

            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.expectedTokenAmount == 1, display_parser_result(&result););
            UNIT_TEST_DETAIL_ASSERT(index, result.status.unexpected_token.pExpectedTokens[0] == CS64_INI_TOKEN_VALUE, display_parser_result(&result););

            index++;
        }
    }

    cs64_ini_data_free(parserContext.pData);
    cs64_ini_lexer_free(parserContext.pTokenResult);

    UNIT_TEST_MEM_CHECK_ASSERT

    /* Plan
     * [ ... ] Optional
     * { ... } Invalid Token Options
     *
     * Variable Tests.
     * X = 3 Combo = 2 Total =  6
     * [Comment END] Values Delem Values X{Delem, SecB, SecE}
     * X = 5 Combo = 2 Total = 10
     * [Comment END] Values Delem Values Comment X{Delem, Comment, Value, SecB, SecE}
     */
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
            printf("\tnext           = %p\n",  pEntry->pNext);
            printf("\tprevious       = %p\n",  pEntry->pPrev);
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
            printf("\tnext            = %p\n",  pEntry->pNext);
            printf("\tprevious        = %p\n",  pEntry->pPrev);
            printf("\tparent  section = %p\n",  pEntry->type.value.pSection);
            printf("\tname  byte size = %zd\n", pEntry->type.value.nameByteSize);
            printf("\tvalue byte size = %zd\n", pEntry->type.value.valueByteSize);
            printf("\tname            = %s\n", cs64_ini_get_entry_name(pEntry));
            printf("\tvalue           = %s\n", cs64_ini_get_entry_value(pEntry));
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

    printf("\tlast comment size    = %zd\n", pData->lastCommentSize);
    printf("\tlast comment pointer = %p\n",  pData->pLastComment);
    if(pData->pLastComment != NULL) {
        printf("\tlast comment = %s\n",  pData->pLastComment);
    }

    printf("\tglobals first value = %p\n", pData->globals.pFirstValue);
    printf("\tglobals last  value = %p\n", pData->globals.pLastValue);
    printf("\tfirst section = %p\n", pData->pFirstSection);
    printf("\tlast  section = %p\n", pData->pLastSection);

    CS64Size l = 0;
    while(l < pData->hashTable.entryCapacity) {
        printf("[%p] ", &pData->hashTable.pEntries[l]);
        cs64_ini_display_entry( &pData->hashTable.pEntries[l] );
        l++;
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
