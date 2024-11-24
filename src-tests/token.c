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

// Data Storage Test
int empty_alloc_test();
int fill_element_test();
// Character Tests
int used_character_test();
int whitespace_character_test();
int value_character_test();
// Tokenizer Tests
int comment_token_test();
int value_token_test();
int quote_value_token_test();
// Lexer Test
int lexer_test();

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

    testResult = value_token_test();
    if(testResult != 0)
        return testResult;

    testResult = quote_value_token_test();
    if(testResult != 0)
        return testResult;

    testResult = lexer_test();
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

        CS64Size length = strlen((char*)validCase1[i]);
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

        CS64Size length = strlen((char*)validCase2[i]);
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

    CS64UTF8 encodingErrorCase[][0x18] = {
        "w;\xff",
        "wh;\xff\n\xff",
        "whe; \xff\n;",
        "wher;\xff =[]\\\"",
        "where; \xff\n\xff\n",
        "wherei;\xff \nBlabla\n",
        "whereis; ;;\xff\nBlabla\n",
    };
    CS64INIToken encodingErrorCaseTokens[] = {
        {CS64_INI_TOKEN_COMMENT, 1, 0},
        {CS64_INI_TOKEN_COMMENT, 2, 0},
        {CS64_INI_TOKEN_COMMENT, 3, 0},
        {CS64_INI_TOKEN_COMMENT, 4, 0},
        {CS64_INI_TOKEN_COMMENT, 5, 0},
        {CS64_INI_TOKEN_COMMENT, 6, 0},
        {CS64_INI_TOKEN_COMMENT, 7, 0}
    };
    CS64Size encodingErrorCaseLinePositions[] = {
        2,
        3,
        5,
        5,
        7,
        7,
        11
    };

    i = 0;
    while(i < sizeof(encodingErrorCase) / sizeof(encodingErrorCase[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = i + 1;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen((char*)encodingErrorCase[i]);
        CS64INIToken token = cs64_ini_tokenize_comment(&tokenResult, encodingErrorCase[i], length, i + 1);

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

        if(tokenResult.linePosition != encodingErrorCaseLinePositions[i]) {
            printf("Error comment_token_test Invalid Case Index %u. Column expected %zu got %zu\n", i, encodingErrorCaseLinePositions[i], tokenResult.linePosition);
            return 11;
        }

        if(token.type != encodingErrorCaseTokens[i].type || token.index != encodingErrorCaseTokens[i].index || token.byteLength != encodingErrorCaseTokens[i].byteLength) {
            printf("Error comment_token_test Invalid Case Index %u. Tokens do not match.\n", i);
            printf("Expected %i %zu %zu.\n", encodingErrorCaseTokens[i].type, encodingErrorCaseTokens[i].index, encodingErrorCaseTokens[i].byteLength);
            printf("Returned %i %zu %zu.\n", token.type, token.index, token.byteLength);
            return 12;
        }

        i++;
    }

    return 0;
}

int value_token_test() {
    CS64INITokenResult tokenResult = {0};

    CS64UTF8 validCase1[][0x10] = {
        "Ab\nbla",
        "Cde=",
        "Fghi;",
        "Jklmn[",
        "oPqrst]",
        "uNwxyzNo\\owI",
        "BlablaBla\"Bla",
    };
    CS64INIToken validCase1Tokens[] = {
        {CS64_INI_TOKEN_VALUE, 0, 2},
        {CS64_INI_TOKEN_VALUE, 0, 3},
        {CS64_INI_TOKEN_VALUE, 0, 4},
        {CS64_INI_TOKEN_VALUE, 0, 5},
        {CS64_INI_TOKEN_VALUE, 0, 6},
        {CS64_INI_TOKEN_VALUE, 0, 8},
        {CS64_INI_TOKEN_VALUE, 0, 9}
    };
    CS64Size validCase1linePositions[] = {
        2,
        3,
        4,
        5,
        6,
        8,
        9
    };

    unsigned i = 0;
    while(i < sizeof(validCase1) / sizeof(validCase1[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = 0;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen((char*)validCase1[i]);
        CS64INIToken token = cs64_ini_tokenize_value(&tokenResult, validCase1[i], length, 0);

        if(tokenResult.state != CS64_INI_LEXER_SUCCESS) {
            printf("Error value_token_test Valid Case 1 Index %u. State expected CS64_INI_LEXER_SUCCESS got %u\n", i, tokenResult.state);
            return 1;
        }

        if(tokenResult.lineCount != 0) {
            printf("Error value_token_test Valid Case 1 Index %u. Line Count expected %u got %zu\n", i, 0, tokenResult.lineCount);
            return 2;
        }

        if(tokenResult.linePosition != validCase1linePositions[i]) {
            printf("Error value_token_test Valid Case 1 Index %u. Column expected %zu got %zu\n%s\n", i, validCase1linePositions[i], tokenResult.linePosition, validCase1[i]);
            return 3;
        }

        if(token.type != validCase1Tokens[i].type || token.index != validCase1Tokens[i].index || token.byteLength != validCase1Tokens[i].byteLength) {
            printf("Error value_token_test Valid Case 1 Index %u. Tokens do not match.\n", i);
            printf("Expected %i %zu %zu.\n", validCase1Tokens[i].type, validCase1Tokens[i].index, validCase1Tokens[i].byteLength);
            printf("Returned %i %zu %zu.\n", token.type, token.index, token.byteLength);
            return 4;
        }

        i++;
    }

    CS64UTF8 validCase2[][0x18] = {
        "$$$$$$$Ab\nbla",
        "@@@@@@@@@@Cde=",
        "555555555Fghi;",
        ";;;;;;;;Jklmn[",
        ":::::::oPqrst]",
        "__uNwxyzNo\\ow",
        "0BlablaBla\"Ba",
    };
    CS64INIToken validCase2Tokens[] = {
        {CS64_INI_TOKEN_VALUE, 10, 3},
        {CS64_INI_TOKEN_VALUE, 10, 3},
        {CS64_INI_TOKEN_VALUE,  9, 4},
        {CS64_INI_TOKEN_VALUE,  8, 5},
        {CS64_INI_TOKEN_VALUE,  7, 6},
        {CS64_INI_TOKEN_VALUE,  2, 8},
        {CS64_INI_TOKEN_VALUE,  1, 9}
    };
    CS64Size validCase2linePositions[] = {
        13,
        13,
        13,
        13,
        13,
        10,
        10
    };

    i = 0;
    while(i < sizeof(validCase2) / sizeof(validCase2[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = validCase2Tokens[i].index;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen((char*)validCase2[i]);
        CS64INIToken token = cs64_ini_tokenize_value(&tokenResult, validCase2[i], length, tokenResult.linePosition);

        if(tokenResult.state != CS64_INI_LEXER_SUCCESS) {
            printf("Error value_token_test Valid Case 2 Index %u. State expected CS64_INI_LEXER_SUCCESS got %u\n", i, tokenResult.state);
            return 5;
        }

        if(tokenResult.lineCount != 0) {
            printf("Error value_token_test Valid Case 2 Index %u. Line Count expected %u got %zu\n", i, 0, tokenResult.lineCount);
            return 6;
        }

        if(tokenResult.linePosition != validCase2linePositions[i]) {
            printf("Error value_token_test Valid Case 2 Index %u. Column expected %zu got %zu\n", i, validCase2linePositions[i], tokenResult.linePosition);
            return 7;
        }

        if(token.type != validCase2Tokens[i].type || token.index != validCase2Tokens[i].index || token.byteLength != validCase2Tokens[i].byteLength) {
            printf("Error value_token_test Valid Case 2 Index %u. Tokens do not match.\n", i);
            printf("Expected %i %zu %zu.\n", validCase2Tokens[i].type, validCase2Tokens[i].index, validCase2Tokens[i].byteLength);
            printf("Returned %i %zu %zu.\n", token.type, token.index, token.byteLength);
            return 8;
        }

        i++;
    }

    CS64UTF8 encodingErrorCase[][0x18] = {
        "wT\xff'",
        "whTo\xff\"\xff",
        "wheTok\xff\";",
        "wherToke\xff[]\\\"",
        "whereToken\xff",
        "whereiTokens\xffla\n",
        "whereisTokensY\xfflabla\n",
    };
    CS64INIToken encodingErrorCaseTokens[] = {
        {CS64_INI_TOKEN_VALUE, 1, 0},
        {CS64_INI_TOKEN_VALUE, 2, 0},
        {CS64_INI_TOKEN_VALUE, 3, 0},
        {CS64_INI_TOKEN_VALUE, 4, 0},
        {CS64_INI_TOKEN_VALUE, 5, 0},
        {CS64_INI_TOKEN_VALUE, 6, 0},
        {CS64_INI_TOKEN_VALUE, 7, 0}
    };
    CS64Size encodingErrorCaseLinePositions[] = {
         2,
         4,
         6,
         8,
        10,
        12,
        14
    };

    i = 0;
    while(i < sizeof(encodingErrorCase) / sizeof(encodingErrorCase[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = i + 1;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen((char*)encodingErrorCase[i]);
        CS64INIToken token = cs64_ini_tokenize_value(&tokenResult, encodingErrorCase[i], length, i + 1);

        if(tokenResult.state != CS64_INI_LEXER_ENCODING_ERROR) {
            printf("Error value_token_test Invalid Case Index %u. State expected CS64_INI_LEXER_ENCODING_ERROR got %u\n", i, tokenResult.state);
            return 8;
        }
        else {
            if(tokenResult.status.encoding.badByteAmount == 0 || tokenResult.status.encoding.badBytes[0] != 0xff) {
                printf("Error value_token_test Invalid Case Index %u. Length %u Bytes 0x%02x 0x%02x 0x%02x 0x%02x\n", i, tokenResult.status.encoding.badByteAmount, tokenResult.status.encoding.badBytes[0], tokenResult.status.encoding.badBytes[1], tokenResult.status.encoding.badBytes[2], tokenResult.status.encoding.badBytes[3]);
                return 9;
            }
        }

        if(tokenResult.lineCount != 0) {
            printf("Error value_token_test Invalid Case Index %u. Line Count expected %u got %zu\n", i, 0, tokenResult.lineCount);
            return 10;
        }

        if(tokenResult.linePosition != encodingErrorCaseLinePositions[i]) {
            printf("Error value_token_test Invalid Case Index %u. Column expected %zu got %zu\n", i, encodingErrorCaseLinePositions[i], tokenResult.linePosition);
            return 11;
        }

        if(token.type != encodingErrorCaseTokens[i].type || token.index != encodingErrorCaseTokens[i].index || token.byteLength != encodingErrorCaseTokens[i].byteLength) {
            printf("Error value_token_test Invalid Case Index %u. Tokens do not match.\n", i);
            printf("Expected %i %zu %zu.\n", encodingErrorCaseTokens[i].type, encodingErrorCaseTokens[i].index, encodingErrorCaseTokens[i].byteLength);
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
        "\"; =[]\\\"\"",
        "' \n'\xff",
        "' \nBlabla'\xff\n",
        "' ;;;\nBlabla\n'",
    };
    CS64INIToken validCase1Tokens[] = {
        {CS64_INI_TOKEN_QUOTE_VALUE, 0, 2},
        {CS64_INI_TOKEN_QUOTE_VALUE, 0, 3},
        {CS64_INI_TOKEN_QUOTE_VALUE, 0, 2},
        {CS64_INI_TOKEN_QUOTE_VALUE, 0, 9},
        {CS64_INI_TOKEN_QUOTE_VALUE, 0, 4},
        {CS64_INI_TOKEN_QUOTE_VALUE, 0, 10},
        {CS64_INI_TOKEN_QUOTE_VALUE, 0, 14}
    };
    CS64Size validCase1linePositions[] = {
        2,
        3,
        2,
        9,
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

        CS64Size length = strlen((char*)validCase1[i]);
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
        "wher\" =[]\\\"\"",
        "where\" \"\xff\n",
        "wherei\" \"Blabla\n\xff",
        "whereis\" \\\";\"Blabla\n",
    };
    CS64INIToken validCase2Tokens[] = {
        {CS64_INI_TOKEN_QUOTE_VALUE, 1, 2},
        {CS64_INI_TOKEN_QUOTE_VALUE, 2, 3},
        {CS64_INI_TOKEN_QUOTE_VALUE, 3, 4},
        {CS64_INI_TOKEN_QUOTE_VALUE, 4, 8},
        {CS64_INI_TOKEN_QUOTE_VALUE, 5, 3},
        {CS64_INI_TOKEN_QUOTE_VALUE, 6, 3},
        {CS64_INI_TOKEN_QUOTE_VALUE, 7, 6}
    };
    CS64Size validCase2linePositions[] = {
        3,
        1,
        1,
        8 + 4,
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

        CS64Size length = strlen((char*)validCase2[i]);
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

    CS64UTF8 encodingErrorCase[][0x18] = {
        "w'\xff'",
        "wh\"\xff\"\xff",
        "whe\" \xff\";",
        "wher\"\xff\n=[]\\\"",
        "where\"\n\xff\"\xff\n",
        "wherei\"\xff \"Blabla\n",
        "whereis\"\n;\n\xff\"Blabla\n",
    };
    CS64INIToken encodingErrorCaseTokens[] = {
        {CS64_INI_TOKEN_QUOTE_VALUE, 1, 0},
        {CS64_INI_TOKEN_QUOTE_VALUE, 2, 0},
        {CS64_INI_TOKEN_QUOTE_VALUE, 3, 0},
        {CS64_INI_TOKEN_QUOTE_VALUE, 4, 0},
        {CS64_INI_TOKEN_QUOTE_VALUE, 5, 0},
        {CS64_INI_TOKEN_QUOTE_VALUE, 6, 0},
        {CS64_INI_TOKEN_QUOTE_VALUE, 7, 0}
    };
    CS64Size encodingErrorCaseLinePositions[] = {
        2,
        3,
        5,
        5,
        0,
        7,
        0
    };
    CS64Size encodingErrorCaseLineCount[] = {
        0,
        0,
        0,
        0,
        1,
        0,
        2
    };

    i = 0;
    while(i < sizeof(encodingErrorCase) / sizeof(encodingErrorCase[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = i + 1;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen((char*)encodingErrorCase[i]);
        CS64INIToken token = cs64_ini_tokenize_value_quote(&tokenResult, encodingErrorCase[i], length, i + 1);

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

        if(tokenResult.lineCount != encodingErrorCaseLineCount[i]) {
            printf("Error quote_value_token_test Invalid Case Index %u. Line Count expected %zu got %zu\n", i, encodingErrorCaseLineCount[i], tokenResult.lineCount);
            return 10;
        }

        if(tokenResult.linePosition != encodingErrorCaseLinePositions[i]) {
            printf("Error quote_value_token_test Invalid Case Index %u. Column expected %zu got %zu\n", i, encodingErrorCaseLinePositions[i], tokenResult.linePosition);
            return 11;
        }

        if(token.type != encodingErrorCaseTokens[i].type || token.index != encodingErrorCaseTokens[i].index || token.byteLength != encodingErrorCaseTokens[i].byteLength) {
            printf("Error quote_value_token_test Invalid Case Index %u. Tokens do not match.\n", i);
            printf("Expected %i %zu %zu.\n", encodingErrorCaseTokens[i].type, encodingErrorCaseTokens[i].index, encodingErrorCaseTokens[i].byteLength);
            printf("Returned %i %zu %zu.\n", token.type, token.index, token.byteLength);
            return 12;
        }

        i++;
    }

    CS64UTF8 badEndErrorCase[][0x18] = {
        "w' \\'",
        "wh\"\\\"",
        "whe\" \\\";",
        "wher\" \n=[]\\\"",
        "where\"\n\\\" \n",
        "wherei\" \\\"Blabla\n",
        "whereis\"\n;\n\\\"Blabla",
    };
    CS64INIToken badEndErrorCaseTokens[] = {
        {CS64_INI_TOKEN_QUOTE_VALUE, 1, 0},
        {CS64_INI_TOKEN_QUOTE_VALUE, 2, 0},
        {CS64_INI_TOKEN_QUOTE_VALUE, 3, 0},
        {CS64_INI_TOKEN_QUOTE_VALUE, 4, 0},
        {CS64_INI_TOKEN_QUOTE_VALUE, 5, 0},
        {CS64_INI_TOKEN_QUOTE_VALUE, 6, 0},
        {CS64_INI_TOKEN_QUOTE_VALUE, 7, 0}
    };
    CS64Size badEndErrorCaseLinePositions[] = {
        5,
        5,
        8,
        5,
        0,
        0,
        8
    };
    CS64Size badEndErrorCaseLineCount[] = {
        0,
        0,
        0,
        1,
        2,
        1,
        2
    };

    i = 0;
    while(i < sizeof(badEndErrorCase) / sizeof(badEndErrorCase[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = i + 1;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen((char*)badEndErrorCase[i]);
        CS64INIToken token = cs64_ini_tokenize_value_quote(&tokenResult, badEndErrorCase[i], length, i + 1);

        if(tokenResult.state != CS64_INI_LEXER_EXPECTED_ERROR) {
            if(tokenResult.state == CS64_INI_LEXER_ENCODING_ERROR) {
                printf("Error quote_value_token_test Bad End Case Index %u. Length %u Bytes 0x%02x 0x%02x 0x%02x 0x%02x\n", i, tokenResult.status.encoding.badByteAmount, tokenResult.status.encoding.badBytes[0], tokenResult.status.encoding.badBytes[1], tokenResult.status.encoding.badBytes[2], tokenResult.status.encoding.badBytes[3]);
                return 13;
            }
            else {
                printf("Error quote_value_token_test Bad End Case Index %u. State expected CS64_INI_LEXER_EXPECTED_ERROR got %u\n", i, tokenResult.state);
                return 14;
            }
        }
        else {
            if( tokenResult.status.expected.result != badEndErrorCase[i][length - 1] ) {
                if(tokenResult.status.expected.result != tokenResult.status.expected.expected)
                    printf("Error quote_value_token_test Bad End Case Index %u. Expected 0x%x not 0x%x.\n", i, tokenResult.status.expected.expected, tokenResult.status.expected.result);
                else
                    printf("Error quote_value_token_test Bad End Case Index %u. 0x%x has been escaped. Thus the string is not completed.\n", i, tokenResult.status.expected.expected);
                return 15;
            }
        }

        if(tokenResult.lineCount != badEndErrorCaseLineCount[i]) {
            printf("Error quote_value_token_test Bad End Case Index %u. Line Count expected %zu got %zu\n", i, badEndErrorCaseLineCount[i], tokenResult.lineCount);
            return 16;
        }

        if(tokenResult.linePosition != badEndErrorCaseLinePositions[i]) {
            printf("Error quote_value_token_test Bad End Case Index %u. Column expected %zu got %zu\n", i, badEndErrorCaseLinePositions[i], tokenResult.linePosition);
            return 17;
        }

        if(token.type != badEndErrorCaseTokens[i].type || token.index != badEndErrorCaseTokens[i].index || token.byteLength != badEndErrorCaseTokens[i].byteLength) {
            printf("Error quote_value_token_test Bad End Case Index %u. Tokens do not match.\n", i);
            printf("Expected %i %zu %zu.\n", badEndErrorCaseTokens[i].type, badEndErrorCaseTokens[i].index, badEndErrorCaseTokens[i].byteLength);
            printf("Returned %i %zu %zu.\n", token.type, token.index, token.byteLength);
            return 18;
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

int lexer_test() {
    mallocPagesLeft = 0;

    CS64INITokenResult tokenResult = cs64_ini_lexer("", 0);

    if(tokenResult.state != CS64_INI_LEXER_NO_MEMORY_ERROR || tokenResult.lineCount != 0 || tokenResult.linePosition != 0) {
        printf("Error lexer_test cs64_ini_lexer was supposed to return CS64_INI_LEXER_NO_MEMORY_ERROR but got %u. Line number %zu. Column %zu\n", tokenResult.state, tokenResult.lineCount, tokenResult.linePosition);
        cs64_ini_lexer_free(&tokenResult);
        return 1;
    }
    cs64_ini_lexer_free(&tokenResult);

    if(pointerTrackAmount != 0) {
        printf("Error lexer_test Outright fail case: pointerTrackAmount is supposed to be zero after test not be %i.\n", pointerTrackAmount);
        return 2;
    }

    mallocPagesLeft = 1;

    tokenResult = cs64_ini_lexer("[][][][][][]", 12);

    if(tokenResult.state != CS64_INI_LEXER_NO_MEMORY_ERROR || tokenResult.lineCount != 1 || tokenResult.linePosition != 4) {
        printf("Error lexer_test cs64_ini_lexer was supposed to return CS64_INI_LEXER_NO_MEMORY_ERROR but got %u. Line number %zu. Column %zu\n", tokenResult.state, tokenResult.lineCount, tokenResult.linePosition);
        cs64_ini_lexer_free(&tokenResult);
        return 3;
    }
    cs64_ini_lexer_free(&tokenResult);

    if(pointerTrackAmount != 0) {
        printf("Error lexer_test: pointerTrackAmount is supposed to be zero after test not be %i.\n", pointerTrackAmount);
        return 4;
    }

    mallocPagesLeft = 7;

    // Now for the real test
    CS64UTF8 fileData[] = ";\n\nkey =\tvalue; Commenter\n[Section] ; comment\nkey = \"value\"; Comment\n\n\n; Comment\n\"key2\" = value value2\"\tvalue3\"; Comment";
    CS64Size fileDataSize = sizeof(fileData) / sizeof(fileData[0]) - 1;
    CS64INIToken expectedFileDataTokens[] = {
        {CS64_INI_TOKEN_COMMENT, 0, 1},
        {CS64_INI_TOKEN_END, 1, 2},
        {CS64_INI_TOKEN_VALUE, 3, 3},
        {CS64_INI_TOKEN_DELEMETER, 7, 1},
        {CS64_INI_TOKEN_VALUE, 9, 5},
        {CS64_INI_TOKEN_COMMENT, 14, 11},
        {CS64_INI_TOKEN_END, 25, 1},
        {CS64_INI_TOKEN_SECTION_START, 26, 1},
        {CS64_INI_TOKEN_VALUE, 27, 7},
        {CS64_INI_TOKEN_SECTION_END, 34, 1},
        {CS64_INI_TOKEN_COMMENT, 36, 9},
        {CS64_INI_TOKEN_END, 45, 1},
        {CS64_INI_TOKEN_VALUE, 46, 3},
        {CS64_INI_TOKEN_DELEMETER, 50, 1},
        {CS64_INI_TOKEN_QUOTE_VALUE, 52, 7},
        {CS64_INI_TOKEN_COMMENT, 59, 9},
        {CS64_INI_TOKEN_END, 68, 3},
        {CS64_INI_TOKEN_COMMENT, 71, 9},
        {CS64_INI_TOKEN_END, 80, 1},
        {CS64_INI_TOKEN_QUOTE_VALUE, 81, 6},
        {CS64_INI_TOKEN_DELEMETER, 88, 1},
        {CS64_INI_TOKEN_VALUE, 90, 12},
        {CS64_INI_TOKEN_QUOTE_VALUE, 102, 9},
        {CS64_INI_TOKEN_COMMENT, 111, 9},
        {CS64_INI_TOKEN_END, 120, 0}
    };

    tokenResult = cs64_ini_lexer(fileData, fileDataSize);

    if(tokenResult.state != CS64_INI_LEXER_SUCCESS) {
        printf("Error lexer_test: fileData did not produce CS64_INI_LEXER_SUCCESS, but returned %u.\n", tokenResult.state);

        CS64Size tokenIndex = 0;

        CS64INIToken *pToken = cs64_ini_token_data_get_token(tokenResult.pTokenStorage, tokenIndex);
        while(pToken != NULL) {
            printf("%i %zu %zu\n", pToken->type, pToken->index, pToken->byteLength);
            tokenIndex++;
            pToken = cs64_ini_token_data_get_token(tokenResult.pTokenStorage, tokenIndex);
        }

        cs64_ini_lexer_free(&tokenResult);

        return 5;
    }

    CS64Size tokenIndex = 0;
    CS64INIToken *pToken = cs64_ini_token_data_get_token(tokenResult.pTokenStorage, tokenIndex);
    while(pToken != NULL) {
        if(expectedFileDataTokens[tokenIndex].type != pToken->type || expectedFileDataTokens[tokenIndex].index != pToken->index || expectedFileDataTokens[tokenIndex].byteLength != pToken->byteLength) {
            printf("Expected %i %zu %zu\n", expectedFileDataTokens[tokenIndex].type, expectedFileDataTokens[tokenIndex].index, expectedFileDataTokens[tokenIndex].byteLength);
            printf("Returned %i %zu %zu\n", pToken->type, pToken->index, pToken->byteLength);
            cs64_ini_lexer_free(&tokenResult);
            return 6;
        }
        tokenIndex++;
        pToken = cs64_ini_token_data_get_token(tokenResult.pTokenStorage, tokenIndex);
    }
    cs64_ini_lexer_free(&tokenResult);

    mallocPagesLeft = 7;

    CS64UTF8 fileBadQuoteData[] = ";\n\nkey =\tvalue; Commenter\n[Section] ; comment\nkey = \"value\"; Comment\n\n\n; Comment\n\"key2\" = value value2\"\tvalue3";
    CS64Size fileBadQuoteDataSize = sizeof(fileBadQuoteData) / sizeof(fileBadQuoteData[0]) - 1;

    tokenResult = cs64_ini_lexer(fileBadQuoteData, fileBadQuoteDataSize);

    if(tokenResult.state == CS64_INI_LEXER_EXPECTED_ERROR) {
        if(tokenResult.status.expected.expected != '\"' || tokenResult.status.expected.result != '3' || tokenResult.lineCount != 9 || tokenResult.linePosition != 29) {
            printf("Error lexer_test: fileBadQuoteData produced CS64_INI_LEXER_EXPECTED_ERROR, but expected 0x%x while 0x%x.\n", tokenResult.status.expected.expected, tokenResult.status.expected.result);
            printf("    Line Count = %zu; Column = %zu\n", tokenResult.lineCount, tokenResult.linePosition);

            cs64_ini_lexer_free(&tokenResult);

            return 7;
        }
    }
    else {
        printf("Error lexer_test: fileBadQuoteData did not produce CS64_INI_LEXER_EXPECTED_ERROR, but returned %u.\n", tokenResult.state);

        CS64Size tokenIndex = 0;

        CS64INIToken *pToken = cs64_ini_token_data_get_token(tokenResult.pTokenStorage, tokenIndex);
        while(pToken != NULL) {
            printf("%i %zu %zu\n", pToken->type, pToken->index, pToken->byteLength);
            tokenIndex++;
            pToken = cs64_ini_token_data_get_token(tokenResult.pTokenStorage, tokenIndex);
        }

        cs64_ini_lexer_free(&tokenResult);

        return 8;
    }
    cs64_ini_lexer_free(&tokenResult);

    mallocPagesLeft = 3;

    unsigned badBytePlacements[] = {
        0,
        2,
        12,
        32};

    CS64UTF8 expectedBadBytes[][4] = {
        {0xff, 0x53, 0x65, 0x63},
        {0xff, 0x63, 0x74, 0x69},
        {0xff, 0x72, 0x72, 0x75},
        {0xff, 0x75, 0x65, 0x22}
    };

    CS64Size expectedBadByteLineNumber[] = {
        1,
        1,
        1,
        3
    };

    CS64Size expectedBadByteColumns[] = {
        0,
        2,
        13, // 12
        8 // 9
    };

    unsigned i = 0;
    while(i < sizeof(badBytePlacements) / sizeof(badBytePlacements[0])) {
        mallocPagesLeft = 3;

        CS64UTF8 fileToBeBadData[] = "[Section]; Corruption\n\nkey =\t\"value\"";
        CS64Size fileToBeBadDataSize = sizeof(fileToBeBadData) / sizeof(fileToBeBadData[0]) - 1;

        fileToBeBadData[badBytePlacements[i]] = 0xff;

        tokenResult.status.encoding.badBytes[0] = 0;
        tokenResult.status.encoding.badBytes[1] = 0;
        tokenResult.status.encoding.badBytes[2] = 0;
        tokenResult.status.encoding.badBytes[3] = 0;

        tokenResult = cs64_ini_lexer(fileToBeBadData, fileToBeBadDataSize);

        if(tokenResult.state == CS64_INI_LEXER_ENCODING_ERROR) {
            if(tokenResult.status.encoding.badByteAmount != 4 ||
                tokenResult.status.encoding.badBytes[0] != expectedBadBytes[i][0] ||
                tokenResult.status.encoding.badBytes[1] != expectedBadBytes[i][1] ||
                tokenResult.status.encoding.badBytes[2] != expectedBadBytes[i][2] ||
                tokenResult.status.encoding.badBytes[3] != expectedBadBytes[i][3] ||
                tokenResult.lineCount != expectedBadByteLineNumber[i] || tokenResult.linePosition != expectedBadByteColumns[i]) {
                printf("Error lexer_test: fileToBeBadData %i produced CS64_INI_LEXER_ENCODING_ERROR, Length %u {0x%x, 0x%x, 0x%x, 0x%x}.\n", i, tokenResult.status.encoding.badByteAmount, tokenResult.status.encoding.badBytes[0], tokenResult.status.encoding.badBytes[1], tokenResult.status.encoding.badBytes[2], tokenResult.status.encoding.badBytes[3]);
                printf("    Line Count = %zu; Column = %zu\n", tokenResult.lineCount, tokenResult.linePosition);

                cs64_ini_lexer_free(&tokenResult);

                return 9;
            }
        }
        else {
            printf("Error lexer_test: fileToBeBadData %i did not produce CS64_INI_LEXER_ENCODING_ERROR, but returned %u.\n", badBytePlacements[i], tokenResult.state);

            CS64Size tokenIndex = 0;

            CS64INIToken *pToken = cs64_ini_token_data_get_token(tokenResult.pTokenStorage, tokenIndex);
            while(pToken != NULL) {
                printf("%i %zu %zu\n", pToken->type, pToken->index, pToken->byteLength);
                tokenIndex++;
                pToken = cs64_ini_token_data_get_token(tokenResult.pTokenStorage, tokenIndex);
            }

            cs64_ini_lexer_free(&tokenResult);

            return 10;
        }
        cs64_ini_lexer_free(&tokenResult);
        i++;
    }

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
