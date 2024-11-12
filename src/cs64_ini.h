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

CS64UniChar cs64_ini_ascii_read(CS64UTF8 *pData, CS64Size *pSize);
CS64UniChar cs64_ini_utf_8_read(CS64UTF8 *pData, CS64Size *pSize);

#endif // CS64_INI_LIBRARY_H

#ifdef CS64_INI_LIBRARY_IMP

CS64UniChar cs64_ini_ascii_read(CS64UTF8 *pData, CS64Size *pSize) {
    if(*pData == '\0') {
        *pSize = 0;
        return 0;
    } else if(*pData >= 0b10000000) {
        *pSize = 0;
        return CS64_INI_BAD_BYTE;
    }

    *pSize = 1;
    return *pData;
}
// CS64UniChar cs64_ini_utf_8_read(CS64UTF8 *pData, CS64Size *pSize);

#endif
