#ifndef CS64_INI_LIBRARY_H
#define CS64_INI_LIBRARY_H

typedef uint8_t  CS64UTF8;
typedef uint32_t CS64UniChar;
typedef size_t   CS64Size;

typedef enum {
    CS64_INI_MAX_CODE       = 0x10ffff,
    CS64_INI_BAD_NOT_ASCII  = 0x110000,
    CS64_INI_BAD_CONTINUE   = 0x110001,
    CS64_INI_BAD_OVERLONG   = 0x110002,
    CS64_INI_BAD_EARLY_NULL = 0x110003
} CS64UniCharCode;

/**
 * This function reads an ASCII value.
 * @warning UTF-8 characters need to stay within the ASCII range.
 * @param pDataHead Pointer to the current byte in the UTF-8 stream.
 * @param pCharacterByteSize Pointer to a variable storing the read character size. If zero then either NULL has been found or error. (updated by the function)
 * @return 0-127 if there is no error. CS64_INI_BAD_NOT_ASCII is the stream is not ASCII.
 */
CS64UniChar cs64_ini_ascii_read(const CS64UTF8 *const pDataHead, CS64Size *pCharacterByteSize);

CS64UniChar cs64_ini_utf_8_read(const CS64UTF8 *const pDataHead, CS64Size *pCharacterByteSize);

/**
 * This function implements the encoding scheme for converting Unicode characters into UTF-8 byte sequences. UTF-8 uses a variable number of bytes to represent different characters: 1 byte for ASCII, 2 bytes for most common extended characters, and up to 4 bytes for rare Unicode characters.
 * @param pDataHead A pointer to an array of bytes where the UTF-8 representation will be written.
 * @param remainingDataSize The size (in bytes) of the buffer pointed to by pDataHead. This is used to prevent writing beyond the allocated memory.
 * @param character The Unicode character to be converted to UTF-8.
 * @return  The function returns the number of bytes written to the buffer, indicating the length of the UTF-8 representation. If a character is too big for the buffer then return 0. If the character is out of unicode range then -1.
 */
int cs64_ini_utf_8_write(CS64UTF8 *pDataHead, CS64Size remainingDataSize, CS64UniChar character);

#endif // CS64_INI_LIBRARY_H

#ifdef CS64_INI_LIBRARY_IMP

// ### TEXT HANDLING SECTION ###

CS64UniChar cs64_ini_ascii_read(const CS64UTF8 *const pDataHead, CS64Size *pCharacterByteSize) {
    if(*pDataHead >= 0x80 || *pDataHead == 0) {
        *pCharacterByteSize = 0; // Indicate that the loop calling this function should end.

        if(*pDataHead != 0)
            return CS64_INI_BAD_NOT_ASCII;

        return 0; // Encountering NULL means EOF.
    }

    *pCharacterByteSize = 1; // Indicate that there are more bytes to be read.
    return *pDataHead;
}

#define CHECK_NEVER_BYTE( byte ) (byte == 0xc1 || byte == 0xc2 || (byte >= 0xf5 && byte <= 0xff))

CS64UniChar cs64_ini_utf_8_read(const CS64UTF8 *const pDataHead, CS64Size *pCharacterByteSize) {
    CS64UniChar unicodeCharacter;

    return unicodeCharacter;
}

// Clean up Macros!
#undef CHECK_NEVER_BYTE

int cs64_ini_utf_8_write(CS64UTF8 *pDataHead, CS64Size remainingDataSize, CS64UniChar character) {
    if(character < 0x80) { // ASCII range. UTF-8 1 Byte Case.
        if(remainingDataSize < 1)
            return 0; // Not Enough Space

        pDataHead[0] = character;
        return 1; // 1 Byte used.
    }
    else if(character < 0x800) { // UTF-8 2 Byte Case.
        if(remainingDataSize < 2)
            return 0; // Not Enough Space

        pDataHead[1] = 0b10000000 | (0b00111111 & (character >> 0));
        pDataHead[0] = 0b11000000 | (0b00011111 & (character >> 6));
        return 2; // 2 Bytes used.
    }
    else if(character < 0x10000) { // UTF-8 3 Byte Case.
        if(remainingDataSize < 3)
            return 0; // Not Enough Space

        pDataHead[2] = 0b10000000 | (0b00111111 & (character >>  0));
        pDataHead[1] = 0b10000000 | (0b00111111 & (character >>  6));
        pDataHead[0] = 0b11100000 | (0b00001111 & (character >> 12));
        return 3; // 3 Bytes used.
    }
    else if(character < 0x110000) { // UTF-8 4 Byte Case.
        if(remainingDataSize < 4)
            return 0; // Not Enough Space

        pDataHead[3] = 0b10000000 | (0b00111111 & (character >>  0));
        pDataHead[2] = 0b10000000 | (0b00111111 & (character >>  6));
        pDataHead[1] = 0b10000000 | (0b00111111 & (character >> 12));
        pDataHead[0] = 0b11110000 | (0b00000111 & (character >> 18));
        return 4; // 4 Bytes used.
    }
    else
        return -1; // character is too big for this algorithm.
}

#endif // CS64_INI_LIBRARY_IMP
