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

int empty_alloc_test();
int fill_element_test();
int used_character_test();
int whitespace_character_test();
int value_character_test();
int comment_token_test();
int quote_value_token_test();

int main() {
    int testResult;

    testResult = empty_alloc_test();
    if(testResult != 0)
        return testResult;

    testResult = fill_element_test();
    if(testResult != 0)
        return testResult;

    testResult = used_character_test();
    if(testResult != 0)
        return testResult;

    testResult = whitespace_character_test();
    if(testResult != 0)
        return testResult;

    testResult = value_character_test();
    if(testResult != 0)
        return testResult;

    testResult = comment_token_test();
    if(testResult != 0)
        return testResult;

    testResult = quote_value_token_test();
    if(testResult != 0)
        return testResult;

    return testResult;
}

int empty_alloc_test() {
    mallocPagesLeft = 0;

    CS64INITokenData *pTokenData = cs64_ini_token_data_alloc();

    if(pTokenData != NULL) {
        printf("Error: cs64_ini_token_data_alloc was supposed to fail for empty_alloc_test case. Not return %p.\n", pTokenData);
        return 1;
    }

    mallocPagesLeft = 1;

    pTokenData = cs64_ini_token_data_alloc();

    if(pTokenData == NULL) {
        printf("Error empty_alloc_test: pTokenData failed to allocate! There is a very slight chance that the program ran out of memory.\n");
        return 2;
    }

    if(pTokenData->tokenAmount != 0) {
        printf("Error empty_alloc_test: expected 0, but go pTokenData->tokenAmount %zu.\n", pTokenData->tokenAmount);
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
        {CS64_INI_TOKEN_VALUE,          0, 12},
        {CS64_INI_TOKEN_DELEMETER,     12,  1},
        {CS64_INI_TOKEN_VALUE,         13,  7},
        {CS64_INI_TOKEN_END,           20,  1},
        {CS64_INI_TOKEN_SECTION_START, 21,  1}, // 4
        {CS64_INI_TOKEN_VALUE,         22,  6},
        {CS64_INI_TOKEN_SECTION_END,   28,  1},
        {CS64_INI_TOKEN_END,           29,  1},
        {CS64_INI_TOKEN_VALUE,         30,  6}, // 8
        {CS64_INI_TOKEN_DELEMETER,     36,  1},
        {CS64_INI_TOKEN_VALUE,         37, 14},
        {CS64_INI_TOKEN_END,           51,  1}
    };

    unsigned length = 1;
    while(length < sizeof(tokens) / sizeof(tokens[0])) {
        mallocPagesLeft = 1;
        CS64INITokenData *pTokenData = cs64_ini_token_data_alloc();

        if(pTokenData == NULL) {
            printf("Error fill_element_test: pTokenData failed to allocate! There is a very slight chance that the program ran out of memory.\n");
            return 1;
        }

        int appendResult;
        unsigned tokenIndex = 0;
        while(tokenIndex < length) {
            CS64INIToken *pToken = cs64_ini_token_data_get_token(pTokenData, tokenIndex);

            if(pToken != NULL) {
                printf("Error fill_element_test %i: pToken for cs64_ini_token_data_get_token(%i) is supposed to be empty not %p.\n", length, tokenIndex, pToken);
                return 2;
            }

            if(tokenIndex != 0 && (tokenIndex % CS64_INI_TOKEN_AMOUNT) == 0) {
                // First disable memory case with cs64_ini_token_data_append_token to see if it returns false and does nothing.
                mallocPagesLeft = 0;
                appendResult = cs64_ini_token_data_append_token(pTokenData, tokens[tokenIndex]);

                if(appendResult) {
                    printf("Error fill_element_test %i: pToken for cs64_ini_token_data_append_token(tokens[%i]) no allocation case failed.\n", length, tokenIndex);
                    return 3;
                }

                // Actually add the token.
                mallocPagesLeft = 1;
                appendResult = cs64_ini_token_data_append_token(pTokenData, tokens[tokenIndex]);

                if(!appendResult) {
                    printf("Error fill_element_test %i: pToken for cs64_ini_token_data_append_token(tokens[%i]) allocation case success.\n", length, tokenIndex);
                    return 4;
                }
            }
            else {
                // Actually add the token.
                mallocPagesLeft = 0;
                appendResult = cs64_ini_token_data_append_token(pTokenData, tokens[tokenIndex]);

                if(!appendResult) {
                    printf("Error fill_element_test %i: pToken for cs64_ini_token_data_append_token(tokens[%i]) normal case failure.\n", length, tokenIndex);
                    return 5;
                }
            }

            pToken = cs64_ini_token_data_get_token(pTokenData, tokenIndex);

            if(pToken == NULL) {
                printf("Error fill_element_test %i: pToken for cs64_ini_token_data_get_token(%i) is not supposed to be empty.\n", length, tokenIndex);
                return 6;
            }

            CS64INIToken *pLastToken = cs64_ini_token_data_last_token(pTokenData);

            if(pToken != pLastToken) {
                printf("Error fill_element_test %i: pToken for cs64_ini_token_data_last_token(%i) is pToken %p != pLastToken %p.\n", length, tokenIndex, pToken, pLastToken);
                return 7;
            }

            tokenIndex++;
        }

        tokenIndex = 0;
        while(tokenIndex < length) {
            CS64INIToken *pToken = cs64_ini_token_data_get_token(pTokenData, tokenIndex);

            if(pToken == NULL) {
                printf("Error fill_element_test %i: pToken for cs64_ini_token_data_get_token(%i) is not supposed to be empty.\n", length, tokenIndex);
                return 8;
            }

            if(tokens[tokenIndex].type != pToken->type || tokens[tokenIndex].index != pToken->index || tokens[tokenIndex].byteLength != pToken->byteLength) {
                printf("Error fill_element_test %i: memory did not match at index %i.\n", length, tokenIndex);

                unsigned x = 0;
                while(x < sizeof(tokens[0])) {
                    printf("%02x ", ((unsigned char*)&tokens[tokenIndex])[x]);
                    x++;
                }
                printf("\nTokens[%u] %i %zu %zu.\n", tokenIndex, tokens[tokenIndex].type, tokens[tokenIndex].index, tokens[tokenIndex].byteLength);

                x = 0;
                while(x < sizeof(tokens[0])) {
                    printf("%02x ", ((unsigned char*)pToken)[x]);
                    x++;
                }
                printf("\nToken %i %zu %zu.\n",            pToken->type,            pToken->index,            pToken->byteLength);
                return 9;
            }
            tokenIndex++;
        }

        cs64_ini_token_data_free(pTokenData);

        if(pointerTrackAmount != 0) {
            printf("Error fill_element_test: pointerTrackAmount is supposed to be zero after test not be %i.\n", pointerTrackAmount);
            return 10;
        }

        length++;
    }

    return 0;
}

int used_character_test() {
    CS64UniChar c = 0;

    while(c < 0x1000) {
        switch(c) {
            case CS64_INI_COMMENT:
            case CS64_INI_DELEMETER:
            case CS64_INI_END:
            case CS64_INI_SECTION_BEGIN:
            case CS64_INI_SECTION_END:
            case CS64_INI_VALUE_SLASH:
            case CS64_INI_VALUE_QUOTE:
                break;
            default:
                if(cs64_ini_is_character_used(c)) {
                    printf("Error used_character_test: character 0x%x is not an opcode, but cs64_ini_is_character_used return true.\n", c);
                    return 1;
                }
        }
        c++;
    }

    if(!cs64_ini_is_character_used(CS64_INI_COMMENT)) {
        printf("Error used_character_test: cs64_ini_is_character_used failed to detect CS64_INI_COMMENT.\n");
        return 2;
    }
    if(!cs64_ini_is_character_used(CS64_INI_DELEMETER)) {
        printf("Error used_character_test: cs64_ini_is_character_used failed to detect CS64_INI_DELEMETER.\n");
        return 3;
    }
    if(!cs64_ini_is_character_used(CS64_INI_END)) {
        printf("Error used_character_test: cs64_ini_is_character_used failed to detect CS64_INI_END.\n");
        return 4;
    }
    if(!cs64_ini_is_character_used(CS64_INI_SECTION_BEGIN)) {
        printf("Error used_character_test: cs64_ini_is_character_used failed to detect CS64_INI_SECTION_BEGIN.\n");
        return 5;
    }
    if(!cs64_ini_is_character_used(CS64_INI_SECTION_END)) {
        printf("Error used_character_test: cs64_ini_is_character_used failed to detect CS64_INI_SECTION_END.\n");
        return 6;
    }
    if(!cs64_ini_is_character_used(CS64_INI_VALUE_SLASH)) {
        printf("Error used_character_test: cs64_ini_is_character_used failed to detect CS64_INI_VALUE_SLASH.\n");
        return 7;
    }
    if(!cs64_ini_is_character_used(CS64_INI_VALUE_QUOTE)) {
        printf("Error used_character_test: cs64_ini_is_character_used failed to detect CS64_INI_VALUE_QUOTE.\n");
        return 8;
    }

    return 0;
}

int whitespace_character_test() {
    CS64UniChar c = 0;

    while(c < 0x1000) {
        switch(c) {
            case 0x0009:
            case 0x000a:
            case 0x000b:
            case 0x000c:
            case 0x000d:
            case 0x0020:
            case 0x0085:
            case 0x00a0:
            case 0x1680:
            case 0x180e:
            case 0x2000:
            case 0x2001:
            case 0x2002:
            case 0x2003:
            case 0x2004:
            case 0x2005:
            case 0x2006:
            case 0x2007:
            case 0x2008:
            case 0x2009:
            case 0x200a:
            case 0x200b:
            case 0x200c:
            case 0x200d:
            case 0x2028:
            case 0x2029:
            case 0x202f:
            case 0x205f:
            case 0x2060:
            case 0x3000:
            case 0xfeff:
                break;
            default:
                if(cs64_ini_is_character_whitespace(c)) {
                    printf("Error whitespace_character_test: character 0x%x is not whitespace, but cs64_ini_is_character_whitespace returned true.\n", c);
                    return 1;
                }
        }
        c++;
    }

    if(!cs64_ini_is_character_whitespace(0x000a)) {
        printf("Error whitespace_character_test: cs64_ini_is_character_whitespace failed to detect 0x0a.\n");
        return 2;
    }
    if(!cs64_ini_is_character_whitespace(0x000b)) {
        printf("Error whitespace_character_test: cs64_ini_is_character_whitespace failed to detect 0x0b.\n");
        return 3;
    }
    if(!cs64_ini_is_character_whitespace(0x000c)) {
        printf("Error whitespace_character_test: cs64_ini_is_character_whitespace failed to detect 0x0c.\n");
        return 4;
    }
    if(!cs64_ini_is_character_whitespace(0x000d)) {
        printf("Error whitespace_character_test: cs64_ini_is_character_whitespace failed to detect 0x0d.\n");
        return 5;
    }
    if(!cs64_ini_is_character_whitespace(0x0020)) {
        printf("Error whitespace_character_test: cs64_ini_is_character_whitespace failed to detect 0x20.\n");
        return 6;
    }
    if(!cs64_ini_is_character_whitespace(0x0085)) {
        printf("Error whitespace_character_test: cs64_ini_is_character_whitespace failed to detect 0x85.\n");
        return 7;
    }
    if(!cs64_ini_is_character_whitespace(0x00a0)) {
        printf("Error whitespace_character_test: cs64_ini_is_character_whitespace failed to detect 0xa0.\n");
        return 8;
    }
    return 0;
}

int value_character_test() {
    char characters[0x200] = {0};

    CS64UniChar c = 0x20;

    while(c < 0x7f) {
        if(cs64_ini_is_character_whitespace(c) || cs64_ini_is_character_used(c)) {
            c++;
            continue;
        }

        characters[c] = 1;
        c++;
    }

    c = 0xa0;
    while(c < 0x100) {
        if(cs64_ini_is_character_whitespace(c) || cs64_ini_is_character_used(c)) {
            c++;
            continue;
        }

        characters[c] = 1;
        c++;
    }

    characters[0xad] = 0;

    c = 0;
    while(c < sizeof(characters) / sizeof(characters[0])) {
        if(characters[c]) {
            if(!cs64_ini_is_character_value(c)) {
                printf("Error value_character_test: cs64_ini_is_character_value returned false for 0x%02x.\n", c);
                return 1;
            }
        }
        else {
            if( cs64_ini_is_character_value(c)) {
                printf("Error value_character_test: cs64_ini_is_character_value returned true for 0x%02x.\n", c);
                return 2;
            }
        }
        c++;
    }

    if(cs64_ini_is_character_value(CS64_INI_COMMENT)) {
        printf("Error used_character_test: cs64_ini_is_character_value should not return true for CS64_INI_COMMENT.\n");
        return 3;
    }
    if(cs64_ini_is_character_value(CS64_INI_DELEMETER)) {
        printf("Error used_character_test: cs64_ini_is_character_value should not return true for CS64_INI_DELEMETER.\n");
        return 4;
    }
    if(cs64_ini_is_character_value(CS64_INI_END)) {
        printf("Error used_character_test: cs64_ini_is_character_value should not return true for CS64_INI_END.\n");
        return 5;
    }
    if(cs64_ini_is_character_value(CS64_INI_SECTION_BEGIN)) {
        printf("Error used_character_test: cs64_ini_is_character_value should not return true for CS64_INI_SECTION_BEGIN.\n");
        return 6;
    }
    if(cs64_ini_is_character_value(CS64_INI_SECTION_END)) {
        printf("Error used_character_test: cs64_ini_is_character_value should not return true for CS64_INI_SECTION_END.\n");
        return 7;
    }
    if(cs64_ini_is_character_value(CS64_INI_VALUE_SLASH)) {
        printf("Error used_character_test: cs64_ini_is_character_value should not return true for CS64_INI_VALUE_SLASH.\n");
        return 8;
    }
    if(cs64_ini_is_character_value(CS64_INI_VALUE_QUOTE)) {
        printf("Error used_character_test: cs64_ini_is_character_value should not return true for CS64_INI_VALUE_QUOTE.\n");
        return 9;
    }

    return 0;
}

int comment_token_test() {
    CS64INITokenResult tokenResult = {0};

    // Bad bytes, but cs64_ini_tokenize_comment is not supposed to read them
    CS64UTF8 validCase1[][0x10] = {
        ";",
        ";\n",
        "; \n;",
        "; =[]\\\"",
        "; \n\xff\n",
        "; \nBlabla\xff\n",
        "; ;;;\nBlabla\n",
    };
    CS64INIToken validCase1Tokens[] = {
        {CS64_INI_TOKEN_COMMENT, 0, 1},
        {CS64_INI_TOKEN_COMMENT, 0, 1},
        {CS64_INI_TOKEN_COMMENT, 0, 2},
        {CS64_INI_TOKEN_COMMENT, 0, 7},
        {CS64_INI_TOKEN_COMMENT, 0, 2},
        {CS64_INI_TOKEN_COMMENT, 0, 2},
        {CS64_INI_TOKEN_COMMENT, 0, 5}
    };
    CS64Size validCase1linePositions[] = {
        1,
        1,
        2,
        7,
        2,
        2,
        5
    };

    unsigned i = 0;
    while(i < sizeof(validCase1) / sizeof(validCase1[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = 0;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen(validCase1[i]);
        CS64INIToken token = cs64_ini_tokenize_comment(&tokenResult, validCase1[i], length, 0);

        if(tokenResult.state != CS64_INI_LEXER_SUCCESS) {
            printf("Error comment_token_test Valid Case 1 Index %u. State expected CS64_INI_LEXER_SUCCESS got %u\n", i, tokenResult.state);
            return 1;
        }

        if(tokenResult.lineCount != 0) {
            printf("Error comment_token_test Valid Case 1 Index %u. Line Count expected 0 got %zu\n", i, tokenResult.lineCount);
            return 2;
        }

        if(tokenResult.linePosition != validCase1linePositions[i]) {
            printf("Error comment_token_test Valid Case 1 Index %u. Column expected %zu got %zu\n", i, validCase1linePositions[i], tokenResult.linePosition);
            return 3;
        }

        if(token.type != validCase1Tokens[i].type || token.index != validCase1Tokens[i].index || token.byteLength != validCase1Tokens[i].byteLength) {
            printf("Error comment_token_test Valid Case 1 Index %u. Tokens do not match.\n", i);
            printf("Expected %i %zu %zu.\n", validCase1Tokens[i].type, validCase1Tokens[i].index, validCase1Tokens[i].byteLength);
            printf("Returned %i %zu %zu.\n", token.type, token.index, token.byteLength);
            return 4;
        }

        i++;
    }

    // Bad bytes, but cs64_ini_tokenize_comment is not supposed to read them
    CS64UTF8 validCase2[][0x18] = {
        "w;",
        "wh;\n\xff",
        "whe; \n;",
        "wher; =[]\\\"",
        "where; \n\xff\n",
        "wherei; \nBlabla\n\xff",
        "whereis; ;;;\nBlabla\n",
    };
    CS64INIToken validCase2Tokens[] = {
        {CS64_INI_TOKEN_COMMENT, 1, 1},
        {CS64_INI_TOKEN_COMMENT, 2, 1},
        {CS64_INI_TOKEN_COMMENT, 3, 2},
        {CS64_INI_TOKEN_COMMENT, 4, 7},
        {CS64_INI_TOKEN_COMMENT, 5, 2},
        {CS64_INI_TOKEN_COMMENT, 6, 2},
        {CS64_INI_TOKEN_COMMENT, 7, 5}
    };
    CS64Size validCase2linePositions[] = {
        1 + 1,
        1 + 2,
        2 + 3,
        7 + 4,
        2 + 5,
        2 + 6,
        5 + 7
    };

    i = 0;
    while(i < sizeof(validCase2) / sizeof(validCase2[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = i + 1;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen(validCase2[i]);
        CS64INIToken token = cs64_ini_tokenize_comment(&tokenResult, validCase2[i], length, i + 1);

        if(tokenResult.state != CS64_INI_LEXER_SUCCESS) {
            printf("Error comment_token_test Valid Case 2 Index %u. State expected CS64_INI_LEXER_SUCCESS got %u\n", i, tokenResult.state);
            return 5;
        }

        if(tokenResult.lineCount != 0) {
            printf("Error comment_token_test Valid Case 2 Index %u. Line Count expected 0 got %zu\n", i, tokenResult.lineCount);
            return 6;
        }

        if(tokenResult.linePosition != validCase2linePositions[i]) {
            printf("Error comment_token_test Valid Case 2 Index %u. Column expected %zu got %zu\n", i, validCase2linePositions[i], tokenResult.linePosition);
            return 7;
        }

        if(token.type != validCase2Tokens[i].type || token.index != validCase2Tokens[i].index || token.byteLength != validCase2Tokens[i].byteLength) {
            printf("Error comment_token_test Valid Case 2 Index %u. Tokens do not match.\n", i);
            printf("Expected %i %zu %zu.\n", validCase2Tokens[i].type, validCase2Tokens[i].index, validCase2Tokens[i].byteLength);
            printf("Returned %i %zu %zu.\n", token.type, token.index, token.byteLength);
            return 8;
        }

        i++;
    }

    CS64UTF8 invalidCase[][0x18] = {
        "w;\xff",
        "wh;\xff\n\xff",
        "whe; \xff\n;",
        "wher;\xff =[]\\\"",
        "where; \xff\n\xff\n",
        "wherei;\xff \nBlabla\n",
        "whereis; ;;\xff\nBlabla\n",
    };
    CS64INIToken invalidCaseTokens[] = {
        {CS64_INI_TOKEN_COMMENT, 1, 0},
        {CS64_INI_TOKEN_COMMENT, 2, 0},
        {CS64_INI_TOKEN_COMMENT, 3, 0},
        {CS64_INI_TOKEN_COMMENT, 4, 0},
        {CS64_INI_TOKEN_COMMENT, 5, 0},
        {CS64_INI_TOKEN_COMMENT, 6, 0},
        {CS64_INI_TOKEN_COMMENT, 7, 0}
    };
    CS64Size invalidCaseLinePositions[] = {
        2,
        3,
        5,
        5,
        7,
        7,
        11
    };

    i = 0;
    while(i < sizeof(invalidCase) / sizeof(invalidCase[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = i + 1;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen(invalidCase[i]);
        CS64INIToken token = cs64_ini_tokenize_comment(&tokenResult, invalidCase[i], length, i + 1);

        if(tokenResult.state != CS64_INI_LEXER_ENCODING_ERROR) {
            printf("Error comment_token_test Invalid Case Index %u. State expected CS64_INI_LEXER_ENCODING_ERROR got %u\n", i, tokenResult.state);
            return 8;
        }
        else {
            if(tokenResult.status.encoding.badByteAmount == 0 || tokenResult.status.encoding.badBytes[0] != 0xff) {
                printf("Error comment_token_test Invalid Case Index %u. Length %u Bytes 0x%02x 0x%02x 0x%02x 0x%02x\n", i, tokenResult.status.encoding.badByteAmount, tokenResult.status.encoding.badBytes[0], tokenResult.status.encoding.badBytes[1], tokenResult.status.encoding.badBytes[2], tokenResult.status.encoding.badBytes[3]);
                return 9;
            }
        }

        if(tokenResult.lineCount != 0) {
            printf("Error comment_token_test Invalid Case Index %u. Line Count expected 0 got %zu\n", i, tokenResult.lineCount);
            return 10;
        }

        if(tokenResult.linePosition != invalidCaseLinePositions[i]) {
            printf("Error comment_token_test Invalid Case Index %u. Column expected %zu got %zu\n", i, invalidCaseLinePositions[i], tokenResult.linePosition);
            return 11;
        }

        if(token.type != invalidCaseTokens[i].type || token.index != invalidCaseTokens[i].index || token.byteLength != invalidCaseTokens[i].byteLength) {
            printf("Error comment_token_test Invalid Case Index %u. Tokens do not match.\n", i);
            printf("Expected %i %zu %zu.\n", invalidCaseTokens[i].type, invalidCaseTokens[i].index, invalidCaseTokens[i].byteLength);
            printf("Returned %i %zu %zu.\n", token.type, token.index, token.byteLength);
            return 12;
        }

        i++;
    }

    return 0;
}

int quote_value_token_test() {
    CS64INITokenResult tokenResult = {0};

    // Bad bytes, but cs64_ini_tokenize_value_quote is not supposed to read them
    CS64UTF8 validCase1[][0x10] = {
        "''",
        "';'\n",
        "\"\"\n;",
        "\"; =[]\\\"",
        "' \n'\xff",
        "' \nBlabla'\xff\n",
        "' ;;;\nBlabla\n'",
    };
    CS64INIToken validCase1Tokens[] = {
        {CS64_INI_TOKEN_VALUE, 0, 2},
        {CS64_INI_TOKEN_VALUE, 0, 3},
        {CS64_INI_TOKEN_VALUE, 0, 2},
        {CS64_INI_TOKEN_VALUE, 0, 8},
        {CS64_INI_TOKEN_VALUE, 0, 4},
        {CS64_INI_TOKEN_VALUE, 0, 10},
        {CS64_INI_TOKEN_VALUE, 0, 14}
    };
    CS64Size validCase1linePositions[] = {
        2,
        3,
        2,
        8,
        1,
        7,
        1
    };
    CS64Size validCase1LineCount[] = {
        0,
        0,
        0,
        0,
        1,
        1,
        2
    };

    unsigned i = 0;
    while(i < sizeof(validCase1) / sizeof(validCase1[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = 0;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen(validCase1[i]);
        CS64INIToken token = cs64_ini_tokenize_value_quote(&tokenResult, validCase1[i], length, 0);

        if(tokenResult.state != CS64_INI_LEXER_SUCCESS) {
            printf("Error quote_value_token_test Valid Case 1 Index %u. State expected CS64_INI_LEXER_SUCCESS got %u\n", i, tokenResult.state);
            return 1;
        }

        if(tokenResult.lineCount != validCase1LineCount[i]) {
            printf("Error quote_value_token_test Valid Case 1 Index %u. Line Count expected %zu got %zu\n", i, validCase1LineCount[i], tokenResult.lineCount);
            return 2;
        }

        if(tokenResult.linePosition != validCase1linePositions[i]) {
            printf("Error quote_value_token_test Valid Case 1 Index %u. Column expected %zu got %zu\n%s\n", i, validCase1linePositions[i], tokenResult.linePosition, validCase1[i]);
            return 3;
        }

        if(token.type != validCase1Tokens[i].type || token.index != validCase1Tokens[i].index || token.byteLength != validCase1Tokens[i].byteLength) {
            printf("Error quote_value_token_test Valid Case 1 Index %u. Tokens do not match.\n", i);
            printf("Expected %i %zu %zu.\n", validCase1Tokens[i].type, validCase1Tokens[i].index, validCase1Tokens[i].byteLength);
            printf("Returned %i %zu %zu.\n", token.type, token.index, token.byteLength);
            return 4;
        }

        i++;
    }

    // Bad bytes, but cs64_ini_tokenize_value_quote is not supposed to read them
    CS64UTF8 validCase2[][0x18] = {
        "w''",
        "wh'\n'\xff",
        "whe' \n';",
        "wher\" =[]\\\"",
        "where\" \"\xff\n",
        "wherei\" \"Blabla\n\xff",
        "whereis\" ;;;\"Blabla\n",
    };
    CS64INIToken validCase2Tokens[] = {
        {CS64_INI_TOKEN_VALUE, 1, 2},
        {CS64_INI_TOKEN_VALUE, 2, 3},
        {CS64_INI_TOKEN_VALUE, 3, 4},
        {CS64_INI_TOKEN_VALUE, 4, 7},
        {CS64_INI_TOKEN_VALUE, 5, 3},
        {CS64_INI_TOKEN_VALUE, 6, 3},
        {CS64_INI_TOKEN_VALUE, 7, 6}
    };
    CS64Size validCase2linePositions[] = {
        3,
        1,
        1,
        7 + 4,
        3 + 5,
        3 + 6,
        6 + 7
    };
    CS64Size validCase2LineCount[] = {
        0,
        1,
        1,
        0,
        0,
        0,
        0
    };

    i = 0;
    while(i < sizeof(validCase2) / sizeof(validCase2[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = i + 1;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen(validCase2[i]);
        CS64INIToken token = cs64_ini_tokenize_value_quote(&tokenResult, validCase2[i], length, i + 1);

        if(tokenResult.state != CS64_INI_LEXER_SUCCESS) {
            printf("Error quote_value_token_test Valid Case 2 Index %u. State expected CS64_INI_LEXER_SUCCESS got %u\n", i, tokenResult.state);
            return 5;
        }

        if(tokenResult.lineCount != validCase2LineCount[i]) {
            printf("Error quote_value_token_test Valid Case 2 Index %u. Line Count expected %zu got %zu\n", i, validCase2LineCount[i], tokenResult.lineCount);
            return 6;
        }

        if(tokenResult.linePosition != validCase2linePositions[i]) {
            printf("Error quote_value_token_test Valid Case 2 Index %u. Column expected %zu got %zu\n", i, validCase2linePositions[i], tokenResult.linePosition);
            return 7;
        }

        if(token.type != validCase2Tokens[i].type || token.index != validCase2Tokens[i].index || token.byteLength != validCase2Tokens[i].byteLength) {
            printf("Error quote_value_token_test Valid Case 2 Index %u. Tokens do not match.\n", i);
            printf("Expected %i %zu %zu.\n", validCase2Tokens[i].type, validCase2Tokens[i].index, validCase2Tokens[i].byteLength);
            printf("Returned %i %zu %zu.\n", token.type, token.index, token.byteLength);
            return 8;
        }

        i++;
    }

    CS64UTF8 invalidCase[][0x18] = {
        "w'\xff'",
        "wh\"\xff\"\xff",
        "whe\" \xff\";",
        "wher\"\xff\n=[]\\\"",
        "where\"\n\xff\"\xff\n",
        "wherei\"\xff \"Blabla\n",
        "whereis\"\n;\n\xff\"Blabla\n",
    };
    CS64INIToken invalidCaseTokens[] = {
        {CS64_INI_TOKEN_VALUE, 1, 0},
        {CS64_INI_TOKEN_VALUE, 2, 0},
        {CS64_INI_TOKEN_VALUE, 3, 0},
        {CS64_INI_TOKEN_VALUE, 4, 0},
        {CS64_INI_TOKEN_VALUE, 5, 0},
        {CS64_INI_TOKEN_VALUE, 6, 0},
        {CS64_INI_TOKEN_VALUE, 7, 0}
    };
    CS64Size invalidCaseLinePositions[] = {
        2,
        3,
        5,
        5,
        0,
        7,
        0
    };
    CS64Size invalidCaseLineCount[] = {
        0,
        0,
        0,
        0,
        1,
        0,
        2
    };

    i = 0;
    while(i < sizeof(invalidCase) / sizeof(invalidCase[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = i + 1;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen(invalidCase[i]);
        CS64INIToken token = cs64_ini_tokenize_value_quote(&tokenResult, invalidCase[i], length, i + 1);

        if(tokenResult.state != CS64_INI_LEXER_ENCODING_ERROR) {
            printf("Error quote_value_token_test Invalid Case Index %u. State expected CS64_INI_LEXER_ENCODING_ERROR got %u\n", i, tokenResult.state);
            return 8;
        }
        else {
            if(tokenResult.status.encoding.badByteAmount == 0 || tokenResult.status.encoding.badBytes[0] != 0xff) {
                printf("Error quote_value_token_test Invalid Case Index %u. Length %u Bytes 0x%02x 0x%02x 0x%02x 0x%02x\n", i, tokenResult.status.encoding.badByteAmount, tokenResult.status.encoding.badBytes[0], tokenResult.status.encoding.badBytes[1], tokenResult.status.encoding.badBytes[2], tokenResult.status.encoding.badBytes[3]);
                return 9;
            }
        }

        if(tokenResult.lineCount != invalidCaseLineCount[i]) {
            printf("Error quote_value_token_test Invalid Case Index %u. Line Count expected %zu got %zu\n", i, invalidCaseLineCount[i], tokenResult.lineCount);
            return 10;
        }

        if(tokenResult.linePosition != invalidCaseLinePositions[i]) {
            printf("Error quote_value_token_test Invalid Case Index %u. Column expected %zu got %zu\n", i, invalidCaseLinePositions[i], tokenResult.linePosition);
            return 11;
        }

        if(token.type != invalidCaseTokens[i].type || token.index != invalidCaseTokens[i].index || token.byteLength != invalidCaseTokens[i].byteLength) {
            printf("Error quote_value_token_test Invalid Case Index %u. Tokens do not match.\n", i);
            printf("Expected %i %zu %zu.\n", invalidCaseTokens[i].type, invalidCaseTokens[i].index, invalidCaseTokens[i].byteLength);
            printf("Returned %i %zu %zu.\n", token.type, token.index, token.byteLength);
            return 12;
        }

        i++;
    }

    /*
    printf("validCase1Tokens = {");
        printf("\n{CS64_INI_TOKEN_COMMENT, %zu, %zu},", token.index, token.byteLength);
    printf("\n};\n");
    */

    return 0;
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
