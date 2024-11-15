#include <stddef.h>
#include <stdint.h>

#define CS64_INI_LIBRARY_IMP
#include "cs64_ini.h"

#include <stdio.h>
#include <string.h>

int invalid_ascii_test();
int reencoding_test();
void print_bytes(const char* const name, CS64UTF8 *pUTF8Data);

int main() {
    int testResult = reencoding_test();
    if(testResult != 0)
        return testResult;

    testResult = invalid_ascii_test();
    if(testResult != 0)
        return testResult;
    return testResult;
}

int invalid_ascii_test() {
    CS64UTF8 ascii_byte;
    CS64Size characterByteSize = 1;
    CS64UniChar c;

    c = 128;
    while(c < 0x100) {
        ascii_byte = c;

        CS64UniChar character = cs64_ini_ascii_read(&ascii_byte, sizeof(ascii_byte), &characterByteSize);

        if(character != CS64_INI_BAD_NOT_ASCII) {
            printf("Invalid ASCII: cs64_ini_ascii_read failed to produce CS64_INI_BAD_NOT_ASCII value 0x%x produced 0x%x\n", CS64_INI_BAD_NOT_ASCII, character);
            printf("Input: 0x%02x", ascii_byte);

            return 1;
        }
        else if(characterByteSize != 0) {
            printf("Invalid ASCII: characterByteSize 0x%x should be 0x%x\n", characterByteSize, 0);
            printf("Input: 0x%02x", ascii_byte);

            return 2;
        }
        c++;
    }
}

int reencoding_test() {
    CS64UTF8 utf8_data[8];
    CS64Size characterByteSize = 1;
    CS64UniChar c = 0;

    // Reencoding ASCII Test.
    c = 0;
    while(c < 128) {
        memset(utf8_data, 0, sizeof(utf8_data) / sizeof(utf8_data[0]));

        int writeResult = cs64_ini_utf_8_write(utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]), c);

        if(writeResult <= 0) {
            printf("Reencoding ASCII: cs64_ini_utf_8_write failed for unicode char 0x%x with error code 0x%x\n", c, writeResult);

            return 1;
        }
        else if(writeResult != 1) {
            printf("Reencoding ASCII: cs64_ini_utf_8_write failed got length %i not 1!\n", writeResult);

            return 2;
        }

        CS64UniChar character = cs64_ini_ascii_read(utf8_data, writeResult, &characterByteSize);

        if(character != c) {
            printf("Reencoding ASCII: cs64_ini_ascii_read failed for unicode char 0x%x produced 0x%x\n", c, character);
            print_bytes("Bytes", utf8_data);

            return 3;
        }
        c++;
    }

    // Reencoding UTF-8 Test.
    c = 0;
    while(c < CS64_INI_MAX_CODE + 1) {
        memset(utf8_data, 0, sizeof(utf8_data) / sizeof(utf8_data[0]));

        int writeResult = cs64_ini_utf_8_write(utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]), c);

        if(writeResult <= 0) {
            printf("Reencoding UTF-8: cs64_ini_utf_8_write failed for unicode char 0x%x with error code 0x%x\n", c, writeResult);

            return 4;
        }
        else if(c < 0x80) {
            if(writeResult != 1) {
                printf("Reencoding UTF-8: cs64_ini_utf_8_write expected 1 for character 0x%x but got length of %i\n", c, writeResult);
                return 5;
            }
        }
        else if(c < 0x800) {
            if(writeResult != 2) {
                printf("Reencoding UTF-8: cs64_ini_utf_8_write expected 2 for character 0x%x but got length of %i\n", c, writeResult);
                return 6;
            }
        }
        else if(c < 0x10000) {
            if(writeResult != 3) {
                printf("Reencoding UTF-8: cs64_ini_utf_8_write expected 3 for character 0x%x but got length of %i\n", c, writeResult);
                return 7;
            }
        }
        else if(c < 0x110000) {
            if(writeResult != 4) {
                printf("Reencoding UTF-8: cs64_ini_utf_8_write expected 4 for character 0x%x but got length of %i\n", c, writeResult);
                return 8;
            }
        }

        CS64UniChar character = cs64_ini_utf_8_read(utf8_data, writeResult, &characterByteSize);

        if(character != c) {
            printf("Reencoding UTF-8: cs64_ini_utf_8_read failed for unicode char 0x%x produced 0x%x\n", c, character);
            print_bytes("Bytes", utf8_data);

            return 5;
        }
        c++;
    }

    return 0;
}

void print_bytes(const char* const pName, CS64UTF8 *pUTF8Data) {
    int i = 0;

    printf("%s: 0x", pName);

    while(pUTF8Data[i] != 0) {
        printf("%02x", pUTF8Data[i]);
        i++;
    }

    printf("\n");
}
