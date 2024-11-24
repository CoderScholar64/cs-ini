#ifndef CS64_INI_LIBRARY_H
#define CS64_INI_LIBRARY_H

#if !defined(CS64_INI_MALLOC) || !defined(CS64_INI_FREE)
    #include <stdlib.h>
#endif

#ifndef CS64_INI_MALLOC
    #define CS64_INI_MALLOC(size) malloc(size)
#endif

#ifndef CS64_INI_FREE
    #define CS64_INI_FREE(pointer) free(pointer)
#endif

#ifndef CS64_INI_TOKEN_AMOUNT
    #define CS64_INI_TOKEN_AMOUNT 338
#endif

#if CS64_INI_TOKEN_AMOUNT < 4
    #error CS64_INI_TOKEN_AMOUNT should be more than 4. Reason: the implementation for storing tokens might break at 1 and would definitely break at zero.
#endif

#ifndef CS64UTF8
    #ifdef UINT8_MAX
        #define CS64UTF8 uint8_t
    #else
        #error uint8_t is required for CS64UTF8. Include stdint.h and/or stddef.h if available.
    #endif
#endif

#ifndef CS64UniChar
    #ifdef UINT32_MAX
        #define CS64UniChar uint32_t
    #else
        #error uint32_t is required for CS64UniChar. Include stdint.h and/or stddef.h if available.
    #endif
#endif

#ifndef CS64Size
    #ifdef SIZE_MAX
        #define CS64Size size_t
    #else
        #error size_t is required for CS64Size. Include stdint.h and/or stddef.h if available.
    #endif
#endif

#ifndef CS64Offset
    #ifdef SIZE_MAX
        #define CS64Offset size_t
    #else
        #error size_t is required for CS64Offset. Include stdint.h and/or stddef.h if available.
    #endif
#endif

/* Infulenced from rini another ini parsing library from raysan5. */
#ifndef CS64_INI_COMMENT
    #define CS64_INI_COMMENT       ((CS64UniChar)';')
#endif

#ifndef CS64_INI_DELEMETER
    #define CS64_INI_DELEMETER     ((CS64UniChar)'=')
#endif

#ifndef CS64_INI_END
    #define CS64_INI_END           ((CS64UniChar)'\n')
#endif

#ifndef CS64_INI_SECTION_BEGIN
    #define CS64_INI_SECTION_BEGIN ((CS64UniChar)'[')
#endif

#ifndef CS64_INI_SECTION_END
    #define CS64_INI_SECTION_END   ((CS64UniChar)']')
#endif

#ifndef CS64_INI_VALUE_SLASH
    #define CS64_INI_VALUE_SLASH   ((CS64UniChar)'\\')
#endif

#ifndef CS64_INI_VALUE_QUOTE
    #define CS64_INI_VALUE_QUOTE   ((CS64UniChar)'"')
#endif

typedef enum {
    CS64_INI_MAX_CODE            = 0x10ffff,
    CS64_INI_MAX_CODE_AMOUNT     = 0x110000,
    CS64_INI_BAD_NOT_ASCII       = 0x110001,
    CS64_INI_BAD_NOT_UTF_8       = 0x110002,
    CS64_INI_BAD_OVERLONG        = 0x110003,
    CS64_INI_BAD_LACK_SPACE      = 0x110004,
    CS64_INI_BAD_TOO_BIG         = 0x110005,
    CS64_INI_BAD_CONTINUE_BYTE_0 = 0x110006,
    CS64_INI_BAD_CONTINUE_BYTE_1 = 0x110007,
    CS64_INI_BAD_CONTINUE_BYTE_2 = 0x110008,
    CS64_INI_BAD_CONTINUE_BYTE_3 = 0x110009
} CS64UniCharCode;

typedef enum {
    CS64_INI_LEXER_SUCCESS              = 0,
    CS64_INI_LEXER_NO_MEMORY_ERROR      = 1,
    CS64_INI_LEXER_ENCODING_ERROR       = 2,
    CS64_INI_LEXER_EXPECTED_ERROR       = 3,
    CS64_INI_LEXER_UNHANDLED_CHAR_ERROR = 4
} CS64INILexerState;

typedef enum {
    /* CS64_INI_TOKEN_WHITE_SPACE would not be stored anyways. */
    CS64_INI_TOKEN_DELEMETER,     /* DELEMETER */
    CS64_INI_TOKEN_VALUE,         /* VALUE can be qouted */
    CS64_INI_TOKEN_COMMENT,       /* COMMENT_START *Every character except CS64_INI_TOKEN_END */
    CS64_INI_TOKEN_END,           /* New Line */
    CS64_INI_TOKEN_SECTION_START, /* Start of section */
    CS64_INI_TOKEN_SECTION_END    /* End of section */
} CS64INITokenType;

typedef struct {
    CS64INITokenType type;
    CS64Offset       index;
    CS64Offset       byteLength;
} CS64INIToken;

typedef struct CS64INITokenArrayList {
    struct CS64INITokenArrayList *pNext;
    CS64INIToken tokens[CS64_INI_TOKEN_AMOUNT];
} CS64INITokenArrayList;

typedef struct {
    CS64Size tokenAmount;
    CS64INITokenArrayList *pLastPage;
    CS64INITokenArrayList  firstPage;
} CS64INITokenData;

typedef struct {
    CS64INILexerState state;
    CS64Size lineCount; /* the number of lines that has been processed. */
    CS64Size linePosition; /* in the amount of unicode points not bytes. Useful for lexer errors. */
    CS64INITokenData *pTokenStorage;
    union {
        struct {
            unsigned badByteAmount;
            CS64UTF8 badBytes[4];
        } encoding;
        struct {
            CS64UniChar expected;
            CS64UniChar result;
        } expected;
        struct {
            CS64UniChar unhandled;
        } unhandled;
    } status;
} CS64INITokenResult;

CS64INITokenData* cs64_ini_token_data_alloc();
int cs64_ini_token_data_append_token(CS64INITokenData *pData, CS64INIToken token);
CS64INIToken* cs64_ini_token_data_last_token(CS64INITokenData *pData);
CS64INIToken* cs64_ini_token_data_get_token(CS64INITokenData *pData, CS64Size tokenIndex);
void cs64_ini_token_data_free(CS64INITokenData *pData);

/* CS64INIToken cs64_ini_token_merge(const CS64INIToken *const pLastToken, const CS64INIToken *const pNextToken); */ /* TODO Make tokens mergable. */
int cs64_ini_is_character_used(CS64UniChar character);
int cs64_ini_is_character_value(CS64UniChar character);
int cs64_ini_is_character_whitespace(CS64UniChar character);
CS64INIToken cs64_ini_tokenize_comment(CS64INITokenResult *pResult, const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize, CS64Size UTF8Offset);
CS64INIToken cs64_ini_tokenize_value(CS64INITokenResult *pResult, const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize, CS64Size UTF8Offset);
CS64INIToken cs64_ini_tokenize_value_quote(CS64INITokenResult *pResult, const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize, CS64Size UTF8Offset);
CS64INITokenResult cs64_ini_lexer(const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize);

/**
 * This function reads an ASCII value.
 * @warning UTF-8 characters need to stay within the ASCII range.
 * @param pDataHead Pointer to the current byte in the UTF-8 stream. @warning NULL is not optional. This value must be present.
 * @param remainingDataSize The space left to read. Used to prevent buffer overflows. @warning make sure you subtract this value using *pCharacterByteSize to successfully test agaisnt buffer-overflows.
 * @param pCharacterByteSize Pointer to a variable storing the read character size. If zero then either NULL has been found or error. (updated by the function)
 * @return 0-127 if there is no error. CS64_INI_BAD_NOT_ASCII is the stream is not ASCII.
 */
CS64UniChar cs64_ini_ascii_read(const CS64UTF8 *const pDataHead, CS64Size remainingDataSize, CS64Size *pCharacterByteSize);

/**
 * This function reads a UTF-8 value.
 * @param pDataHead Pointer to the current byte in the UTF-8 stream. @warning NULL is not optional. This value must be present.
 * @param remainingDataSize The space left to read. Used to prevent buffer overflows. @warning make sure you subtract this value using *pCharacterByteSize to successfully test agaisnt buffer-overflows.
 * @param pCharacterByteSize Pointer to a variable storing the read character size. If zero then either NULL has been found or error. (updated by the function)
 * @return Any value that is lesser than or equal to CS64_INI_MAX_CODE is a correct UTF-8 character.
 */
CS64UniChar cs64_ini_utf_8_read(const CS64UTF8 *const pDataHead, CS64Size remainingDataSize, CS64Size *pCharacterByteSize);

/**
 * This function implements the encoding scheme for converting Unicode characters into UTF-8 byte sequences. UTF-8 uses a variable number of bytes to represent different characters: 1 byte for ASCII, 2 bytes for most common extended characters, and up to 4 bytes for rare Unicode characters.
 * @param pDataHead A pointer to an array of bytes where the UTF-8 representation will be written.
 * @param remainingDataSize The size (in bytes) of the buffer pointed to by pDataHead. This is used to prevent writing beyond the allocated memory.
 * @param character The Unicode character to be converted to UTF-8.
 * @return  The function returns the number of bytes written to the buffer, indicating the length of the UTF-8 representation. If a character is too big for the buffer then return 0. If the character is out of unicode range then -1.
 */
int cs64_ini_utf_8_write(CS64UTF8 *pDataHead, CS64Size remainingDataSize, CS64UniChar character);

#endif /* CS64_INI_LIBRARY_H */

#ifdef CS64_INI_LIBRARY_IMP

 /* ### TEXT HANDLING SECTION ### */

CS64UniChar cs64_ini_ascii_read(const CS64UTF8 *const pDataHead, CS64Size remainingDataSize, CS64Size *pCharacterByteSize) {
    if(remainingDataSize == 0) {
        *pCharacterByteSize = 0; /* Indicate that the loop calling this function should end. */
        return CS64_INI_BAD_LACK_SPACE;
    }
    else if(*pDataHead >= 0x80) {
        *pCharacterByteSize = 0; /* Indicate that the loop calling this function should end. */
        return CS64_INI_BAD_NOT_ASCII;
    }

    *pCharacterByteSize = 1; /* Indicate that there are more bytes to be read. */
    return *pDataHead;
}

#define CHECK_NEVER_BYTE( byte ) (byte == 0xc0 || byte == 0xc1 || (byte >= 0xf5 && byte <= 0xff))
#define IS_CHAR_CONTINUATION( byte ) ((byte & 0b11000000) == 0b10000000)
#define        IS_UTF_1_BYTE( byte ) ((byte & 0b10000000) == 0)
#define        IS_UTF_2_BYTE( byte ) ((byte & 0b11100000) == 0b11000000)
#define        IS_UTF_3_BYTE( byte ) ((byte & 0b11110000) == 0b11100000)
#define        IS_UTF_4_BYTE( byte ) ((byte & 0b11111000) == 0b11110000)

#define IS_UTF_OVERLONG_3_BYTE(byte_0, byte_1) ((byte_0 & 0b00001111) == 0 && (byte_1 & 0b00100000) == 0)
#define IS_UTF_OVERLONG_4_BYTE(byte_0, byte_1) ((byte_0 & 0b00000111) == 0 && (byte_1 & 0b00110000) == 0)

#define IS_UTF_OVERSIZED(byte_0, byte_1) (byte_0 == 0xf4 && byte_1 >= 0x90)

CS64UniChar cs64_ini_utf_8_read(const CS64UTF8 *const pDataHead, CS64Size remainingDataSize, CS64Size *pCharacterByteSize) {
    CS64UniChar result;

    *pCharacterByteSize = 0; /* Indicate that the loop calling this function should end. */

    if(remainingDataSize == 0)
        return CS64_INI_BAD_LACK_SPACE;
    else if(CHECK_NEVER_BYTE(pDataHead[0]))
        return CS64_INI_BAD_NOT_UTF_8;
    else if(IS_CHAR_CONTINUATION(pDataHead[0]))
        return CS64_INI_BAD_CONTINUE_BYTE_0;
    else if(IS_UTF_1_BYTE(pDataHead[0])) {
        *pCharacterByteSize = 1;
        return pDataHead[0];
    }
    else if(IS_UTF_2_BYTE(pDataHead[0])) {
        if(remainingDataSize < 2)
            return CS64_INI_BAD_LACK_SPACE;
        else if(!IS_CHAR_CONTINUATION(pDataHead[1])) /* Check if byte is continuous. */
            return CS64_INI_BAD_CONTINUE_BYTE_1;

        result  = ((pDataHead[0] & 0b00011111) << 6);
        result |= ((pDataHead[1] & 0b00111111) << 0);

        *pCharacterByteSize = 2;

        return result;
    }
    else if(IS_UTF_3_BYTE(pDataHead[0])) {
        if(remainingDataSize < 3)
            return CS64_INI_BAD_LACK_SPACE;
        else if(!IS_CHAR_CONTINUATION(pDataHead[1])) /* Check if byte is continuous. */
            return CS64_INI_BAD_CONTINUE_BYTE_1;
        else if(!IS_CHAR_CONTINUATION(pDataHead[2])) /* Check if byte is continuous. */
            return CS64_INI_BAD_CONTINUE_BYTE_2;
        else if(IS_UTF_OVERLONG_3_BYTE(pDataHead[0], pDataHead[1])) /* Check for overlong error. */
            return CS64_INI_BAD_OVERLONG;

        result  = ((pDataHead[0] & 0b00001111) << 12);
        result |= ((pDataHead[1] & 0b00111111) <<  6);
        result |= ((pDataHead[2] & 0b00111111) <<  0);

        *pCharacterByteSize = 3;

        return result;
    }
    else if(IS_UTF_4_BYTE(pDataHead[0])) {
        if(remainingDataSize < 4)
            return CS64_INI_BAD_LACK_SPACE;
        else if(!IS_CHAR_CONTINUATION(pDataHead[1])) /* Check if byte is continuous. */
            return CS64_INI_BAD_CONTINUE_BYTE_1;
        else if(!IS_CHAR_CONTINUATION(pDataHead[2])) /* Check if byte is continuous. */
            return CS64_INI_BAD_CONTINUE_BYTE_2;
        else if(!IS_CHAR_CONTINUATION(pDataHead[3])) /* Check if byte is continuous. */
            return CS64_INI_BAD_CONTINUE_BYTE_3;
        else if(IS_UTF_OVERLONG_4_BYTE(pDataHead[0], pDataHead[1])) /* Check for overlong error. */
            return CS64_INI_BAD_OVERLONG;
        else if(IS_UTF_OVERSIZED(pDataHead[0], pDataHead[1])) /* Check for over-size case. */
            return CS64_INI_BAD_TOO_BIG;

        result  = ((pDataHead[0] & 0b00000111) << 18);
        result |= ((pDataHead[1] & 0b00111111) << 12);
        result |= ((pDataHead[2] & 0b00111111) <<  6);
        result |= ((pDataHead[3] & 0b00111111) <<  0);

        *pCharacterByteSize = 4;

        return result;
    }

    return CS64_INI_BAD_NOT_UTF_8;
}

/* Clean up Macros! */
#undef CHECK_NEVER_BYTE
#undef IS_CHAR_CONTINUATION
#undef IS_UTF_1_BYTE
#undef IS_UTF_2_BYTE
#undef IS_UTF_3_BYTE
#undef IS_UTF_4_BYTE

#undef IS_UTF_OVERLONG_3_BYTE
#undef IS_UTF_OVERLONG_4_BYTE

#undef IS_UTF_OVERSIZED

int cs64_ini_utf_8_write(CS64UTF8 *pDataHead, CS64Size remainingDataSize, CS64UniChar character) {
    if(character < 0x80) { /* ASCII range. UTF-8 1 Byte Case. */
        if(remainingDataSize < 1)
            return 0; /* Not Enough Space */

        pDataHead[0] = character;
        return 1; /* 1 Byte used. */
    }
    else if(character < 0x800) { /* UTF-8 2 Byte Case. */
        if(remainingDataSize < 2)
            return 0; /* Not Enough Space */

        pDataHead[1] = 0b10000000 | (0b00111111 & (character >> 0));
        pDataHead[0] = 0b11000000 | (0b00011111 & (character >> 6));
        return 2; /* 2 Bytes used. */
    }
    else if(character < 0x10000) { /* UTF-8 3 Byte Case. */
        if(remainingDataSize < 3)
            return 0; /* Not Enough Space */

        pDataHead[2] = 0b10000000 | (0b00111111 & (character >>  0));
        pDataHead[1] = 0b10000000 | (0b00111111 & (character >>  6));
        pDataHead[0] = 0b11100000 | (0b00001111 & (character >> 12));
        return 3; /* 3 Bytes used. */
    }
    else if(character < 0x110000) { /* UTF-8 4 Byte Case. */
        if(remainingDataSize < 4)
            return 0; /* Not Enough Space */

        pDataHead[3] = 0b10000000 | (0b00111111 & (character >>  0));
        pDataHead[2] = 0b10000000 | (0b00111111 & (character >>  6));
        pDataHead[1] = 0b10000000 | (0b00111111 & (character >> 12));
        pDataHead[0] = 0b11110000 | (0b00000111 & (character >> 18));
        return 4; /* 4 Bytes used. */
    }
    else
        return -1; /* character is too big for this algorithm. */
}

/* ### Token Storage */

CS64INITokenData* cs64_ini_token_data_alloc() {
    CS64INITokenData *pData = CS64_INI_MALLOC(sizeof(CS64INITokenData));

    if(pData != NULL) {
        pData->tokenAmount = 0;
        pData->pLastPage = &pData->firstPage;
        pData->firstPage.pNext = NULL;

        return pData;
    }
    else
        return NULL;
}

int cs64_ini_token_data_append_token(CS64INITokenData *pData, CS64INIToken token) {
    if(pData->tokenAmount != 0 && (pData->tokenAmount % CS64_INI_TOKEN_AMOUNT) == 0) {
        pData->pLastPage->pNext = CS64_INI_MALLOC(sizeof(CS64INITokenArrayList));

        if(pData->pLastPage->pNext == NULL)
            return 0;

        pData->pLastPage = pData->pLastPage->pNext;
        pData->pLastPage->pNext = NULL;
    }

    pData->pLastPage->tokens[pData->tokenAmount % CS64_INI_TOKEN_AMOUNT] = token;
    pData->tokenAmount++;

    return 1;
}

CS64INIToken* cs64_ini_token_data_last_token(CS64INITokenData *pData) {
    if(pData->tokenAmount == 0)
        return NULL;

    return &pData->pLastPage->tokens[(pData->tokenAmount - 1) % CS64_INI_TOKEN_AMOUNT];
}

CS64INIToken* cs64_ini_token_data_get_token(CS64INITokenData *pData, CS64Size tokenIndex) {
    if(pData->tokenAmount <= tokenIndex)
        return NULL; /* Out of bounds! */

    CS64Size tokenArrayPageIndex = tokenIndex / CS64_INI_TOKEN_AMOUNT;
    CS64Size tokenPageIndex      = tokenIndex % CS64_INI_TOKEN_AMOUNT;

    if(tokenArrayPageIndex == 0)
        return &pData->firstPage.tokens[tokenPageIndex];

    CS64INITokenArrayList *pCurrentPage = &pData->firstPage;
    while(tokenArrayPageIndex != 0) {
        pCurrentPage = pCurrentPage->pNext;
        tokenArrayPageIndex--;
    }

    return &pCurrentPage->tokens[tokenPageIndex];
}

void cs64_ini_token_data_free(CS64INITokenData *pData) {
    CS64INITokenArrayList *pCurrentPage = pData->firstPage.pNext;
    CS64INITokenArrayList *pNextPage;

    while(pCurrentPage != NULL) {
        pNextPage = pCurrentPage->pNext;
        CS64_INI_FREE(pCurrentPage);
        pCurrentPage = pNextPage;
    }

    CS64_INI_FREE(pData);
}

/* ### Lexer */

int cs64_ini_is_character_used(CS64UniChar character) {
    /* WARNING Never remove this function! It can detect naming conflicts. */
    switch(character) {
        case CS64_INI_COMMENT:
        case CS64_INI_DELEMETER:
        case CS64_INI_END:
        case CS64_INI_SECTION_BEGIN:
        case CS64_INI_SECTION_END:
        case CS64_INI_VALUE_SLASH:
        case CS64_INI_VALUE_QUOTE:
            return 1;
        default:
            return 0;
    }
}

int cs64_ini_is_character_value(CS64UniChar character) {
    if(cs64_ini_is_character_used(character))
        return 0;
    else if(cs64_ini_is_character_whitespace(character))
        return 0;
    else if(character > 0x20 && character < 0x07f)
        return 1;
    else if(character > 0xa0 && character < 0x0ad)
        return 1;
    else if(character > 0xad && character < 0x100)
        return 1;
    return 0;
}

int cs64_ini_is_character_whitespace(CS64UniChar character) {
    /* Check for whitespace characters */
    switch(character) {
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
        case 0xFEFF:
            return 1; /* This is whitespace! */
        default:
            return 0; /* Not whitespace */
    }
}

#define INVALID_CHARACTER_TEST(pResult, ret)\
    if(characterSize == 0) {\
        (pResult)->state = CS64_INI_LEXER_ENCODING_ERROR;\
        (pResult)->status.encoding.badByteAmount = UTF8ByteSize - UTF8Offset;\
        \
        if((pResult)->status.encoding.badByteAmount > sizeof((pResult)->status.encoding.badBytes) / sizeof((pResult)->status.encoding.badBytes[0]))\
            (pResult)->status.encoding.badByteAmount = sizeof((pResult)->status.encoding.badBytes) / sizeof((pResult)->status.encoding.badBytes[0]);\
        \
        CS64Size count = 0;\
        while(count < (pResult)->status.encoding.badByteAmount) {\
            (pResult)->status.encoding.badBytes[count] = pUTF8Data[UTF8Offset + count];\
            count++;\
        }\
        return ret; /* NOTE: Invalid Character Error. */\
    }

CS64INIToken cs64_ini_tokenize_comment(CS64INITokenResult *pResult, const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize, CS64Size UTF8Offset) {
    CS64Size characterSize;
    CS64UniChar character;
    CS64INIToken token = {CS64_INI_TOKEN_COMMENT, UTF8Offset, 0};

    while(UTF8Offset < UTF8ByteSize) {
        character = cs64_ini_utf_8_read(&pUTF8Data[UTF8Offset], UTF8ByteSize - UTF8Offset, &characterSize);

        /* Check the characterSize to detect ASCII/UTF-8 error */
        INVALID_CHARACTER_TEST(pResult, token)

        if(character == ((CS64UniChar)'\n')) {
            token.byteLength = UTF8Offset - token.index;
            return token;
        }

        pResult->linePosition++;
        UTF8Offset += characterSize;
    }

    if(UTF8Offset == UTF8ByteSize)
        token.byteLength = UTF8Offset - token.index;

    return token;
}

CS64INIToken cs64_ini_tokenize_value(CS64INITokenResult *pResult, const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize, CS64Size UTF8Offset) {
    CS64Size characterSize;
    CS64UniChar character;
    CS64INIToken token = {CS64_INI_TOKEN_VALUE, UTF8Offset, 0};

    while(UTF8Offset < UTF8ByteSize) {
        character = cs64_ini_utf_8_read(&pUTF8Data[UTF8Offset], UTF8ByteSize - UTF8Offset, &characterSize);

        /* Check the characterSize to detect ASCII/UTF-8 error */
        INVALID_CHARACTER_TEST(pResult, token)

        if(!cs64_ini_is_character_value(character)) {
            token.byteLength = UTF8Offset - token.index;
            return token;
        }

        UTF8Offset += characterSize;
    }

    if(UTF8Offset == UTF8ByteSize)
        token.byteLength = UTF8Offset - token.index;

    return token;
}

CS64INIToken cs64_ini_tokenize_value_quote(CS64INITokenResult *pResult, const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize, CS64Size UTF8Offset) {
    CS64Size characterSize;
    CS64UniChar character;
    CS64INIToken token = {CS64_INI_TOKEN_VALUE, UTF8Offset, 0};

    const CS64UniChar quote = cs64_ini_utf_8_read(&pUTF8Data[UTF8Offset], UTF8ByteSize - UTF8Offset, &characterSize);
    character = quote;

    /* Before doing anything check the characterSize to detect ASCII/UTF-8 error */
    INVALID_CHARACTER_TEST(pResult, token)

    pResult->linePosition++;
    UTF8Offset += characterSize;

    int noSlash = 1;

    while(UTF8Offset < UTF8ByteSize) {
        character = cs64_ini_utf_8_read(&pUTF8Data[UTF8Offset], UTF8ByteSize - UTF8Offset, &characterSize);

        /* Check the characterSize to detect ASCII/UTF-8 error */
        INVALID_CHARACTER_TEST(pResult, token)

        UTF8Offset += characterSize;

        if(character == ((CS64UniChar)'\n')) {
            pResult->lineCount++;
            pResult->linePosition = 0;
        }
        else
            pResult->linePosition++;

        if(noSlash) {
            if(character == quote) {
                break;
            }
            else if(character == CS64_INI_VALUE_SLASH) {
                noSlash = 0;
            }
        }
        else {
            noSlash = 1;
        }
    }

    if(character != quote) {
        pResult->state = CS64_INI_LEXER_EXPECTED_ERROR;
        pResult->status.expected.expected = quote;
        pResult->status.expected.result = character;
        return token; /* NOTE: Expected Quote Error. */
    }

    token.byteLength = UTF8Offset - token.index;

    return token;
}

CS64INITokenResult cs64_ini_lexer(const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize) {
    CS64INITokenResult result;
    result.state = CS64_INI_LEXER_SUCCESS;
    result.lineCount = 0;
    result.linePosition = 0;
    result.pTokenStorage = cs64_ini_token_data_alloc();

    /* Memory safety! */
    if(result.pTokenStorage == NULL) {
        result.state = CS64_INI_LEXER_NO_MEMORY_ERROR;
        return result; /* NOTE: Generic out of memory exception. */
    }
    result.lineCount++;

    CS64Size UTF8Offset = 0;
    CS64Size characterSize;
    CS64UniChar character;
    CS64INIToken token;

    while(UTF8Offset < UTF8ByteSize) {
        character = cs64_ini_utf_8_read(&pUTF8Data[UTF8Offset], UTF8ByteSize - UTF8Offset, &characterSize);

        /* Before doing anything check the characterSize to detect ASCII/UTF-8 error */
        INVALID_CHARACTER_TEST(&result, result)

        if(character == CS64_INI_END) {
            token.type       = CS64_INI_TOKEN_END;
            token.index      = UTF8Offset;
            token.byteLength = characterSize;
        }
        else if(character == CS64_INI_SECTION_BEGIN) {
            token.type       = CS64_INI_TOKEN_SECTION_START;
            token.index      = UTF8Offset;
            token.byteLength = characterSize;
        }
        else if(character == CS64_INI_SECTION_END) {
            token.type       = CS64_INI_TOKEN_SECTION_END;
            token.index      = UTF8Offset;
            token.byteLength = characterSize;
        }
        else if(character == CS64_INI_COMMENT) {
            token = cs64_ini_tokenize_comment(&result, pUTF8Data, UTF8ByteSize, UTF8Offset);

            /* If byte length is zero then tokenization had failed. */
            if(token.byteLength == 0)
                break;

            UTF8Offset += token.byteLength;
            characterSize = 0; /* Do not advance the position so CS64_INI_TOKEN_END can properly be produced. */
        }
        else if(character == CS64_INI_DELEMETER) {
            token.type       = CS64_INI_TOKEN_DELEMETER;
            token.index      = UTF8Offset;
            token.byteLength = characterSize;
        }
        else if(character == CS64_INI_VALUE_QUOTE) {
            token = cs64_ini_tokenize_value_quote(&result, pUTF8Data, UTF8ByteSize, UTF8Offset);

            /* If byte length is zero then tokenization had failed. */
            if(token.byteLength == 0)
                break;

            UTF8Offset += token.byteLength;
            characterSize = 0; /* Do not advance the position so CS64_INI_TOKEN_END can properly be produced. */
        }
        else if(cs64_ini_is_character_whitespace(character)) { /* Skip whitespace. */
            UTF8Offset += characterSize;
            continue;
        }
        else if(cs64_ini_is_character_value(character)) {
            token = cs64_ini_tokenize_value(&result, pUTF8Data, UTF8ByteSize, UTF8Offset);

            /* If byte length is zero then tokenization had failed. */
            if(token.byteLength == 0)
                break;

            UTF8Offset += token.byteLength;
            characterSize = 0; /* Do not advance the position so CS64_INI_TOKEN_END can properly be produced. */
        }
        else {
            result.state = CS64_INI_LEXER_UNHANDLED_CHAR_ERROR;
            result.status.unhandled.unhandled = character;
            return result; /* NOTE: Valid ASCII or UTF-8, but unhandled character Error. */
        }

        if(!cs64_ini_token_data_append_token(result.pTokenStorage, token)) {
            result.state = CS64_INI_LEXER_NO_MEMORY_ERROR;
            return result; /* NOTE: Generic out of memory exception. The program probably somehow ran out of space! Error. */
        }

        if(character == ((CS64UniChar)'\n')) {
            result.lineCount++;
            result.linePosition = 0;
        }
        else
            result.linePosition++;

        UTF8Offset += characterSize;
    }

    return result;
}
#undef INVALID_CHARACTER_TEST

#endif /* CS64_INI_LIBRARY_IMP */
