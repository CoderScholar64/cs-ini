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

#ifndef CS64_INI_IMP_DETAIL_VALUE_SIZE
    #define CS64_INI_IMP_DETAIL_VALUE_SIZE 16
#endif

#ifndef CS64_INI_IMP_DETAIL_SECTION_NAME_SIZE
    #define CS64_INI_IMP_DETAIL_SECTION_NAME_SIZE 16
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

    #define CS64_SIZE_MAX SIZE_MAX
#elifndef CS64_SIZE_MAX
    #error CS64_SIZE_MAX Also needs to be defined!
#endif

#ifndef CS64Offset
    #define CS64Offset CS64Size
#endif

#ifndef CS64_INI_HASH_FUNCTION_NAME
    /* Indicate that the standard hash function is being used */
    #define CS64_INI_HASH_STANDARD_FUNCTION

    CS64Offset cs64_ini_standard_hash_function(const CS64UTF8 *const pString, CS64Offset hash, CS64Size *pStringByteSize);

    #define CS64_INI_HASH_FUNCTION(pString, hash, pStringByteSize) cs64_ini_standard_hash_function(pString, hash, pStringByteSize)

    #if CS64_SIZE_MAX > 0xFFFFFFFF
    #define CS64_INI_INITIAL_HASH 0xcbf29ce484222325
    #else
    #define CS64_INI_INITIAL_HASH 0x811c9dc5
    #endif
#else
    CS64Offset CS64_INI_HASH_FUNCTION_NAME(const CS64UTF8 *const pString, CS64Offset hash, CS64Size *pStringByteSize);

    #define CS64_INI_HASH_FUNCTION(pString, hash, pStringByteSize) CS64_INI_HASH_FUNCTION_NAME(pString, hash, pStringByteSize)

    #ifndef CS64_INI_INITIAL_HASH
    #error The CS64_INI_INITIAL_HASH define is required for custom hashing function.
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
    CS64_INI_MAX_CODE              = 0x10ffff,
    CS64_INI_MAX_CODE_AMOUNT       = 0x110000,
    CS64_INI_NOT_ASCII_ERROR       = 0x110001,
    CS64_INI_NOT_UTF_8_ERROR       = 0x110002,
    CS64_INI_OVERLONG_ERROR        = 0x110003,
    CS64_INI_LACK_SPACE_ERROR      = 0x110004,
    CS64_INI_TOO_BIG_ERROR         = 0x110005,
    CS64_INI_CONTINUE_BYTE_0_ERROR = 0x110006,
    CS64_INI_CONTINUE_BYTE_1_ERROR = 0x110007,
    CS64_INI_CONTINUE_BYTE_2_ERROR = 0x110008,
    CS64_INI_CONTINUE_BYTE_3_ERROR = 0x110009
} CS64INIUniCharState;

typedef enum {
    CS64_INI_LEXER_SUCCESS              = 0, /* Anything other than zero, is an error */
    CS64_INI_LEXER_EARLY_NULL_ERROR     = 1,
    CS64_INI_LEXER_NO_MEMORY_ERROR      = 2,
    CS64_INI_LEXER_ENCODING_ERROR       = 3,
    CS64_INI_LEXER_EXPECTED_ERROR       = 4,
    CS64_INI_LEXER_UNHANDLED_CHAR_ERROR = 5
} CS64INILexerState;

typedef enum {
    CS64_INI_PARSER_SUCCESS             = 0, /* Anything other than zero, is an error */
    CS64_INI_PARSER_UNEXPECTED_ERROR    = 1, /* This error states that it expected a token, but got something else instead. */
    CS64_INI_PARSER_REDECLARATION_ERROR = 2, /* Make sure that there is no naming conflicts. */
    CS64_INI_PARSER_INI_DATA_ERROR      = 3, /* This means that there is an error with the ini data functions used to construct the hash table. */
    CS64_INI_PARSER_LEXER_MEM_ERROR     = 4  /* The memory limit has been exceeded */
} CS64INIParserState;

typedef enum {
    CS64_INI_ENTRY_SUCCESS               = 0, /* Anything other than zero, is an error */
    CS64_INI_ENTRY_DATA_NULL_ERROR       = 1,
    CS64_INI_ENTRY_SECTION_EMPTY_ERROR   = 2,
    CS64_INI_ENTRY_VARIABLE_EMPTY_ERROR  = 3,
    CS64_INI_ENTRY_ENTRY_EMPTY_ERROR     = 4,
    CS64_INI_ENTRY_ENTRY_EXISTS_ERROR    = 5,
    CS64_INI_ENTRY_ENTRY_DNE_ERROR       = 6,
    CS64_INI_ENTRY_NO_MEMORY_ERROR       = 7,
    CS64_INI_ENTRY_ILLEGAL_STRING_ERROR  = 8,
    CS64_INI_ENTRY_INVALID_ENCODE_ERROR  = 9
} CS64INIEntryState;

typedef enum {
    /* CS64_INI_TOKEN_WHITE_SPACE would not be stored anyways. */
    CS64_INI_TOKEN_DELEMETER,     /* DELEMETER */
    CS64_INI_TOKEN_VALUE,         /* VALUE */
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
    /* This is for allocations for the hash tables */
    CS64Size delimeterCount; /* delimeters count the number of entries that would need to be counted */
    CS64Size sectionBeginCount; /* sectionBeginCount count the number of sections that would need to be stored. */
    CS64Size sectionEndCount; /* sectionEndCount count the number of sections that would need to be stored. It is here for extra error detection. */
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

typedef struct {
    CS64INIParserState state;

    union {
        struct {
            CS64INIToken receivedToken;
            CS64Size expectedTokenAmount;
            const CS64INITokenType* pExpectedTokens;
        } unexpected_token;
        struct {
            const CS64UTF8 *pFunctionName;
            CS64INIEntryState functionStatus;
        } data_error;
    } status;
} CS64INIParserResult;

struct CS64INIEntry;

typedef struct CS64Value {
    struct CS64INIEntry *pSection;
    CS64Size nameByteSize;
    CS64Size valueByteSize;
    /* This could be a union to hold integers/floats efficiently */
    union {
        CS64UTF8 fixed[CS64_INI_IMP_DETAIL_VALUE_SIZE];
        struct {
            CS64UTF8 *pName;
            CS64UTF8 *pValue;
        } dynamic;
    } data;
} CS64Value;

typedef struct CS64SectionHeader {
    struct CS64INIEntry *pFirstValue;  /* CS64_INI_ENTRY_VALUE or CS64_INI_ENTRY_DYNAMIC_VALUE */
    struct CS64INIEntry *pLastValue;
} CS64SectionHeader;

typedef struct CS64Section {
    CS64SectionHeader header;
    CS64Size nameByteSize;
    union {
        CS64UTF8 fixed[CS64_INI_IMP_DETAIL_SECTION_NAME_SIZE];
        CS64UTF8 *pDynamic;
    } name;
} CS64Section;

typedef enum {
    CS64_INI_ENTRY_EMPTY,
    CS64_INI_ENTRY_WAS_OCCUPIED,
    CS64_INI_ENTRY_SECTION,
    CS64_INI_ENTRY_DYNAMIC_SECTION,
    CS64_INI_ENTRY_VALUE,
    CS64_INI_ENTRY_DYNAMIC_VALUE
} CS64EntryType;

typedef struct CS64INIEntry {
    CS64EntryType entryType;
    struct CS64INIEntry *pNext;
    struct CS64INIEntry *pPrev;

    CS64Size commentSize;
    CS64UTF8 *pComment;
    CS64Size inlineCommentSize;
    CS64UTF8 *pInlineComment;
    union {
        CS64Value   value;
        CS64Section section;
    } type;
} CS64INIEntry;

typedef struct {
    CS64INIEntry *pEntries;
    CS64Size currentEntryAmount;
    CS64Size entryCapacity;
    CS64Size entryCapacityUpLimit;
    CS64Size entryCapacityDownLimit;
} CS64INIHashTable;

typedef struct {
    CS64INIHashTable hashTable;

    CS64Size lastCommentSize;
    CS64UTF8 *pLastComment;

    /* Useful for exporting in order. */
    CS64SectionHeader globals;
    CS64INIEntry *pFirstSection;
    CS64INIEntry  *pLastSection;
} CS64INIData;

/* This is mostly for internal use only. */
typedef struct {
    /* Nonconstants */
    CS64UTF8 *pStringBuffer; /* Initially, with a byte size of 512. */
    CS64UTF8 *pValueBuffer; /* Initially, with a byte size of 512. */
    CS64Size stringBufferLimit;
    CS64INIData *pData; /* Initially empty. */
    CS64Size tokenOffset; /* Initially zero. */
    CS64INIEntry *pSection; /* Initially NULL. Which means "global" section. */

    /* Constants */
    const CS64UTF8 *pSource;
    CS64INITokenResult *pTokenResult;
} CS64INIParserContext;

/* Public functions */

CS64INIData* cs64_ini_data_alloc();
/*TODO Add save and load functions*/
int cs64_ini_data_reserve(CS64INIData* pData, CS64Size numberOfSectionsAndValues);
void cs64_ini_data_free(CS64INIData* pData);

CS64INIEntryState cs64_ini_add_variable(CS64INIData *pData, const CS64UTF8 *const pSection, const CS64UTF8 *const pName, const CS64UTF8 *pValue, CS64INIEntry** ppEntry);
CS64INIEntryState cs64_ini_add_section(CS64INIData *pData, const CS64UTF8 *const pSection, CS64INIEntry** ppEntry);
CS64INIEntry* cs64_ini_get_section(CS64INIData *pData, const CS64UTF8 *const pSection);
CS64INIEntry* cs64_ini_get_variable(CS64INIData *pData, const CS64UTF8 *const pSection, const CS64UTF8 *const pName);
CS64INIEntryState cs64_ini_del_entry(CS64INIData *pData, CS64INIEntry *pEntry);

CS64EntryType cs64_ini_get_entry_type(const CS64INIEntry *const pEntry);

CS64INIEntry* cs64_ini_get_first_section(CS64INIData *pData);
CS64INIEntry* cs64_ini_get_first_global_value(CS64INIData *pData);
CS64INIEntry* cs64_ini_get_first_section_value(CS64INIEntry *pEntry);

CS64INIEntry* cs64_ini_get_next_entry(CS64INIEntry *pEntry);
CS64INIEntry* cs64_ini_get_prev_entry(CS64INIEntry *pEntry);

CS64INIEntry* cs64_ini_get_entry_section(const CS64INIEntry *const pEntry);
const CS64UTF8 *const cs64_ini_get_entry_section_name(const CS64INIEntry *const pEntry);

CS64INIEntryState cs64_ini_set_entry_name(CS64INIData *pData, const CS64UTF8 *const pValue, CS64INIEntry **ppEntry);
const CS64UTF8 *const cs64_ini_get_entry_name(const CS64INIEntry *const pEntry);

CS64INIEntryState cs64_ini_set_entry_value(CS64INIEntry *pEntry, const CS64UTF8 *const pValue);
const CS64UTF8 *const cs64_ini_get_entry_value(const CS64INIEntry *const pEntry);

CS64INIEntryState cs64_ini_set_entry_comment(CS64INIEntry *pEntry, const CS64UTF8 *const pValue);
const CS64UTF8 *const cs64_ini_get_entry_comment(const CS64INIEntry *const pEntry);

CS64INIEntryState cs64_ini_set_entry_inline_comment(CS64INIEntry *pEntry, const CS64UTF8 *const pValue);
const CS64UTF8 *const cs64_ini_get_entry_inline_comment(const CS64INIEntry *const pEntry);

CS64INIEntryState cs64_ini_set_last_comment(CS64INIData *pData, const CS64UTF8 *const pValue);
const CS64UTF8 *const cs64_ini_get_last_comment(CS64INIData *pData);

/* Private functions */

CS64INITokenData* cs64_ini_token_data_alloc();
int cs64_ini_token_data_append_token(CS64INITokenData *pData, CS64INIToken token);
CS64INIToken* cs64_ini_token_data_last_token(CS64INITokenData *pData);
CS64INIToken* cs64_ini_token_data_get_token(CS64INITokenData *pData, CS64Size tokenIndex);
void cs64_ini_token_data_free(CS64INITokenData *pData);

int cs64_ini_is_character_used(CS64UniChar character);
int cs64_ini_is_character_value(CS64UniChar character);
int cs64_ini_is_character_whitespace(CS64UniChar character);
CS64INIToken cs64_ini_tokenize_comment(CS64INITokenResult *pResult, const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize, CS64Size UTF8Offset);
CS64INIToken cs64_ini_tokenize_value(CS64INITokenResult *pResult, const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize, CS64Size UTF8Offset);
CS64INIToken cs64_ini_tokenize_value_quote(CS64INITokenResult *pResult, const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize, CS64Size UTF8Offset);
CS64INITokenResult cs64_ini_lexer(const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize);
void cs64_ini_lexer_free(CS64INITokenResult *pResult);

CS64INIParserResult cs64_ini_parse_line(CS64INIParserContext *pParserContext);

/**
 * This function reads an ASCII value.
 * @warning UTF-8 characters need to stay within the ASCII range.
 * @param pDataHead Pointer to the current byte in the UTF-8 stream. @warning NULL is not optional. This value must be present.
 * @param remainingDataSize The space left to read. Used to prevent buffer overflows. @warning make sure you subtract this value using *pCharacterByteSize to successfully test agaisnt buffer-overflows.
 * @param pCharacterByteSize Pointer to a variable storing the read character size. If zero then either NULL has been found or error. (updated by the function)
 * @return 0-127 if there is no error. CS64_INI_NOT_ASCII_ERROR is the stream is not ASCII.
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
        return CS64_INI_LACK_SPACE_ERROR;
    }
    else if(*pDataHead >= 0x80) {
        *pCharacterByteSize = 0; /* Indicate that the loop calling this function should end. */
        return CS64_INI_NOT_ASCII_ERROR;
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
        return CS64_INI_LACK_SPACE_ERROR;
    else if(CHECK_NEVER_BYTE(pDataHead[0]))
        return CS64_INI_NOT_UTF_8_ERROR;
    else if(IS_CHAR_CONTINUATION(pDataHead[0]))
        return CS64_INI_CONTINUE_BYTE_0_ERROR;
    else if(IS_UTF_1_BYTE(pDataHead[0])) {
        *pCharacterByteSize = 1;
        return pDataHead[0];
    }
    else if(IS_UTF_2_BYTE(pDataHead[0])) {
        if(remainingDataSize < 2)
            return CS64_INI_LACK_SPACE_ERROR;
        else if(!IS_CHAR_CONTINUATION(pDataHead[1])) /* Check if byte is continuous. */
            return CS64_INI_CONTINUE_BYTE_1_ERROR;

        result  = ((pDataHead[0] & 0b00011111) << 6);
        result |= ((pDataHead[1] & 0b00111111) << 0);

        *pCharacterByteSize = 2;

        return result;
    }
    else if(IS_UTF_3_BYTE(pDataHead[0])) {
        if(remainingDataSize < 3)
            return CS64_INI_LACK_SPACE_ERROR;
        else if(!IS_CHAR_CONTINUATION(pDataHead[1])) /* Check if byte is continuous. */
            return CS64_INI_CONTINUE_BYTE_1_ERROR;
        else if(!IS_CHAR_CONTINUATION(pDataHead[2])) /* Check if byte is continuous. */
            return CS64_INI_CONTINUE_BYTE_2_ERROR;
        else if(IS_UTF_OVERLONG_3_BYTE(pDataHead[0], pDataHead[1])) /* Check for overlong error. */
            return CS64_INI_OVERLONG_ERROR;

        result  = ((pDataHead[0] & 0b00001111) << 12);
        result |= ((pDataHead[1] & 0b00111111) <<  6);
        result |= ((pDataHead[2] & 0b00111111) <<  0);

        *pCharacterByteSize = 3;

        return result;
    }
    else if(IS_UTF_4_BYTE(pDataHead[0])) {
        if(remainingDataSize < 4)
            return CS64_INI_LACK_SPACE_ERROR;
        else if(!IS_CHAR_CONTINUATION(pDataHead[1])) /* Check if byte is continuous. */
            return CS64_INI_CONTINUE_BYTE_1_ERROR;
        else if(!IS_CHAR_CONTINUATION(pDataHead[2])) /* Check if byte is continuous. */
            return CS64_INI_CONTINUE_BYTE_2_ERROR;
        else if(!IS_CHAR_CONTINUATION(pDataHead[3])) /* Check if byte is continuous. */
            return CS64_INI_CONTINUE_BYTE_3_ERROR;
        else if(IS_UTF_OVERLONG_4_BYTE(pDataHead[0], pDataHead[1])) /* Check for overlong error. */
            return CS64_INI_OVERLONG_ERROR;
        else if(IS_UTF_OVERSIZED(pDataHead[0], pDataHead[1])) /* Check for over-size case. */
            return CS64_INI_TOO_BIG_ERROR;

        result  = ((pDataHead[0] & 0b00000111) << 18);
        result |= ((pDataHead[1] & 0b00111111) << 12);
        result |= ((pDataHead[2] & 0b00111111) <<  6);
        result |= ((pDataHead[3] & 0b00111111) <<  0);

        *pCharacterByteSize = 4;

        return result;
    }

    return CS64_INI_NOT_UTF_8_ERROR;
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

/* ### Token Storage ### */

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

/* ### Lexer ### */

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
        case 0xfeff:
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
    }\
    else if(pUTF8Data[UTF8Offset] == '\0') {\
        (pResult)->state = CS64_INI_LEXER_EARLY_NULL_ERROR;\
        return ret;\
    }

#define ADVANCE_CHARACTER(pResult)\
    UTF8Offset += characterSize;\
\
    if(character == ((CS64UniChar)'\n')) {\
        (pResult)->lineCount++;\
        (pResult)->linePosition = 0;\
    }\
    else\
        (pResult)->linePosition++;

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

        ADVANCE_CHARACTER(pResult)
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

        ADVANCE_CHARACTER(pResult)

        if(noSlash) {
            if(character == quote) {
                token.byteLength = UTF8Offset - token.index;
                return token;
            }
            else if(character == CS64_INI_VALUE_SLASH) {
                noSlash = 0;
            }
        }
        else {
            noSlash = 1;
        }
    }

    pResult->state = CS64_INI_LEXER_EXPECTED_ERROR;
    pResult->status.expected.expected = quote;
    pResult->status.expected.result = character;
    return token; /* NOTE: Expected Quote Error. */
}

#define SET_TOKEN(t)\
    token.type       = t;\
    token.index      = UTF8Offset;\
    token.byteLength = characterSize;

#define CALL_TOKEN_FUNCTION(fun)\
    token = fun(&result, pUTF8Data, UTF8ByteSize, UTF8Offset);\
\
    /* If byte length is zero then tokenization had failed. */\
    if(token.byteLength == 0)\
        break;\
\
    UTF8Offset += token.byteLength;\
    characterSize = 0; /* Do not advance the position so CS64_INI_TOKEN_END can properly be produced. */\
    result.linePosition--;

#define CALC_UTF_8_BYTE_SIZE(X)\
    CS64Size X ## _BYTE_SIZE;\
    if(X < 0x80) /* ASCII range. UTF-8 1 Byte Case. */\
        X ## _BYTE_SIZE = 1;\
    else if(X < 0x800) /* UTF-8 2 Byte Case. */\
        X ## _BYTE_SIZE = 2;\
    else if(X < 0x10000) /* UTF-8 3 Byte Case. */\
        X ## _BYTE_SIZE = 3;\
    else /* UTF-8 4 Byte Case. */\
        X ## _BYTE_SIZE = 4;\

CS64INITokenResult cs64_ini_lexer(const CS64UTF8 *const pUTF8Data, CS64Size UTF8ByteSize) {
    CS64INITokenResult result;
    result.state = CS64_INI_LEXER_SUCCESS;
    result.lineCount = 0;
    result.linePosition = 0;
    result.delimeterCount = 0;
    result.sectionBeginCount = 0;
    result.sectionEndCount = 0;
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

    CALC_UTF_8_BYTE_SIZE(CS64_INI_COMMENT);
    CALC_UTF_8_BYTE_SIZE(CS64_INI_VALUE_QUOTE);

    while(UTF8Offset < UTF8ByteSize) {
        character = cs64_ini_utf_8_read(&pUTF8Data[UTF8Offset], UTF8ByteSize - UTF8Offset, &characterSize);

        /* Before doing anything check the characterSize to detect ASCII/UTF-8 error */
        INVALID_CHARACTER_TEST(&result, result)

        if(character == CS64_INI_END) {
            SET_TOKEN(CS64_INI_TOKEN_END)
        }
        else if(character == CS64_INI_SECTION_BEGIN) {
            SET_TOKEN(CS64_INI_TOKEN_SECTION_START)
            result.sectionBeginCount++;
        }
        else if(character == CS64_INI_SECTION_END) {
            SET_TOKEN(CS64_INI_TOKEN_SECTION_END)
            result.sectionEndCount++;
        }
        else if(character == CS64_INI_COMMENT) {
            CALL_TOKEN_FUNCTION(cs64_ini_tokenize_comment)

            if(token.byteLength != 0) {
                token.index      += CS64_INI_COMMENT_BYTE_SIZE;
                token.byteLength -= CS64_INI_COMMENT_BYTE_SIZE;
            }
        }
        else if(character == CS64_INI_DELEMETER) {
            SET_TOKEN(CS64_INI_TOKEN_DELEMETER)
            result.delimeterCount++;
        }
        else if(character == CS64_INI_VALUE_QUOTE) {
            CALL_TOKEN_FUNCTION(cs64_ini_tokenize_value_quote)

            if(token.byteLength != 0) {
                token.index      += CS64_INI_VALUE_QUOTE_BYTE_SIZE;
                token.byteLength -= CS64_INI_VALUE_QUOTE_BYTE_SIZE;
            }

            if(token.byteLength != 0) {
                token.byteLength -= CS64_INI_VALUE_QUOTE_BYTE_SIZE;
            }
        }
        else if(cs64_ini_is_character_whitespace(character)) { /* Skip whitespace. */
            ADVANCE_CHARACTER(&result)
            continue;
        }
        else if(cs64_ini_is_character_value(character)) {
            CALL_TOKEN_FUNCTION(cs64_ini_tokenize_value)
        }
        else {
            result.state = CS64_INI_LEXER_UNHANDLED_CHAR_ERROR;
            result.status.unhandled.unhandled = character;
            return result; /* NOTE: Valid ASCII or UTF-8, but unhandled character Error. */
        }

        CS64INIToken *pToken = cs64_ini_token_data_last_token(result.pTokenStorage);

        if(pToken != NULL && token.type == pToken->type && (pToken->type == CS64_INI_TOKEN_END)) {
            pToken->byteLength = (token.index + token.byteLength) - pToken->index;
        }
        else if(!cs64_ini_token_data_append_token(result.pTokenStorage, token)) {
            result.state = CS64_INI_LEXER_NO_MEMORY_ERROR;
            return result; /* NOTE: Generic out of memory exception. The program probably somehow ran out of space! Error. */
        }

        ADVANCE_CHARACTER(&result)
    }

    /* Add an end */
    CS64INIToken *pToken = cs64_ini_token_data_last_token(result.pTokenStorage);
    if(pToken == NULL || pToken->type != CS64_INI_TOKEN_END) {
        token.type       = CS64_INI_TOKEN_END;
        token.index      = UTF8Offset;
        token.byteLength = 0;

        if(!cs64_ini_token_data_append_token(result.pTokenStorage, token)) {
            result.state = CS64_INI_LEXER_NO_MEMORY_ERROR;
            return result; /* NOTE: Generic out of memory exception. The program probably somehow ran out of space! Error. */
        }
    }

    return result;
}
#undef ADVANCE_CHARACTER
#undef CALL_TOKEN_FUNCTION
#undef SET_TOKEN
#undef INVALID_CHARACTER_TEST

void cs64_ini_lexer_free(CS64INITokenResult *pData) {
    if(pData->pTokenStorage != NULL)
        cs64_ini_token_data_free(pData->pTokenStorage);
}

/* ### Hash Table ### */

#ifdef CS64_INI_HASH_STANDARD_FUNCTION

CS64Offset cs64_ini_standard_hash_function(const CS64UTF8 *const pString, CS64Offset hash, CS64Size *pStringByteSize) {
    /* This implementation uses the Fowler–Noll–Vo hash algorithm. It is NOT cyroptographically secure, but this is just an INI file parser. */

    #if CS64_SIZE_MAX > 0xFFFFFFFF
    const static CS64Offset prime = 0x00000100000001b3;
    #else
    const static CS64Offset prime = 0x01000193;
    #endif

    while(pString[*pStringByteSize] != '\0') {
        hash ^= pString[*pStringByteSize];
        hash *= prime;
        (*pStringByteSize)++;
    }

    (*pStringByteSize)++;

    return hash;
}

#endif

static int cs64_ini_are_strings_equal(const CS64UTF8 *const x, const CS64UTF8 *const y) {
    CS64Size length = 0;

    while(x[length] == y[length] && x[length] != '\0' && y[length] != '\0') {
        length++;
    }

    if(x[length] != y[length])
        return 0; /* False */
    return 1; /* True */
}

static CS64Size cs64_ini_string_byte_size(const CS64UTF8 *const str) {
    CS64Size length = 0;

    while(str[length] != '\0') {
        length++;
    }
    length++;

    return length;
}

#define INITIAL_CAPACITY 16
#define P2_LIMIT 512
#define CALC_UPPER_LIMIT(x) ((x) / 2 + (x) / 4 + (x) / 16)
#define CALC_P2_LOWER_LIMIT(x) CALC_UPPER_LIMIT((x) / 2)
#define CALC_LR_LOWER_LIMIT(x) CALC_UPPER_LIMIT((x) - P2_LIMIT)
#define CALC_LOWER_LIMIT(x)\
if((x.entryCapacity) <= INITIAL_CAPACITY)\
    x.entryCapacityDownLimit = 0; /* This effectively prevents resizing. INITIAL_CAPACITY is also the minimum capacity. */\
else if(x.entryCapacity > P2_LIMIT)\
    x.entryCapacityDownLimit = CALC_LR_LOWER_LIMIT(x.entryCapacity);\
else\
    x.entryCapacityDownLimit = CALC_P2_LOWER_LIMIT(x.entryCapacity);
#define INIT_HASH_TABLE(hashTable, entryAmount, capacity, onFailureRoutine)\
    hashTable.currentEntryAmount = entryAmount;\
    hashTable.entryCapacity = capacity;\
    hashTable.entryCapacityUpLimit = CALC_UPPER_LIMIT(hashTable.entryCapacity);\
    CALC_LOWER_LIMIT(hashTable)\
    hashTable.pEntries = CS64_INI_MALLOC(hashTable.entryCapacity * sizeof(CS64INIEntry));\
\
    if(hashTable.pEntries == NULL) onFailureRoutine\
\
    {CS64Offset entryIndex = 0;\
    while(entryIndex < hashTable.entryCapacity) {\
        hashTable.pEntries[entryIndex].entryType = CS64_INI_ENTRY_EMPTY;\
\
        hashTable.pEntries[entryIndex].pComment = NULL;\
        hashTable.pEntries[entryIndex].pInlineComment = NULL;\
\
        entryIndex++;\
    }}
#define IS_STRING_PRESENT(x) (x != NULL && x[0] != '\0')
#define IS_ENTRY_EMPTY(x) ((x)->entryType == CS64_INI_ENTRY_EMPTY || (x)->entryType == CS64_INI_ENTRY_WAS_OCCUPIED)
#define STRING_COPY(dst, src) {\
    CS64Size length = 0;\
    while(src[length] != '\0') {\
        dst[length] = src[length];\
        length++;\
    }\
    dst[length] = '\0';\
    length++;\
}
#define IS_ENTRY_SECTION(x) ((x)->entryType == CS64_INI_ENTRY_SECTION || (x)->entryType == CS64_INI_ENTRY_DYNAMIC_SECTION)
#define IS_SAME_SECTION_ENTRY(x, pSectionName, sectionByteSize) (IS_ENTRY_SECTION(x) &&\
    ((x)->type.section.nameByteSize == sectionByteSize)                              &&\
    (((x)->entryType == CS64_INI_ENTRY_SECTION        && cs64_ini_are_strings_equal((x)->type.section.name.fixed, pSectionName)) ||\
    ((x)->entryType == CS64_INI_ENTRY_DYNAMIC_SECTION && cs64_ini_are_strings_equal((x)->type.section.name.pDynamic, pSectionName))))
#define ATTEMPT_TO_FIND_ENTRY(x, pSectionName, index, originalIndex, srcHashTable, checker, findConditionStatement, notFoundCondition)\
    if(checker) {\
        findConditionStatement\
\
        index = (1 + index) % srcHashTable.entryCapacity;\
        x = &srcHashTable.pEntries[index];\
\
        while(index != originalIndex && checker) {\
            findConditionStatement\
\
            index = (1 + index) % srcHashTable.entryCapacity;\
            x = &srcHashTable.pEntries[index];\
        }\
\
        if(index == originalIndex) {\
            notFoundCondition;\
        }\
    }
#define ATTEMPT_TO_FIND_SECTION(x, pSectionName, sectionByteSize, index, originalIndex, srcHashTable, checker, findCondition, notFoundCondition)\
    ATTEMPT_TO_FIND_ENTRY(x, pSectionName, index, originalIndex, srcHashTable, checker, if(IS_SAME_SECTION_ENTRY(x, pSectionName, sectionByteSize)) {findCondition;}, notFoundCondition)
#define IS_ENTRY_VALUE(x) ((x)->entryType == CS64_INI_ENTRY_VALUE || (x)->entryType == CS64_INI_ENTRY_DYNAMIC_VALUE)
#define IS_CORRECT_VARIABLE(x, pSectionName, sectionByteSize, pVariableName, findCondition)\
    if(IS_ENTRY_VALUE(x)) {\
        if(sectionByteSize > 1 && pSectionName != NULL) {\
            if((x)->type.value.pSection != NULL && IS_SAME_SECTION_ENTRY((x)->type.value.pSection, pSectionName, sectionByteSize)) {\
                if((x)->entryType == CS64_INI_ENTRY_VALUE) {\
                    if(cs64_ini_are_strings_equal((x)->type.value.data.fixed, pVariableName)) {\
                        findCondition\
                    }\
                } else if((x)->entryType == CS64_INI_ENTRY_DYNAMIC_VALUE) {\
                    if(cs64_ini_are_strings_equal((x)->type.value.data.dynamic.pName, pVariableName)) {\
                        findCondition\
                    }\
                }\
            }\
        }\
        else {\
            if((x)->entryType == CS64_INI_ENTRY_VALUE) {\
                if(cs64_ini_are_strings_equal((x)->type.value.data.fixed, pVariableName)) {\
                    findCondition\
                }\
            } else if((x)->entryType == CS64_INI_ENTRY_DYNAMIC_VALUE) {\
                if(cs64_ini_are_strings_equal((x)->type.value.data.dynamic.pName, pVariableName)) {\
                    findCondition\
                }\
            }\
        }\
    }
#define ATTEMPT_TO_FIND_VARIABLE(x, pSectionName, sectionByteSize, pVariableName, index, originalIndex, srcHashTable, checker, findCondition, notFoundCondition)\
    ATTEMPT_TO_FIND_ENTRY(x, pVariableName, index, originalIndex, srcHashTable, checker, IS_CORRECT_VARIABLE(x, pSectionName, sectionByteSize, pVariableName, findCondition), notFoundCondition)
#define UTF8_CHECK(x)\
    if(x != NULL) {\
        CS64Size c = 0;\
        CS64Size l = cs64_ini_string_byte_size(x);\
        CS64Size charByteSize;\
\
        while(c < l) {\
            if(cs64_ini_utf_8_read(&(x)[c], l - c, &charByteSize) > CS64_INI_MAX_CODE)\
                return CS64_INI_ENTRY_INVALID_ENCODE_ERROR;\
            c += charByteSize;\
        }\
    }

CS64INIData* cs64_ini_data_alloc() {
    CS64INIData *pData = CS64_INI_MALLOC(sizeof(CS64INIData));

    if(pData == NULL) {
        return NULL; /* Malloc had failed in this case. */
    }

    INIT_HASH_TABLE(pData->hashTable, 0, INITIAL_CAPACITY, {CS64_INI_FREE(pData); return NULL;})

    pData->lastCommentSize = 0;
    pData->pLastComment    = NULL;

    pData->pFirstSection = NULL;
    pData->pLastSection  = NULL;
    pData->globals.pFirstValue = NULL;
    pData->globals.pLastValue  = NULL;

    return pData;
}

int cs64_ini_data_reserve(CS64INIData* pData, CS64Size numberOfSectionsAndValues) {
    /* pData is the INI file. */
    if(pData == NULL)
        return -1;

    /* pData make sure that the hash table has entries. */
    if(pData->hashTable.pEntries == NULL)
        return -2;

    /* This ensures that there will not be a crash to be made. */
    if(pData->hashTable.currentEntryAmount != 0 && numberOfSectionsAndValues < pData->hashTable.currentEntryAmount)
        return -3;

    /* Initialize a new hash table */
    CS64INIData newINIData = *pData;
    newINIData.globals.pFirstValue = NULL;
    newINIData.globals.pLastValue  = NULL;
    newINIData.pFirstSection = NULL;
    newINIData.pLastSection  = NULL;

    CS64Size newCapacity = INITIAL_CAPACITY;

    if(numberOfSectionsAndValues > INITIAL_CAPACITY) {
        if(CALC_UPPER_LIMIT(P2_LIMIT) <= numberOfSectionsAndValues) {
            CS64Size count = numberOfSectionsAndValues / CALC_UPPER_LIMIT(P2_LIMIT);
            if(numberOfSectionsAndValues % CALC_UPPER_LIMIT(P2_LIMIT) != 0)
                count++;

            newCapacity = P2_LIMIT * count;
        }
        else {
            CS64Size shift = 0;
            while((CALC_UPPER_LIMIT(INITIAL_CAPACITY) << shift) < numberOfSectionsAndValues) {
                shift++;
            }

            newCapacity = INITIAL_CAPACITY << shift;
        }
    }

    INIT_HASH_TABLE(newINIData.hashTable, pData->hashTable.currentEntryAmount, newCapacity, {return -4;})

    CS64Offset originalIndex;
    CS64Offset index;
    CS64Offset sectionHash;
    CS64Offset variableHash;
    CS64INIEntry *pEntry;
    CS64INIEntry *pSectionEntry;
    CS64Size nameByteSize;

    const CS64INIEntry *pVariable = pData->globals.pFirstValue;
    while(pVariable != NULL) {
        const CS64UTF8 *pName;

        if(pVariable->entryType == CS64_INI_ENTRY_VALUE)
            pName = &pVariable->type.value.data.fixed[0];
        else
            pName = pVariable->type.value.data.dynamic.pName;

        nameByteSize = 0;

        variableHash = CS64_INI_HASH_FUNCTION(pName, CS64_INI_INITIAL_HASH, &nameByteSize);

        originalIndex = variableHash % newINIData.hashTable.entryCapacity;

        index = originalIndex;

        pEntry = &newINIData.hashTable.pEntries[index];

        ATTEMPT_TO_FIND_VARIABLE(
            pEntry, NULL, 0, pName, index, originalIndex, newINIData.hashTable, !IS_ENTRY_EMPTY(pEntry),
            {CS64_INI_FREE(newINIData.hashTable.pEntries); return -5;},
            {CS64_INI_FREE(newINIData.hashTable.pEntries); return -6;})

        /* Copy source variable over to new table! */
        *pEntry = *pVariable;

        pEntry->pNext = NULL;
        pEntry->pPrev = newINIData.globals.pLastValue;

        if(pData->globals.pLastValue != NULL && newINIData.globals.pLastValue) {
            newINIData.globals.pLastValue->pNext = pEntry;
            pEntry->pPrev = newINIData.globals.pLastValue;
        }

        if(pData->globals.pFirstValue == pVariable)
            newINIData.globals.pFirstValue = pEntry;

        newINIData.globals.pLastValue = pEntry;

        pVariable = pVariable->pNext;
    }

    /* Write down the sections first */
    const CS64INIEntry *pSection = pData->pFirstSection;
    CS64Size sectionByteSize;
    while(pSection != NULL) {
        const CS64UTF8 *pSectionName;

        if(pSection->entryType == CS64_INI_ENTRY_SECTION)
            pSectionName = pSection->type.section.name.fixed;
        else
            pSectionName = pSection->type.section.name.pDynamic;

        sectionByteSize = 0;
        sectionHash = CS64_INI_HASH_FUNCTION(pSectionName, CS64_INI_INITIAL_HASH, &sectionByteSize);

        originalIndex = sectionHash % newINIData.hashTable.entryCapacity;

        index = originalIndex;

        pSectionEntry = &newINIData.hashTable.pEntries[index];

        ATTEMPT_TO_FIND_SECTION(pSectionEntry, pSectionName, sectionByteSize, index, originalIndex, newINIData.hashTable, !IS_ENTRY_EMPTY(pSectionEntry),
            {CS64_INI_FREE(newINIData.hashTable.pEntries); return -7;},
            {CS64_INI_FREE(newINIData.hashTable.pEntries); return -8;})

        /* Copy source section over to new table! */
        *pSectionEntry = *pSection;

        pSectionEntry->pPrev = newINIData.pLastSection;

        if(newINIData.pLastSection != NULL)
            newINIData.pLastSection->pNext = pSectionEntry;

        if(pData->pFirstSection == pSection)
            newINIData.pFirstSection = pSectionEntry;

        newINIData.pLastSection = pSectionEntry;

        pSectionEntry->type.section.header.pFirstValue = NULL;
        pSectionEntry->type.section.header.pLastValue  = NULL;

        pSection = pSection->pNext;
    }

    /* Lastly write down the section values */
    pSection      = pData->pFirstSection;
    pSectionEntry = newINIData.pFirstSection;
    while(pSection != NULL && pSectionEntry != NULL) {
        const CS64UTF8 *pSectionName;

        if(pSection->entryType == CS64_INI_ENTRY_SECTION)
            pSectionName = pSection->type.section.name.fixed;
        else
            pSectionName = pSection->type.section.name.pDynamic;

        sectionByteSize = 0;
        sectionHash = CS64_INI_HASH_FUNCTION(pSectionName, CS64_INI_INITIAL_HASH, &sectionByteSize);

        pVariable = pSection->type.section.header.pFirstValue;
        CS64INIEntry *pLastEntry = NULL;
        while(pVariable != NULL) {
            const CS64UTF8 *pName;

            if(pVariable->entryType == CS64_INI_ENTRY_VALUE)
                pName = &pVariable->type.value.data.fixed[0];
            else
                pName = pVariable->type.value.data.dynamic.pName;

            nameByteSize = 0;

            variableHash = CS64_INI_HASH_FUNCTION(pName, sectionHash, &nameByteSize);

            originalIndex = variableHash % newINIData.hashTable.entryCapacity;

            index = originalIndex;

            pEntry = &newINIData.hashTable.pEntries[index];

            ATTEMPT_TO_FIND_VARIABLE(
                pEntry, pSectionName, sectionByteSize, pName, index, originalIndex, newINIData.hashTable, !IS_ENTRY_EMPTY(pEntry),
                {CS64_INI_FREE(newINIData.hashTable.pEntries); return -9;},
                {CS64_INI_FREE(newINIData.hashTable.pEntries); return -10;})

            *pEntry = *pVariable;

            pEntry->pPrev = pLastEntry;

            if(pEntry->pPrev != NULL) {
                pLastEntry->pNext = pEntry;
            }

            pEntry->type.value.pSection = pSectionEntry;

            if( pSectionEntry->type.section.header.pFirstValue == NULL)
                pSectionEntry->type.section.header.pFirstValue = pEntry;

            pSectionEntry->type.section.header.pLastValue = pEntry;

            pLastEntry = pEntry;
            pVariable = pVariable->pNext;
        }

        pSection = pSection->pNext;
        pSectionEntry = pSectionEntry->pNext;
    }

    /* This hash table is no longer needed */
    CS64_INI_FREE(pData->hashTable.pEntries);

    /* Set the variables of the INI library */
    *pData = newINIData;

    return 0;
}

void cs64_ini_data_free(CS64INIData* pData) {
    if(pData == NULL)
        return;

    if(pData->hashTable.pEntries != NULL) {

        CS64Offset entryIndex = 0;
        while(entryIndex < pData->hashTable.entryCapacity) {
            if(pData->hashTable.pEntries[entryIndex].pComment != NULL)
                CS64_INI_FREE(pData->hashTable.pEntries[entryIndex].pComment);

            if(pData->hashTable.pEntries[entryIndex].pInlineComment != NULL)
                CS64_INI_FREE(pData->hashTable.pEntries[entryIndex].pInlineComment);

            switch(pData->hashTable.pEntries[entryIndex].entryType) {
                case CS64_INI_ENTRY_DYNAMIC_VALUE:
                    CS64_INI_FREE(pData->hashTable.pEntries[entryIndex].type.value.data.dynamic.pName); /* This also frees pValue */
                    break;
                case CS64_INI_ENTRY_DYNAMIC_SECTION:
                    CS64_INI_FREE(pData->hashTable.pEntries[entryIndex].type.section.name.pDynamic);
                    break;
                default:
            }

            entryIndex++;
        }

        CS64_INI_FREE(pData->hashTable.pEntries);
    }

    /* Free Comment */
    if(pData->pLastComment != NULL)
        CS64_INI_FREE(pData->pLastComment);

    CS64_INI_FREE(pData);
}

CS64INIEntryState cs64_ini_add_variable(CS64INIData *pData, const CS64UTF8 *const pSectionName, const CS64UTF8 *const pVariableName, const CS64UTF8 *pValue, CS64INIEntry** ppEntry) {
    /* If the programmer gives ppEntry an address */
    if(ppEntry != NULL)
        *ppEntry = NULL;

    UTF8_CHECK(pSectionName);
    UTF8_CHECK(pVariableName);
    UTF8_CHECK(pValue);

    /* Data must be present for this function to work */
    if(pData == NULL)
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    /* pData make sure that the hash table has entries. */
    if(pData->hashTable.pEntries == NULL)
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    /* Variables must always be named. */
    if(!IS_STRING_PRESENT(pVariableName))
        return CS64_INI_ENTRY_VARIABLE_EMPTY_ERROR;

    /* Check if it is time for a resize */
    if(pData->hashTable.currentEntryAmount >= pData->hashTable.entryCapacityUpLimit)
        cs64_ini_data_reserve(pData, pData->hashTable.entryCapacity + 1);

    /* Check if table is full. */
    if(pData->hashTable.currentEntryAmount == pData->hashTable.entryCapacity)
        return CS64_INI_ENTRY_NO_MEMORY_ERROR;

    CS64Size sectionLength = 0;
    CS64Size nameByteSize = 0;

    CS64Offset sectionHash = CS64_INI_INITIAL_HASH;

    if(IS_STRING_PRESENT(pSectionName))
        sectionHash = CS64_INI_HASH_FUNCTION(pSectionName,  sectionHash, &sectionLength);
    CS64Offset hash = CS64_INI_HASH_FUNCTION(pVariableName, sectionHash, &nameByteSize);

    CS64Offset originalIndex = hash % pData->hashTable.entryCapacity;
    CS64Offset index = originalIndex;
    CS64INIEntry *pEntry = &pData->hashTable.pEntries[index];

    ATTEMPT_TO_FIND_VARIABLE(
        pEntry, pSectionName, sectionLength, pVariableName, index, originalIndex, pData->hashTable, !IS_ENTRY_EMPTY(pEntry),
        {return CS64_INI_ENTRY_ENTRY_EXISTS_ERROR;},
        {return CS64_INI_ENTRY_NO_MEMORY_ERROR;})

    CS64UTF8 emptyValue[] = "";

    if(pValue == NULL)
        pValue = emptyValue;

    CS64Size valueByteSize = cs64_ini_string_byte_size(pValue);
    CS64UTF8 *pDynamicMemory = NULL;

    if(CS64_INI_IMP_DETAIL_VALUE_SIZE < nameByteSize + valueByteSize) {
        pDynamicMemory = CS64_INI_MALLOC(sizeof(CS64UTF8) * (nameByteSize + valueByteSize));

        if(pDynamicMemory == NULL)
            return CS64_INI_ENTRY_NO_MEMORY_ERROR;
    }

    CS64INIEntry *pLastValue = NULL;

    CS64Offset originalSectionIndex;
    CS64Offset sectionIndex;

    if(IS_STRING_PRESENT(pSectionName)) {
        CS64INIEntry *pSectionEntry = NULL;

        originalSectionIndex = sectionHash % pData->hashTable.entryCapacity;
        sectionIndex = originalSectionIndex;
        CS64INIEntry *pSearchForSectionEntry = &pData->hashTable.pEntries[sectionIndex];

        ATTEMPT_TO_FIND_SECTION(
            pSearchForSectionEntry, pSectionName, sectionLength, sectionIndex, originalSectionIndex, pData->hashTable,
            pSearchForSectionEntry->entryType != CS64_INI_ENTRY_EMPTY && pSectionEntry == NULL,
            {pSectionEntry = pSearchForSectionEntry;},
            {})

        if(pSectionEntry == NULL) {

            if(pDynamicMemory != NULL)
                CS64_INI_FREE(pDynamicMemory);

            return CS64_INI_ENTRY_ENTRY_DNE_ERROR;
        }

        pEntry->type.value.pSection = pSectionEntry;

        pLastValue = pSectionEntry->type.section.header.pLastValue;

        if( pSectionEntry->type.section.header.pFirstValue == NULL)
            pSectionEntry->type.section.header.pFirstValue = pEntry;
        pSectionEntry->type.section.header.pLastValue = pEntry;
    }
    else {
        pEntry->type.value.pSection = NULL;

        pLastValue = pData->globals.pLastValue;

        if( pData->globals.pFirstValue == NULL)
            pData->globals.pFirstValue = pEntry;
        pData->globals.pLastValue = pEntry;
    }

    pData->hashTable.currentEntryAmount++;

    pEntry->pPrev = NULL;
    pEntry->pNext = NULL;

    if(pLastValue != NULL) {
        pLastValue->pNext = pEntry;
        pEntry->pPrev = pLastValue;
    }

    /* New entries do not have comments */
    pEntry->commentSize       = 0;
    pEntry->pComment          = NULL;
    pEntry->inlineCommentSize = 0;
    pEntry->pInlineComment    = NULL;

    pEntry->type.value.nameByteSize  = nameByteSize;
    pEntry->type.value.valueByteSize = valueByteSize;

    if(pDynamicMemory != NULL) {
        pEntry->entryType = CS64_INI_ENTRY_DYNAMIC_VALUE;
        pEntry->type.value.data.dynamic.pName = pDynamicMemory;
        pEntry->type.value.data.dynamic.pValue = &pEntry->type.value.data.dynamic.pName[nameByteSize];
        STRING_COPY(pEntry->type.value.data.dynamic.pName,  pVariableName);
        STRING_COPY(pEntry->type.value.data.dynamic.pValue, pValue);
    }
    else {
        pEntry->entryType = CS64_INI_ENTRY_VALUE;
        STRING_COPY((&pEntry->type.section.name.fixed[0]),            pVariableName);
        STRING_COPY((&pEntry->type.section.name.fixed[nameByteSize]), pValue);
    }

    /* Finally return the entry. If the programmer gives ppEntry an address */
    if(ppEntry != NULL)
        *ppEntry = pEntry;

    return CS64_INI_ENTRY_SUCCESS;
}

CS64INIEntryState cs64_ini_add_section(CS64INIData *pData, const CS64UTF8 *const pSectionName, CS64INIEntry** ppEntry) {
    /* If the programmer gives ppEntry an address */
    if(ppEntry != NULL)
        *ppEntry = NULL;

    UTF8_CHECK(pSectionName);

    /* Data must be present for this function to work */
    if(pData == NULL)
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    /* pData make sure that the hash table has entries. */
    if(pData->hashTable.pEntries == NULL)
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    /* There already is a global section. */
    if(!IS_STRING_PRESENT(pSectionName))
        return CS64_INI_ENTRY_SECTION_EMPTY_ERROR;

    /* Check if it is time for a resize */
    if(pData->hashTable.currentEntryAmount >= pData->hashTable.entryCapacityUpLimit)
        cs64_ini_data_reserve(pData, pData->hashTable.entryCapacity + 1);

    /* Check if table is full. */
    if(pData->hashTable.currentEntryAmount == pData->hashTable.entryCapacity)
        return CS64_INI_ENTRY_NO_MEMORY_ERROR;

    CS64Size sectionByteSize = 0;

    CS64Offset originalIndex = CS64_INI_HASH_FUNCTION(pSectionName, CS64_INI_INITIAL_HASH, &sectionByteSize) % pData->hashTable.entryCapacity;
    CS64Offset index = originalIndex;

    CS64INIEntry *pEntry = &pData->hashTable.pEntries[index];

    ATTEMPT_TO_FIND_SECTION(pEntry, pSectionName, sectionByteSize, index, originalIndex, pData->hashTable, !IS_ENTRY_EMPTY(pEntry),
        {return CS64_INI_ENTRY_ENTRY_EXISTS_ERROR;},
        {return CS64_INI_ENTRY_NO_MEMORY_ERROR;})

    if(CS64_INI_IMP_DETAIL_SECTION_NAME_SIZE < sectionByteSize) {
        void *pAttempt = CS64_INI_MALLOC(sizeof(CS64UTF8) * sectionByteSize);

        if(pAttempt == NULL)
            return CS64_INI_ENTRY_NO_MEMORY_ERROR;

        pEntry->type.section.name.pDynamic = pAttempt;

        pEntry->entryType = CS64_INI_ENTRY_DYNAMIC_SECTION;
        STRING_COPY(pEntry->type.section.name.pDynamic, pSectionName);
    }
    else {
        pEntry->entryType = CS64_INI_ENTRY_SECTION;
        STRING_COPY(pEntry->type.section.name.fixed, pSectionName);
    }

    pData->hashTable.currentEntryAmount++;

    /* New Sections do not have variables */
    pEntry->type.section.header.pFirstValue = NULL;
    pEntry->type.section.header.pLastValue  = NULL;

    /* Apply the section byte size */
    pEntry->type.section.nameByteSize = sectionByteSize;

    /* New entries do not have comments */
    pEntry->commentSize       = 0;
    pEntry->pComment          = NULL;
    pEntry->inlineCommentSize = 0;
    pEntry->pInlineComment    = NULL;

    /* Empty section check */
    if(pData->pFirstSection == NULL) {
        /* Begin the double linked list */
        pData->pFirstSection = pEntry;
        pData->pLastSection  = pEntry;

        pEntry->pNext = NULL;
        pEntry->pPrev = NULL;
    } /* Has sections check */
    else {
        /* Append to back of double linked list */
        pData->pLastSection->pNext = pEntry;

        pEntry->pNext = NULL;
        pEntry->pPrev = pData->pLastSection;

        pData->pLastSection = pEntry;
    }

    /* Finally return the entry. If the programmer gives ppEntry an address */
    if(ppEntry != NULL)
        *ppEntry = pEntry;

    return CS64_INI_ENTRY_SUCCESS;
}


CS64INIEntry* cs64_ini_get_variable(CS64INIData *pData, const CS64UTF8 *const pSectionName, const CS64UTF8 *const pName) {
    /* Data must be present for this function to work */
    if(pData == NULL)
        return NULL;

    /* pData make sure that the hash table has entries. */
    if(pData->hashTable.pEntries == NULL)
        return NULL;

    /* If no name and section then there is no entry to find. */
    if(!IS_STRING_PRESENT(pName))
        return NULL;

    if(pData->hashTable.currentEntryAmount == 0)
        return NULL;

    CS64Size sectionLength = 0;
    CS64Size nameLength = 0;

    CS64Offset hash = CS64_INI_INITIAL_HASH;

    if(IS_STRING_PRESENT(pSectionName))
        hash = CS64_INI_HASH_FUNCTION(pSectionName, hash, &sectionLength);
    hash = CS64_INI_HASH_FUNCTION(pName, hash, &nameLength);

    CS64Offset originalIndex = hash % pData->hashTable.entryCapacity;
    CS64Offset index = originalIndex;

    CS64INIEntry *pEntry = &pData->hashTable.pEntries[index];

    ATTEMPT_TO_FIND_VARIABLE(pEntry, pSectionName, sectionLength, pName, index, originalIndex, pData->hashTable, pEntry != NULL, return pEntry;, {})

    return NULL;
}

CS64INIEntry* cs64_ini_get_section(CS64INIData *pData, const CS64UTF8 *const pSectionName) {
    /* Data must be present for this function to work */
    if(pData == NULL)
        return NULL;

    /* pData make sure that the hash table has entries. */
    if(pData->hashTable.pEntries == NULL)
        return NULL;

    /* If no name and section then there is no entry to find. */
    if(!IS_STRING_PRESENT(pSectionName))
        return NULL;

    if(pData->hashTable.currentEntryAmount == 0)
        return NULL;

    CS64Size sectionLength = 0;

    CS64Offset hash = CS64_INI_HASH_FUNCTION(pSectionName, CS64_INI_INITIAL_HASH, &sectionLength);

    CS64Offset originalIndex = hash % pData->hashTable.entryCapacity;
    CS64Offset index = originalIndex;

    CS64INIEntry *pEntry = &pData->hashTable.pEntries[index];

    ATTEMPT_TO_FIND_SECTION(pEntry, pSectionName, sectionLength, index, originalIndex, pData->hashTable, pEntry->entryType != CS64_INI_ENTRY_EMPTY, {return pEntry;}, {})

    return NULL;
}

CS64INIEntryState cs64_ini_del_entry(CS64INIData *pData, CS64INIEntry *pEntry) {
    /* Data must be present for this function to work */
    if(pData == NULL)
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    /* pData make sure that the hash table has entries. */
    if(pData->hashTable.pEntries == NULL)
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    /* Make sure that the table does have items in it to delete. */
    if(pEntry == NULL)
        return CS64_INI_ENTRY_ENTRY_EMPTY_ERROR;

    /* Make sure that the table does have items in it to delete. */
    if(pData->hashTable.currentEntryAmount == 0)
        return CS64_INI_ENTRY_ENTRY_DNE_ERROR;

    if(IS_ENTRY_SECTION(pEntry)) {
        CS64INIEntry *pVariable = pEntry->type.section.header.pFirstValue;
        while(pVariable != NULL) {
            /* Free Comment */
            if(pVariable->pComment != NULL)
                CS64_INI_FREE(pVariable->pComment);

            if(pVariable->pInlineComment != NULL)
                CS64_INI_FREE(pVariable->pInlineComment);

            pVariable->pComment = NULL;
            pVariable->pInlineComment = NULL;

            /* Free if dynamic */
            if(pVariable->entryType == CS64_INI_ENTRY_DYNAMIC_VALUE) {
                CS64_INI_FREE(pVariable->type.value.data.dynamic.pName); /* This also frees pValue */
            }

            pVariable->entryType = CS64_INI_ENTRY_WAS_OCCUPIED;

            pData->hashTable.currentEntryAmount--;

            pVariable = pVariable->pNext;
        }

        if(pEntry == pData->pFirstSection) {
            pData->pFirstSection = pEntry->pNext;
        }

        if(pEntry == pData->pLastSection) {
            pData->pLastSection = pEntry->pPrev;
        }
    } else if(IS_ENTRY_VALUE(pEntry)) {
        CS64SectionHeader *pSectionHeader = &pData->globals;

        if(pEntry->type.value.pSection != NULL)
            pSectionHeader = &pEntry->type.value.pSection->type.section.header;

        if(pEntry == pSectionHeader->pFirstValue)
            pSectionHeader->pFirstValue = pEntry->pNext;

        if(pEntry == pSectionHeader->pLastValue)
            pSectionHeader->pLastValue = pEntry->pPrev;
    } else {
        return CS64_INI_ENTRY_ENTRY_EMPTY_ERROR;
    }

    /* Standard Remove Element from double linked lists! */
    if(pEntry->pPrev != NULL)
        pEntry->pPrev->pNext = pEntry->pNext;

    if(pEntry->pNext != NULL)
        pEntry->pNext->pPrev = pEntry->pPrev;

    /* Free Comment */
    if(pEntry->pComment != NULL)
        CS64_INI_FREE(pEntry->pComment);

    if(pEntry->pInlineComment != NULL)
        CS64_INI_FREE(pEntry->pInlineComment);

    pEntry->pComment = NULL;
    pEntry->pInlineComment = NULL;

    /* Free if dynamic */
    switch(pEntry->entryType) {
        case CS64_INI_ENTRY_DYNAMIC_VALUE:
            if(pEntry->type.value.data.dynamic.pName != NULL)
                CS64_INI_FREE(pEntry->type.value.data.dynamic.pName); /* This also frees pValue */
            break;
        case CS64_INI_ENTRY_DYNAMIC_SECTION:
            if(pEntry->type.section.name.pDynamic != NULL)
                CS64_INI_FREE(pEntry->type.section.name.pDynamic);
            break;
        default:
    }

    pEntry->entryType = CS64_INI_ENTRY_WAS_OCCUPIED;

    pData->hashTable.currentEntryAmount--;

    if(pData->hashTable.currentEntryAmount < pData->hashTable.entryCapacityDownLimit)
        cs64_ini_data_reserve(pData, pData->hashTable.currentEntryAmount);

    return CS64_INI_ENTRY_SUCCESS;
}

/* ### Hash Table Entry ### */

CS64EntryType cs64_ini_get_entry_type(const CS64INIEntry *const pEntry) {
    if(pEntry == NULL)
        return CS64_INI_ENTRY_EMPTY;

    switch(pEntry->entryType) {
        case CS64_INI_ENTRY_SECTION:
        case CS64_INI_ENTRY_DYNAMIC_SECTION:
            return CS64_INI_ENTRY_SECTION;
        case CS64_INI_ENTRY_VALUE:
        case CS64_INI_ENTRY_DYNAMIC_VALUE:
            return CS64_INI_ENTRY_VALUE;
        default:
            return CS64_INI_ENTRY_EMPTY;
    }
}

CS64INIEntry* cs64_ini_get_first_section(CS64INIData *pData) {
    if(pData == NULL)
        return NULL;

    return pData->pFirstSection;
}

CS64INIEntry* cs64_ini_get_first_global_value(CS64INIData *pData) {
    if(pData == NULL)
        return NULL;

    return pData->globals.pFirstValue;
}

CS64INIEntry* cs64_ini_get_first_section_value(CS64INIEntry *pEntry) {
    if(pEntry == NULL)
        return NULL;

    if(cs64_ini_get_entry_type(pEntry) != CS64_INI_ENTRY_SECTION)
        return NULL;

    return pEntry->type.section.header.pFirstValue;
}

CS64INIEntry* cs64_ini_get_next_entry(CS64INIEntry *pEntry) {
    if(pEntry == NULL)
        return NULL;

    return pEntry->pNext;
}

CS64INIEntry* cs64_ini_get_prev_entry(CS64INIEntry *pEntry) {
    if(pEntry == NULL)
        return NULL;

    return pEntry->pPrev;
}

CS64INIEntry* cs64_ini_get_entry_section(const CS64INIEntry *const pEntry) {
    if(pEntry == NULL)
        return NULL;

    if(cs64_ini_get_entry_type(pEntry) == CS64_INI_ENTRY_EMPTY)
        return NULL;

    if(cs64_ini_get_entry_type(pEntry) == CS64_INI_ENTRY_SECTION)
        return NULL;

    return pEntry->type.value.pSection;
}

const CS64UTF8 *const cs64_ini_get_entry_section_name(const CS64INIEntry *const pEntry) {
    const CS64INIEntry *const pSectionEntry = cs64_ini_get_entry_section(pEntry);

    if(pSectionEntry == NULL)
        return NULL;

    if(pSectionEntry->entryType == CS64_INI_ENTRY_DYNAMIC_SECTION)
        return pSectionEntry->type.section.name.pDynamic;
    return pSectionEntry->type.section.name.fixed;
}

CS64INIEntryState cs64_ini_set_entry_name(CS64INIData *pData, const CS64UTF8 *const pValue, CS64INIEntry **ppEntry) {
    UTF8_CHECK(pValue);

    if(pData == NULL)
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    if(ppEntry == NULL)
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    CS64INIEntry *pOldEntry = *ppEntry;

    if(pOldEntry == NULL)
        return CS64_INI_ENTRY_ENTRY_DNE_ERROR;

    /* A section or variable with an empty name is invalid. */
    if(!IS_STRING_PRESENT(pValue))
        return CS64_INI_ENTRY_VARIABLE_EMPTY_ERROR;

    CS64INIEntry *pMovedEntry = NULL;
    CS64EntryType backupEntryType = pOldEntry->entryType;

    CS64Size sectionByteSize;
    CS64Size nameByteSize;
    CS64Offset sectionHash = CS64_INI_INITIAL_HASH;
    CS64Offset originalIndex;
    CS64Offset index;

    switch(pOldEntry->entryType) {
        case CS64_INI_ENTRY_DYNAMIC_SECTION:
        case CS64_INI_ENTRY_SECTION:
        {
            /* Check if there is a section with the name as pValue in the hash table. */
            if(cs64_ini_get_section(pData, pValue) != NULL)
                return CS64_INI_ENTRY_ENTRY_EXISTS_ERROR;

            sectionByteSize = 0;

            /* Handle the deletion. */
            pOldEntry->entryType = CS64_INI_ENTRY_WAS_OCCUPIED;
            sectionHash = CS64_INI_HASH_FUNCTION(pValue, sectionHash, &sectionByteSize);
            originalIndex = sectionHash % pData->hashTable.entryCapacity;
            index = originalIndex;
            pMovedEntry = &pData->hashTable.pEntries[index];

            ATTEMPT_TO_FIND_SECTION(pMovedEntry, pValue, sectionByteSize, index, originalIndex, pData->hashTable, !IS_ENTRY_EMPTY(pMovedEntry),
                {pOldEntry->entryType = backupEntryType; return CS64_INI_ENTRY_ENTRY_EXISTS_ERROR;},
                {pOldEntry->entryType = backupEntryType; return CS64_INI_ENTRY_NO_MEMORY_ERROR;})

            if(CS64_INI_IMP_DETAIL_SECTION_NAME_SIZE < sectionByteSize) {
                void *pAttempt = CS64_INI_MALLOC(sizeof(CS64UTF8) * sectionByteSize);
                if(pAttempt == NULL) {
                    pOldEntry->entryType = backupEntryType;
                    return CS64_INI_ENTRY_NO_MEMORY_ERROR;
                }
                if(backupEntryType == CS64_INI_ENTRY_DYNAMIC_SECTION)
                    CS64_INI_FREE(pOldEntry->type.section.name.pDynamic);

                pMovedEntry->type.section.name.pDynamic = pAttempt;
                pMovedEntry->entryType = CS64_INI_ENTRY_DYNAMIC_SECTION;
                STRING_COPY(pMovedEntry->type.section.name.pDynamic, pValue);
            }
            else {
                if(backupEntryType == CS64_INI_ENTRY_DYNAMIC_SECTION)
                    CS64_INI_FREE(pOldEntry->type.section.name.pDynamic);

                pMovedEntry->entryType = CS64_INI_ENTRY_SECTION;
                STRING_COPY(pMovedEntry->type.section.name.fixed, pValue);
            }

            pMovedEntry->type.section.nameByteSize = sectionByteSize;

            CS64INIEntry *pSectionVariable = pOldEntry->type.section.header.pFirstValue;

            /* Since element has been moved in this case then. */
            pMovedEntry->type.section.header.pFirstValue = NULL;
            pMovedEntry->type.section.header.pLastValue  = NULL;

            /* Modify pData if pOldEntry is at the beginning or end of sections. */
            if(pData->pFirstSection == pOldEntry)
                pData->pFirstSection = pMovedEntry;
            if(pData->pLastSection == pOldEntry)
                pData->pLastSection = pMovedEntry;

            /* Modify Left and Right child of pOldEntry to point to pMovedEntry. */
            if(pOldEntry->pNext != NULL)
                pOldEntry->pNext->pPrev = pMovedEntry;
            pMovedEntry->pNext = pOldEntry->pNext;
            if(pOldEntry->pPrev != NULL)
                pOldEntry->pPrev->pNext = pMovedEntry;
            pMovedEntry->pPrev = pOldEntry->pPrev;

            /* Move the comments */
            pMovedEntry->commentSize       = pOldEntry->commentSize;
            pMovedEntry->pComment          = pOldEntry->pComment;
            pMovedEntry->inlineCommentSize = pOldEntry->inlineCommentSize;
            pMovedEntry->pInlineComment    = pOldEntry->pInlineComment;

            if(pMovedEntry != pOldEntry) {
                pOldEntry->pNext          = NULL;
                pOldEntry->pPrev          = NULL;
                pOldEntry->pComment       = NULL;
                pOldEntry->pInlineComment = NULL;
            }

            /* Fix the hash table locations. */
            CS64INIEntry *pNextEntry;
            while(pSectionVariable != NULL) {
                pSectionVariable->type.value.pSection = pMovedEntry;

                pNextEntry = pSectionVariable->pNext;

                backupEntryType = pSectionVariable->entryType;

                pSectionVariable->entryType = CS64_INI_ENTRY_WAS_OCCUPIED;

                nameByteSize = 0;

                CS64Offset hash = CS64_INI_HASH_FUNCTION(pValue, sectionHash, &nameByteSize);
                originalIndex = hash % pData->hashTable.entryCapacity;
                index = originalIndex;
                CS64INIEntry *pMovedVariable = &pData->hashTable.pEntries[index];

                const CS64UTF8 * pName;

                if(backupEntryType == CS64_INI_ENTRY_DYNAMIC_VALUE)
                    pName = pSectionVariable->type.value.data.dynamic.pName;
                else
                    pName = pSectionVariable->type.value.data.fixed;

                ATTEMPT_TO_FIND_VARIABLE(
                    pMovedVariable, pValue, sectionByteSize, pName, index, originalIndex, pData->hashTable, !IS_ENTRY_EMPTY(pMovedVariable),
                    {pSectionVariable->entryType = backupEntryType; return CS64_INI_ENTRY_ENTRY_EXISTS_ERROR;},
                    {pSectionVariable->entryType = backupEntryType; return CS64_INI_ENTRY_NO_MEMORY_ERROR;})

                if(pMovedEntry->type.section.header.pFirstValue == NULL)
                    pMovedEntry->type.section.header.pFirstValue = pMovedVariable;
                pMovedEntry->type.section.header.pLastValue = pMovedVariable;

                /* Check if the variable has been moved. */
                pMovedVariable->type.value = pSectionVariable->type.value;

                /* Modify Left and Right child of pSectionVariable to point to pMovedVariable. */
                if(pSectionVariable->pNext != NULL)
                    pSectionVariable->pNext->pPrev = pMovedVariable;
                pMovedVariable->pNext = pSectionVariable->pNext;
                if(pSectionVariable->pPrev != NULL)
                    pSectionVariable->pPrev->pNext = pMovedVariable;
                pMovedVariable->pPrev = pSectionVariable->pPrev;

                /* Move the comments */
                pMovedVariable->commentSize       = pSectionVariable->commentSize;
                pMovedVariable->pComment          = pSectionVariable->pComment;
                pMovedVariable->inlineCommentSize = pSectionVariable->inlineCommentSize;
                pMovedVariable->pInlineComment    = pSectionVariable->pInlineComment;

                if(pMovedVariable != pSectionVariable) {
                    /* Zero out the memory on the section variable. */
                    pSectionVariable->pNext          = NULL;
                    pSectionVariable->pPrev          = NULL;
                    pSectionVariable->pComment       = NULL;
                    pSectionVariable->pInlineComment = NULL;
                }

                pMovedVariable->entryType = backupEntryType;

                pSectionVariable = pNextEntry;
            }

            *ppEntry = pMovedEntry;
        }
            break;

        case CS64_INI_ENTRY_DYNAMIC_VALUE:
        case CS64_INI_ENTRY_VALUE:
        {
            CS64Value backupValue = pOldEntry->type.value;

            const CS64UTF8 *const pSectionName = cs64_ini_get_entry_section_name(pOldEntry);

            /* Check if there is a variable with the name as pValue in the hash table. */
            if(cs64_ini_get_variable(pData, pSectionName, pValue) != NULL)
                return CS64_INI_ENTRY_ENTRY_EXISTS_ERROR;

            sectionByteSize = 0;
            nameByteSize = 0;

            if(IS_STRING_PRESENT(pSectionName))
                sectionHash = CS64_INI_HASH_FUNCTION(pSectionName, sectionHash, &sectionByteSize);
            CS64Offset hash = CS64_INI_HASH_FUNCTION(pValue, sectionHash, &nameByteSize);

            originalIndex = hash % pData->hashTable.entryCapacity;
            index = originalIndex;
            CS64INIEntry *pRenamedVariable = &pData->hashTable.pEntries[index];

            pOldEntry->entryType = CS64_INI_ENTRY_WAS_OCCUPIED;

            ATTEMPT_TO_FIND_VARIABLE(
                pRenamedVariable, pSectionName, sectionByteSize, pValue, index, originalIndex, pData->hashTable, !IS_ENTRY_EMPTY(pRenamedVariable),
                {pOldEntry->entryType = backupEntryType; return CS64_INI_ENTRY_ENTRY_EXISTS_ERROR;},
                {pOldEntry->entryType = backupEntryType; return CS64_INI_ENTRY_NO_MEMORY_ERROR;})

            CS64UTF8 *pDynamicMemory = NULL;

            if(CS64_INI_IMP_DETAIL_VALUE_SIZE < nameByteSize + pOldEntry->type.value.valueByteSize) {
                pDynamicMemory = CS64_INI_MALLOC(sizeof(CS64UTF8) * (nameByteSize + pOldEntry->type.value.valueByteSize));

                if(pDynamicMemory == NULL) {
                    pOldEntry->entryType = backupEntryType;
                    return CS64_INI_ENTRY_NO_MEMORY_ERROR;
                }
            }

            if(pDynamicMemory != NULL) {
                pRenamedVariable->entryType = CS64_INI_ENTRY_DYNAMIC_VALUE;
                pRenamedVariable->type.value.data.dynamic.pName = pDynamicMemory;
                pRenamedVariable->type.value.data.dynamic.pValue = &pRenamedVariable->type.value.data.dynamic.pName[nameByteSize];

                STRING_COPY(pRenamedVariable->type.value.data.dynamic.pName,  pValue);
                if(backupEntryType == CS64_INI_ENTRY_DYNAMIC_VALUE) {
                    STRING_COPY(pRenamedVariable->type.value.data.dynamic.pValue, backupValue.data.dynamic.pValue);
                }
                else {
                    STRING_COPY(pRenamedVariable->type.value.data.dynamic.pValue, (&backupValue.data.fixed[backupValue.nameByteSize]));
                }
            }
            else {
                pRenamedVariable->entryType = CS64_INI_ENTRY_VALUE;

                STRING_COPY((&pRenamedVariable->type.section.name.fixed[0]), pValue);
                if(backupEntryType == CS64_INI_ENTRY_DYNAMIC_VALUE) {
                    STRING_COPY((&pRenamedVariable->type.section.name.fixed[nameByteSize]), backupValue.data.dynamic.pValue);
                }
                else {
                    STRING_COPY((&pRenamedVariable->type.section.name.fixed[nameByteSize]), (&backupValue.data.fixed[backupValue.nameByteSize]));
                }
            }

            if(backupEntryType == CS64_INI_ENTRY_DYNAMIC_VALUE)
                CS64_INI_FREE(backupValue.data.dynamic.pName);

            pRenamedVariable->type.value.pSection      = backupValue.pSection;
            pRenamedVariable->type.value.nameByteSize  = nameByteSize;
            pRenamedVariable->type.value.valueByteSize = backupValue.valueByteSize;

            if(pRenamedVariable->type.value.pSection == NULL) {
                if(pData->globals.pFirstValue == pOldEntry)
                    pData->globals.pFirstValue = pRenamedVariable;
                if(pData->globals.pLastValue == pOldEntry)
                    pData->globals.pLastValue = pRenamedVariable;
            }
            else {
                if(pRenamedVariable->type.value.pSection->type.section.header.pFirstValue == pOldEntry)
                    pRenamedVariable->type.value.pSection->type.section.header.pFirstValue = pRenamedVariable;
                if(pRenamedVariable->type.value.pSection->type.section.header.pLastValue == pOldEntry)
                    pRenamedVariable->type.value.pSection->type.section.header.pLastValue = pRenamedVariable;
            }

            /* Modify Left and Right child of pOldEntry to point to pRenamedVariable. */
            if(pOldEntry->pNext != NULL)
                pOldEntry->pNext->pPrev = pRenamedVariable;
            pRenamedVariable->pNext = pOldEntry->pNext;
            if(pOldEntry->pPrev != NULL)
                pOldEntry->pPrev->pNext = pRenamedVariable;
            pRenamedVariable->pPrev = pOldEntry->pPrev;

            /* Move the comments */
            pRenamedVariable->commentSize       = pOldEntry->commentSize;
            pRenamedVariable->pComment          = pOldEntry->pComment;
            pRenamedVariable->inlineCommentSize = pOldEntry->inlineCommentSize;
            pRenamedVariable->pInlineComment    = pOldEntry->pInlineComment;

            if(pRenamedVariable != pOldEntry) {
                pOldEntry->pNext          = NULL;
                pOldEntry->pPrev          = NULL;
                pOldEntry->pComment       = NULL;
                pOldEntry->pInlineComment = NULL;
            }

            *ppEntry = pRenamedVariable;
        }
            break;

        default:
            return CS64_INI_ENTRY_ENTRY_EMPTY_ERROR;
    }

    return CS64_INI_ENTRY_SUCCESS;
}

const CS64UTF8 *const cs64_ini_get_entry_name(const CS64INIEntry *const pEntry) {
    if(pEntry == NULL)
        return NULL;

    switch(pEntry->entryType) {
        case CS64_INI_ENTRY_SECTION:
            return pEntry->type.section.name.fixed;
        case CS64_INI_ENTRY_DYNAMIC_SECTION:
            return pEntry->type.section.name.pDynamic;
        case CS64_INI_ENTRY_VALUE:
            return pEntry->type.value.data.fixed;
        case CS64_INI_ENTRY_DYNAMIC_VALUE:
            return pEntry->type.value.data.dynamic.pName;
        default:
            return NULL;
    }
}

CS64INIEntryState cs64_ini_set_entry_value(CS64INIEntry *pEntry, const CS64UTF8 *pNewValue) {

    UTF8_CHECK(pNewValue);

    const CS64UTF8 emptyString[] = "";

    if(pEntry == NULL)
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    if(!IS_ENTRY_VALUE(pEntry))
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    if(pNewValue == NULL)
        pNewValue = emptyString;

    const CS64Size valueByteSize = cs64_ini_string_byte_size(pNewValue);
    const CS64Size totalByteSize = valueByteSize + pEntry->type.value.nameByteSize;

    if(pEntry->entryType == CS64_INI_ENTRY_DYNAMIC_VALUE) {
        if(totalByteSize > CS64_INI_IMP_DETAIL_VALUE_SIZE) {
            CS64UTF8 *pNameAndValue = CS64_INI_MALLOC(totalByteSize);

            if(pNameAndValue == NULL)
                return CS64_INI_ENTRY_NO_MEMORY_ERROR;

            STRING_COPY( pNameAndValue,                                    pEntry->type.value.data.dynamic.pName)
            STRING_COPY((pNameAndValue + pEntry->type.value.nameByteSize), pNewValue)

            CS64_INI_FREE(pEntry->type.value.data.dynamic.pName);

            pEntry->entryType = CS64_INI_ENTRY_DYNAMIC_VALUE;
            pEntry->type.value.data.dynamic.pName  = pNameAndValue;
            pEntry->type.value.data.dynamic.pValue = pNameAndValue + pEntry->type.value.nameByteSize;
        }
        else {
            CS64UTF8 *pOldPointer = pEntry->type.value.data.dynamic.pName;

            STRING_COPY( pEntry->type.value.data.fixed,                                    pOldPointer)
            STRING_COPY((pEntry->type.value.data.fixed + pEntry->type.value.nameByteSize), pNewValue)

            CS64_INI_FREE(pOldPointer);

            pEntry->entryType = CS64_INI_ENTRY_VALUE;
        }
    }
    else {
        if(totalByteSize > CS64_INI_IMP_DETAIL_VALUE_SIZE) {
            CS64UTF8 *pNameAndValue = CS64_INI_MALLOC(totalByteSize);

            if(pNameAndValue == NULL)
                return CS64_INI_ENTRY_NO_MEMORY_ERROR;

            STRING_COPY( pNameAndValue,                                    pEntry->type.value.data.fixed)
            STRING_COPY((pNameAndValue + pEntry->type.value.nameByteSize), pNewValue)

            pEntry->entryType = CS64_INI_ENTRY_DYNAMIC_VALUE;
            pEntry->type.value.data.dynamic.pName  = pNameAndValue;
            pEntry->type.value.data.dynamic.pValue = pNameAndValue + pEntry->type.value.nameByteSize;
        }
        else {
            STRING_COPY((pEntry->type.value.data.fixed + pEntry->type.value.nameByteSize), pNewValue)

            pEntry->entryType = CS64_INI_ENTRY_VALUE;
        }
    }
    pEntry->type.value.valueByteSize = valueByteSize;

    return CS64_INI_ENTRY_SUCCESS;
}

const CS64UTF8 *const cs64_ini_get_entry_value(const CS64INIEntry *const pEntry) {
    if(pEntry == NULL)
        return NULL;

    if(pEntry->entryType == CS64_INI_ENTRY_VALUE) {
        return &pEntry->type.value.data.fixed[pEntry->type.value.nameByteSize];
    }
    else if(pEntry->entryType == CS64_INI_ENTRY_DYNAMIC_VALUE) {
        return pEntry->type.value.data.dynamic.pValue;
    }
    else
        return NULL;
}

CS64INIEntryState cs64_ini_set_entry_comment(CS64INIEntry *pEntry, const CS64UTF8 *const pValue) {

    UTF8_CHECK(pValue);

    if(pEntry == NULL)
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    /* Free Comment */
    if(pEntry->pComment != NULL)
        CS64_INI_FREE(pEntry->pComment);

    pEntry->pComment = NULL;
    pEntry->commentSize = 0;

    if(!IS_STRING_PRESENT(pValue))
        return CS64_INI_ENTRY_SUCCESS;

    CS64Size valueByteSize = cs64_ini_string_byte_size(pValue);

    pEntry->pComment = CS64_INI_MALLOC(valueByteSize);

    if(pEntry->pComment == NULL)
        return CS64_INI_ENTRY_NO_MEMORY_ERROR;

    STRING_COPY(pEntry->pComment, pValue)
    pEntry->commentSize = valueByteSize;

    return CS64_INI_ENTRY_SUCCESS;
}

const CS64UTF8 *const cs64_ini_get_entry_comment(const CS64INIEntry *const pEntry) {
    if(pEntry == NULL)
        return NULL;
    return pEntry->pComment;
}

CS64INIEntryState cs64_ini_set_entry_inline_comment(CS64INIEntry *pEntry, const CS64UTF8 *const pValue) {

    UTF8_CHECK(pValue);

    /* Also check if new line is in pValue which would invalidate this function! */
    if(pValue != NULL) {
        CS64Size c = 0;
        CS64Size l = cs64_ini_string_byte_size(pValue);
        CS64Size charByteSize;

        while(c < l) {
            if(cs64_ini_utf_8_read(&pValue[c], l - c, &charByteSize) == '\n')
                return CS64_INI_ENTRY_ILLEGAL_STRING_ERROR;
            c += charByteSize;
        }
    }

    if(pEntry == NULL)
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    /* Free Comment */
    if(pEntry->pInlineComment != NULL)
        CS64_INI_FREE(pEntry->pInlineComment);

    pEntry->pInlineComment = NULL;
    pEntry->inlineCommentSize = 0;

    if(!IS_STRING_PRESENT(pValue))
        return CS64_INI_ENTRY_SUCCESS;

    CS64Size valueByteSize = cs64_ini_string_byte_size(pValue);

    pEntry->pInlineComment = CS64_INI_MALLOC(valueByteSize);

    if(pEntry->pInlineComment == NULL)
        return CS64_INI_ENTRY_NO_MEMORY_ERROR;

    STRING_COPY(pEntry->pInlineComment, pValue)
    pEntry->inlineCommentSize = valueByteSize;

    return CS64_INI_ENTRY_SUCCESS;
}

const CS64UTF8 *const cs64_ini_get_entry_inline_comment(const CS64INIEntry *const pEntry) {
    if(pEntry == NULL)
        return NULL;
    return pEntry->pInlineComment;
}

CS64INIEntryState cs64_ini_set_last_comment(CS64INIData *pData, const CS64UTF8 *const pValue) {

    UTF8_CHECK(pValue);

    if(pData == NULL)
        return CS64_INI_ENTRY_DATA_NULL_ERROR;

    /* Free Comment */
    if(pData->pLastComment != NULL)
        CS64_INI_FREE(pData->pLastComment);

    pData->pLastComment = NULL;
    pData->lastCommentSize = 0;

    if(!IS_STRING_PRESENT(pValue))
        return CS64_INI_ENTRY_SUCCESS;

    CS64Size valueByteSize = cs64_ini_string_byte_size(pValue);

    pData->pLastComment = CS64_INI_MALLOC(valueByteSize);

    if(pData->pLastComment == NULL)
        return CS64_INI_ENTRY_NO_MEMORY_ERROR;

    STRING_COPY(pData->pLastComment, pValue)
    pData->lastCommentSize = valueByteSize;

    return CS64_INI_ENTRY_SUCCESS;
}

const CS64UTF8 *const cs64_ini_get_last_comment(CS64INIData *pData) {
    if(pData == NULL)
        return NULL;
    return pData->pLastComment;
}

#undef INITIAL_CAPACITY
#undef P2_LIMIT
#undef CALC_UPPER_LIMIT
#undef CALC_P2_LOWER_LIMIT
#undef CALC_LR_LOWER_LIMIT
#undef CALC_LOWER_LIMIT
#undef INIT_HASH_TABLE
#undef IS_STRING_PRESENT
#undef IS_ENTRY_EMPTY
#undef IS_ENTRY_SECTION
#undef STRING_COPY
#undef IS_SAME_SECTION_ENTRY
#undef ATTEMPT_TO_FIND_SECTION
#undef IS_ENTRY_VALUE
#undef IS_CORRECT_VARIABLE
#undef ATTEMPT_TO_FIND_VARIABLE
#undef UTF8_CHECK

/* ### PARSER SECTION ### */

#define COPY_TOKEN_TO_STRING_BUFFER(X)\
    /* The byte length must not exceed the buffer limit. */\
    if(pToken->byteLength >= pParserContext->stringBufferLimit) {\
        result.state = CS64_INI_PARSER_LEXER_MEM_ERROR;\
        return result;\
    }\
\
    /* Copy operation. */\
    {\
        CS64Size length = 0;\
        while(length != pToken->byteLength) {\
            pParserContext->X[length] = pParserContext->pSource[pToken->index + length];\
            length++;\
        }\
        pParserContext->X[length] = '\0';\
        length++;\
    }

#define RETURN_EXPECTED_TOKEN_ERROR(X, expected_tokens)\
    X.state = CS64_INI_PARSER_UNEXPECTED_ERROR;\
    X.status.unexpected_token.receivedToken = *pToken;\
    X.status.unexpected_token.expectedTokenAmount = sizeof(expected_tokens) / sizeof(expected_tokens[0]);\
    X.status.unexpected_token.pExpectedTokens = expected_tokens;\
    return X;

#define RETURN_IF_WRONG_TOKEN(X, Y, EXPECTED_TOKEN)\
    if((Y)->type != EXPECTED_TOKEN) {\
        const static CS64INITokenType expected_tokens[] = {EXPECTED_TOKEN};\
        RETURN_EXPECTED_TOKEN_ERROR(X, expected_tokens)\
    }

#define RETURN_IF_DATA_ERROR(X, STATE, FUNC_NAME)\
    if(STATE != CS64_INI_ENTRY_SUCCESS) {\
        X.state = CS64_INI_PARSER_INI_DATA_ERROR;\
        X.status.data_error.pFunctionName = FUNC_NAME;\
        X.status.data_error.functionStatus = STATE;\
        return X;\
    }

#define ADD_OR_ERROR_IF_COMMENT_PRESENT(X, STATE, PARSER_CONTEXT, COMMENT_TOKEN_OFFSET, AMOUNT, FUNC, FUNC_NAME)\
    if(AMOUNT != 0) {\
        pToken = cs64_ini_token_data_get_token(PARSER_CONTEXT->pTokenResult->pTokenStorage, COMMENT_TOKEN_OFFSET);\
\
        COPY_TOKEN_TO_STRING_BUFFER(pStringBuffer)\
\
        /* Add the comment to the Entry if possible */\
        STATE = FUNC(pEntry, PARSER_CONTEXT->pStringBuffer);\
\
        /* If this function fails then return an error. */\
        RETURN_IF_DATA_ERROR(X, STATE, FUNC_NAME)\
    }

#define ADD_SECTION_OR_ERROR\
    /* Add the section */\
    pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, sectionTokenOffset);\
\
    COPY_TOKEN_TO_STRING_BUFFER(pStringBuffer)\
\
    entryState = cs64_ini_add_section(pParserContext->pData, pParserContext->pStringBuffer, &pEntry);\
\
    RETURN_IF_DATA_ERROR(result, entryState, add_section_str)\
\
    /* Remember the section. */\
    pParserContext->pSection = pEntry;

#define ADD_VARIABLE_OR_ERROR\
    pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, valueTokenOffset);\
\
    COPY_TOKEN_TO_STRING_BUFFER(pValueBuffer)\
\
    /* Create variable entry */\
    entryState = cs64_ini_add_variable(pParserContext->pData, cs64_ini_get_entry_name(pParserContext->pSection), pParserContext->pStringBuffer, pParserContext->pValueBuffer, &pEntry);\
\
    RETURN_IF_DATA_ERROR(result, entryState, add_variable_str)

CS64INIParserResult cs64_ini_parse_line(CS64INIParserContext *pParserContext) {
    const static CS64UTF8   add_section_str[] = "cs64_ini_add_section";
    const static CS64UTF8  add_variable_str[] = "cs64_ini_add_variable";
    const static CS64UTF8  last_comment_str[] = "cs64_ini_set_last_comment";
    const static CS64UTF8 entry_comment_str[] = "cs64_ini_set_entry_comment";
    const static CS64UTF8 entry_inline_comment_str[] = "cs64_ini_set_entry_inline_comment";

    CS64INIParserResult result;

    result.state = CS64_INI_PARSER_SUCCESS;

    CS64Offset commentTokenOffset = 0;
    CS64Size   commentAmount = 0; /* Zero means no token state. */

    CS64INIToken* pToken;
    CS64INIEntry *pEntry = NULL;
    CS64INIEntryState entryState;

    pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, pParserContext->tokenOffset);
    if(pToken == NULL) {
        result.state = CS64_INI_PARSER_LEXER_MEM_ERROR;
        return result;
    }

    if(pToken->type == CS64_INI_TOKEN_COMMENT) {
        commentTokenOffset = pParserContext->tokenOffset;
        commentAmount = 1;

        /* Comment token successfully read. */
        pParserContext->tokenOffset++;
        pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, pParserContext->tokenOffset);
        if(pToken == NULL) {
            result.state = CS64_INI_PARSER_LEXER_MEM_ERROR;
            return result;
        }

        RETURN_IF_WRONG_TOKEN(result, pToken, CS64_INI_TOKEN_END)

        pParserContext->tokenOffset++;
        pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, pParserContext->tokenOffset);
        if(pToken == NULL) {
            pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, commentTokenOffset);

            COPY_TOKEN_TO_STRING_BUFFER(pStringBuffer)

            entryState = cs64_ini_set_last_comment(pParserContext->pData, pParserContext->pStringBuffer);

            RETURN_IF_DATA_ERROR(result, entryState, last_comment_str)

            return result; /* This means that there is nothing else.*/
        }
    }

    if(pToken->type == CS64_INI_TOKEN_SECTION_START) {
        /* CS64_INI_TOKEN_SECTION_START token successfully read. */
        pParserContext->tokenOffset++;
        pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, pParserContext->tokenOffset);
        if(pToken == NULL) {
            result.state = CS64_INI_PARSER_LEXER_MEM_ERROR;
            return result;
        }

        RETURN_IF_WRONG_TOKEN(result, pToken, CS64_INI_TOKEN_VALUE)

        CS64Offset sectionTokenOffset = pParserContext->tokenOffset;
        CS64Size   sectionAmount = 1;

        pParserContext->tokenOffset++;
        pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, pParserContext->tokenOffset);
        if(pToken == NULL) {
            result.state = CS64_INI_PARSER_LEXER_MEM_ERROR;
            return result;
        }

        RETURN_IF_WRONG_TOKEN(result, pToken, CS64_INI_TOKEN_SECTION_END)

        pParserContext->tokenOffset++;
        pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, pParserContext->tokenOffset);
        if(pToken == NULL) {
            result.state = CS64_INI_PARSER_LEXER_MEM_ERROR;
            return result;
        }

        if(pToken->type == CS64_INI_TOKEN_END) {
            ADD_SECTION_OR_ERROR
        } else if(pToken->type == CS64_INI_TOKEN_COMMENT) {
            CS64Offset inlineCommentTokenOffset = pParserContext->tokenOffset;

            pParserContext->tokenOffset++;
            pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, pParserContext->tokenOffset);
            if(pToken == NULL) {
                result.state = CS64_INI_PARSER_LEXER_MEM_ERROR;
                return result;
            }

            RETURN_IF_WRONG_TOKEN(result, pToken, CS64_INI_TOKEN_END)

            ADD_SECTION_OR_ERROR

            /* Add the inline comment to the entry */
            ADD_OR_ERROR_IF_COMMENT_PRESENT(result, entryState, pParserContext, inlineCommentTokenOffset, 1, cs64_ini_set_entry_inline_comment, entry_inline_comment_str)
        } else {
            const static CS64INITokenType expected_tokens[] = {CS64_INI_TOKEN_COMMENT, CS64_INI_TOKEN_END};
            RETURN_EXPECTED_TOKEN_ERROR(result, expected_tokens)
        }

        ADD_OR_ERROR_IF_COMMENT_PRESENT(result, entryState, pParserContext, commentTokenOffset, commentAmount, cs64_ini_set_entry_comment, entry_comment_str)
    } else if(pToken->type == CS64_INI_TOKEN_VALUE) {

        CS64Size keyAmount = 1;

        /* CS64_INI_TOKEN_VALUE token successfully read. */

        /* Copy token to buffer. */
        COPY_TOKEN_TO_STRING_BUFFER(pStringBuffer)

        /* Advance tokenOffset */
        pParserContext->tokenOffset++;
        pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, pParserContext->tokenOffset);
        if(pToken == NULL) {
            result.state = CS64_INI_PARSER_LEXER_MEM_ERROR;
            return result;
        }

        RETURN_IF_WRONG_TOKEN(result, pToken, CS64_INI_TOKEN_DELEMETER)

        pParserContext->tokenOffset++;
        pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, pParserContext->tokenOffset);
        if(pToken == NULL) {
            result.state = CS64_INI_PARSER_LEXER_MEM_ERROR;
            return result;
        }

        RETURN_IF_WRONG_TOKEN(result, pToken, CS64_INI_TOKEN_VALUE)

        CS64Offset valueTokenOffset = pParserContext->tokenOffset;
        CS64Size   valueAmount = 1;

        pParserContext->tokenOffset++;
        pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, pParserContext->tokenOffset);
        if(pToken == NULL) {
            result.state = CS64_INI_PARSER_LEXER_MEM_ERROR;
            return result;
        }

        if(pToken->type == CS64_INI_TOKEN_END) {
            ADD_VARIABLE_OR_ERROR
        } else if(pToken->type == CS64_INI_TOKEN_COMMENT) {
            CS64Offset inlineCommentTokenOffset = pParserContext->tokenOffset;

            pParserContext->tokenOffset++;
            pToken = cs64_ini_token_data_get_token(pParserContext->pTokenResult->pTokenStorage, pParserContext->tokenOffset);
            if(pToken == NULL) {
                result.state = CS64_INI_PARSER_LEXER_MEM_ERROR;
                return result;
            }

            RETURN_IF_WRONG_TOKEN(result, pToken, CS64_INI_TOKEN_END)

            ADD_VARIABLE_OR_ERROR

            /* Add the inline comment to the entry */
            ADD_OR_ERROR_IF_COMMENT_PRESENT(result, entryState, pParserContext, inlineCommentTokenOffset, 1, cs64_ini_set_entry_inline_comment, entry_inline_comment_str)
        } else {
            const static CS64INITokenType expected_tokens[] = {CS64_INI_TOKEN_COMMENT, CS64_INI_TOKEN_END};
            RETURN_EXPECTED_TOKEN_ERROR(result, expected_tokens)
        }

        ADD_OR_ERROR_IF_COMMENT_PRESENT(result, entryState, pParserContext, commentTokenOffset, commentAmount, cs64_ini_set_entry_comment, entry_comment_str)
    } else if(pToken->type == CS64_INI_TOKEN_END) {
        /* NOP */
    }
    else {
        const static CS64INITokenType expected_tokens[] = {CS64_INI_TOKEN_SECTION_START, CS64_INI_TOKEN_VALUE, CS64_INI_TOKEN_COMMENT, CS64_INI_TOKEN_END};
        RETURN_EXPECTED_TOKEN_ERROR(result, expected_tokens)
    }

    return result;
}

#undef COPY_TOKEN_TO_STRING_BUFFER
#undef RETURN_EXPECTED_TOKEN_ERROR
#undef RETURN_IF_WRONG_TOKEN
#undef RETURN_IF_DATA_ERROR
#undef ADD_OR_ERROR_IF_COMMENT_PRESENT
#undef ADD_SECTION_OR_ERROR
#undef ADD_VARIABLE_OR_ERROR

#endif /* CS64_INI_LIBRARY_IMP */
