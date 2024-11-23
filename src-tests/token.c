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
                printf("\nTokens[%u] %i %zu %zu.\n", tokenIndex, tokens[tokenIndex].type, tokens[tokenIndex].index, tokens[tokenIndex].byteLength);x = 0;
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

    CS64UTF8 validCase1[][0x10] = {
        ";",
        ";\n",
        "; \n;",
        "; =[]\\\"",
        "; \n\n",
        "; \nBlabla\n",
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

    unsigned i = 0;
    while(i < sizeof(validCase1) / sizeof(validCase1[0])) {
        tokenResult.state         = CS64_INI_LEXER_SUCCESS;
        tokenResult.lineCount     = 0;
        tokenResult.linePosition  = 0;
        tokenResult.pTokenStorage = NULL;

        CS64Size length = strlen(validCase1[i]);
        CS64INIToken token = cs64_ini_tokenize_comment(&tokenResult, validCase1[i], length, 0);

        printf("linePosition = %zu\n", tokenResult.linePosition);

        i++;
    }

    /*
    printf("validCase1Tokens = {");
        printf("\n{CS64_INI_TOKEN_COMMENT, %zu, %zu},", token.index, token.byteLength);
    printf("\n};\n");
    */

    return 1;
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
