#ifndef CS64_INI_LIBRARY_H
#define CS64_INI_LIBRARY_H

typedef uint8_t  CS64UTF8;
typedef uint32_t CS64UniChar;
typedef size_t   CS64Size;

typedef enum {
    CS64_INI_MAX_CODE       = 0x10ffff,
    CS64_INI_BAD_BYTE       = 0x110000,
    CS64_INI_BAD_CONTINUE   = 0x110001,
    CS64_INI_BAD_OVERLONG   = 0x110002,
    CS64_INI_BAD_EARLY_NULL = 0x110003
} CS64UniCharCode;

CS64UniChar cs64_ini_ascii_read(const CS64UTF8 *const pData, CS64Size *pSize);
CS64UniChar cs64_ini_utf_8_read(const CS64UTF8 *const pData, CS64Size *pSize);

/**
 * This function implements the encoding scheme for converting Unicode characters into UTF-8 byte sequences. UTF-8 uses a variable number of bytes to represent different characters: 1 byte for ASCII, 2 bytes for most common extended characters, and up to 4 bytes for rare Unicode characters.
 * @param pDataHead A pointer to an array of bytes where the UTF-8 representation will be written.
 * @param remainingDataSize The size (in bytes) of the buffer pointed to by pDataHead. This is used to prevent writing beyond the allocated memory.
 * @param character The Unicode character to be converted to UTF-8.
 * @return  The function returns the number of bytes written to the buffer, indicating the length of the UTF-8 representation. If a character is too big for the buffer then return 0. If the character is out of unicode range then -1.
 */
inline int cs64_ini_utf_8_write(CS64UTF8 *pDataHead, CS64Size remainingDataSize, CS64UniChar character);

#endif // CS64_INI_LIBRARY_H

#ifdef CS64_INI_LIBRARY_IMP

CS64UniChar cs64_ini_ascii_read(CS64UTF8 *pData, CS64Size *pSize) {
    if(*pData == '\0') {
        *pSize = 0;
        return 0;
    } else if(*pData >= 0x80) {
        *pSize = 0;
        return CS64_INI_BAD_BYTE;
    }

    *pSize = 1;
    return *pData;
}
// CS64UniChar cs64_ini_utf_8_read(CS64UTF8 *pData, CS64Size *pSize);

inline int cs64_ini_utf_8_write(CS64UTF8 *pDataHead, CS64Size remainingDataSize, CS64UniChar character) {
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

#endif
