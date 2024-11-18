#include <stddef.h>
#include <stdint.h>

#define CS64_INI_LIBRARY_IMP
#include "cs64_ini.h"

#include <stdio.h>
#include <string.h>

int invalid_ascii_test();
int invalid_utf8_byte_test();
int invalid_utf8_invalid_continuous_test();
int invalid_utf8_overlong_test();
int invalid_utf8_oversize_test();
int reencoding_test();
void print_bytes(const char* const name, CS64UTF8 *pUTF8Data, int size);
int utf8_verify_test();

int main() {
    int testResult;

    testResult = reencoding_test();
    if(testResult != 0)
        return testResult;

    testResult = utf8_verify_test();
    if(testResult != 0)
        return testResult;

    testResult = invalid_utf8_byte_test();
    if(testResult != 0)
        return testResult;

    testResult = invalid_utf8_invalid_continuous_test();
    if(testResult != 0)
        return testResult;

    testResult = invalid_utf8_overlong_test();
    if(testResult != 0)
        return testResult;

    testResult = invalid_utf8_oversize_test();
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

int invalid_utf8_byte_test() {
    CS64UTF8 invalidUTF8values[] = {0xc0, 0xc1, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
    CS64Size characterByteSize = 1;
    CS64UniChar character;
    int i;

    // Single byte case.
    i = 0;
    while(i < sizeof(invalidUTF8values) / sizeof(invalidUTF8values[0])) {
        character = cs64_ini_utf_8_read(&invalidUTF8values[i], 1, &characterByteSize);

        if(character != CS64_INI_BAD_NOT_UTF_8 || characterByteSize != 0) {
            printf("Invalid Unicode Test: cs64_ini_utf_8_read code 0x%x failed to produce CS64_INI_BAD_NOT_UTF_8, but instead produced 0x%x. Byte size 0x%x\n", invalidUTF8values[i], character, characterByteSize);

            return 1;
        }
        i++;
    }

    CS64UTF8 utf8_data[8];
    int length;
    int m;

    // Two byte case.
    memset(utf8_data, 0, sizeof(utf8_data) / sizeof(utf8_data[0]));
    length = cs64_ini_utf_8_write(utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]), 0x80);

    if(length != 2) {
        printf("Invalid Unicode Test Two Byte Case: cs64_ini_utf_8_write length %i. Expected 2\n", length);
        return 2;
    }

    i = 0;
    while(i < sizeof(invalidUTF8values) / sizeof(invalidUTF8values[0])) {
        utf8_data[1] = invalidUTF8values[i];

        character = cs64_ini_utf_8_read(utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]), &characterByteSize);
        if(character <= CS64_INI_MAX_CODE || characterByteSize != 0) {
            printf("Invalid Unicode Test Two Byte Case: cs64_ini_utf_8_read code 0x%x failed to produce CS64_INI_BAD_NOT_UTF_8, but instead produced 0x%x. Byte size 0x%x\n", invalidUTF8values[i], character, characterByteSize);
            print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));

            return 3;
        }
        i++;
    }

    // Three byte case.
    m = 1;
    while(m < 3) {
        memset(utf8_data, 0, sizeof(utf8_data) / sizeof(utf8_data[0]));
        length = cs64_ini_utf_8_write(utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]), 0x800);

        if(length != 3) {
            printf("Invalid Unicode Test Three Byte Case (%i): cs64_ini_utf_8_write length %i. Expected 3\n", m, length);
            return 4;
        }
        i = 0;
        while(i < sizeof(invalidUTF8values) / sizeof(invalidUTF8values[0])) {
            utf8_data[m] = invalidUTF8values[i];

            character = cs64_ini_utf_8_read(utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]), &characterByteSize);
            if(character <= CS64_INI_MAX_CODE || characterByteSize != 0) {
                printf("Invalid Unicode Test Three Byte Case: cs64_ini_utf_8_read code 0x%x failed to produce CS64_INI_BAD_NOT_UTF_8, but instead produced 0x%x. Byte size 0x%x\n", invalidUTF8values[i], character, characterByteSize);
                print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));

                return 5;
            }
            i++;
        }
        m++;
    }

    // Four byte case.
    m = 1;
    while(m < 4) {
        memset(utf8_data, 0, sizeof(utf8_data) / sizeof(utf8_data[0]));
        length = cs64_ini_utf_8_write(utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]), 0x10000);

        if(length != 4) {
            printf("Invalid Unicode Test Four Byte Case (%i): cs64_ini_utf_8_write length %i. Expected 4\n", m, length);
            return 6;
        }
        i = 0;
        while(i < sizeof(invalidUTF8values) / sizeof(invalidUTF8values[0])) {
            utf8_data[m] = invalidUTF8values[i];

            character = cs64_ini_utf_8_read(utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]), &characterByteSize);
            if(character <= CS64_INI_MAX_CODE || characterByteSize != 0) {
                printf("Invalid Unicode Test Four Byte Case (%i): cs64_ini_utf_8_read code 0x%x failed to produce CS64_INI_BAD_NOT_UTF_8, but instead produced 0x%x. Byte size 0x%x\n", characterByteSize, m, invalidUTF8values[i], character);
                print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));

                return 7;
            }
            i++;
        }
        m++;
    }

    return 0;
}

int invalid_utf8_invalid_continuous_test() {
    CS64UTF8 utf8_data[8] = {0};
    CS64Size characterByteSize;
    CS64UniChar character;
    int arrayIndexes[4];

    // Continuous bytes start.
    arrayIndexes[0] = 0;
    while(arrayIndexes[0] < 64) {
        utf8_data[0] = 0b10000000 | arrayIndexes[0]; // Valid Continuous Bytes

        character = cs64_ini_utf_8_read(utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]), &characterByteSize);

        if(character != CS64_INI_BAD_CONTINUE_BYTE_0 || characterByteSize != 0) {
            printf("Invalid Utf8 Invalid Continuous Test Start of character case: cs64_ini_utf_8_read code 0x%x is not CS64_INI_BAD_CONTINUE_BYTE_0, but instead produced 0x%x. Byte size 0x%x\n", utf8_data[0], character, characterByteSize);
            print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));

            return 1;
        }

        arrayIndexes[0]++;
    }

#define INVALID_CONITINOUS(ARRAY_INDEX, UTF_8_INDEX, NAME, VALUE) \
    arrayIndexes[ARRAY_INDEX] = 0; \
    while(arrayIndexes[ARRAY_INDEX] < 64) {\
        utf8_data[UTF_8_INDEX] = 0b00000000 | arrayIndexes[ARRAY_INDEX]; /* Invalid Continuous bytes with 0b00XXXXXX */\
\
        character = cs64_ini_utf_8_read(utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]), &characterByteSize);\
\
        if((character != VALUE && character != CS64_INI_BAD_NOT_UTF_8) || characterByteSize != 0) {\
            printf("Invalid UTF-8 Invalid Continuous " #NAME " 0b00XXXXXX: cs64_ini_utf_8_read did not produce " #VALUE " or CS64_INI_BAD_NOT_UTF_8, but instead produced 0x%x. Byte size 0x%x\n", character, characterByteSize);\
            print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));\
\
            return 2;\
        }\
\
        utf8_data[UTF_8_INDEX] = 0b01000000 | arrayIndexes[ARRAY_INDEX]; /* Invalid Continuous bytes with 0b01XXXXXX */\
\
        character = cs64_ini_utf_8_read(utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]), &characterByteSize);\
\
        if((character != VALUE && character != CS64_INI_BAD_NOT_UTF_8) || characterByteSize != 0) {\
            printf("Invalid UTF-8 Invalid Continuous " #NAME " 0b01XXXXXX: cs64_ini_utf_8_read did not produce " #VALUE " or CS64_INI_BAD_NOT_UTF_8, but instead produced 0x%x. Byte size 0x%x\n", character, characterByteSize);\
            print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));\
\
            return 3;\
        }\
\
        utf8_data[UTF_8_INDEX] = 0b11000000 | arrayIndexes[ARRAY_INDEX]; /* Invalid Continuous bytes with 0b11XXXXXX */\
\
        character = cs64_ini_utf_8_read(utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]), &characterByteSize);\
\
        if((character != VALUE && character != CS64_INI_BAD_NOT_UTF_8) || characterByteSize != 0) {\
            printf("Invalid UTF-8 Invalid Continuous " #NAME " 0b11XXXXXX: cs64_ini_utf_8_read did not produce " #VALUE " or CS64_INI_BAD_NOT_UTF_8, but instead produced 0x%x. Byte size 0x%x\n", character, characterByteSize);\
            print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));\
\
            return 4;\
        }\
        arrayIndexes[ARRAY_INDEX]++;\
    }

    // 2 Byte Case
    arrayIndexes[0] = 0;
    while(arrayIndexes[0] < 32) {
        utf8_data[0] = 0b11000000 | arrayIndexes[0]; // Valid Continuous Bytes

        INVALID_CONITINOUS(1, 1, 2 Byte, CS64_INI_BAD_CONTINUE_BYTE_1)

        arrayIndexes[0]++;
    }

    // 3 Byte Case
    arrayIndexes[0] = 0;
    while(arrayIndexes[0] < 16) {
        utf8_data[0] = 0b11100000 | arrayIndexes[0]; // Valid Continuous Bytes

        arrayIndexes[1] = 0;
        while(arrayIndexes[1] < 64) {
            utf8_data[2] = 0b10000000 | arrayIndexes[1];
            INVALID_CONITINOUS(2, 1, 3 Bytes Bad Middle Byte, CS64_INI_BAD_CONTINUE_BYTE_1)
            arrayIndexes[1]++;
        }

        arrayIndexes[1] = 0;
        while(arrayIndexes[1] < 64) {
            utf8_data[1] = 0b10000000 | arrayIndexes[1];
            INVALID_CONITINOUS(2, 2, 3 Bytes Bad Last Byte, CS64_INI_BAD_CONTINUE_BYTE_2)
            arrayIndexes[1]++;
        }

        arrayIndexes[0]++;
    }

    // 4 Byte Case. I know it is rather slow code, but I decided to test almost every bad continious combo.
    arrayIndexes[0] = 0;
    while(arrayIndexes[0] < 5) {
        utf8_data[0] = 0b11110000 | arrayIndexes[0]; // Valid Continuous Bytes

        arrayIndexes[1] = 0;
        while(arrayIndexes[1] < 64) {
            utf8_data[2] = 0b10000000 | arrayIndexes[1];

            arrayIndexes[2] = 0;
            while(arrayIndexes[2] < 64) {
                utf8_data[3] = 0b10000000 | arrayIndexes[2];
                INVALID_CONITINOUS(3, 1, 4 Bytes Bad 2nd Byte, CS64_INI_BAD_CONTINUE_BYTE_1)
                arrayIndexes[2]++;
            }
            arrayIndexes[1]++;
        }

        arrayIndexes[1] = 0;
        while(arrayIndexes[1] < 64) {
            utf8_data[1] = 0b10000000 | arrayIndexes[1];

            arrayIndexes[2] = 0;
            while(arrayIndexes[2] < 64) {
                utf8_data[3] = 0b10000000 | arrayIndexes[2];
                INVALID_CONITINOUS(3, 2, 4 Bytes Bad 3rd Byte, CS64_INI_BAD_CONTINUE_BYTE_2)
                arrayIndexes[2]++;
            }
            arrayIndexes[1]++;
        }

        arrayIndexes[1] = 0;
        while(arrayIndexes[1] < 64) {
            utf8_data[1] = 0b10000000 | arrayIndexes[1];

            arrayIndexes[2] = 0;
            while(arrayIndexes[2] < 64) {
                utf8_data[2] = 0b10000000 | arrayIndexes[2];
                INVALID_CONITINOUS(3, 3, 4 Bytes Bad 4th Byte, CS64_INI_BAD_CONTINUE_BYTE_3)
                arrayIndexes[2]++;
            }
            arrayIndexes[1]++;
        }

        arrayIndexes[0]++;
    }

    return 0;
}

int invalid_utf8_overlong_test() {
    CS64UTF8 utf8_data[8] = {0};
    CS64Size characterByteSize = 0;
    CS64UniChar number;

    // Two byte case.
    number = 0;
    while(number <= 0x80 ) {
        utf8_data[1] = 0b10000000 | (0b00111111 & (number >> 0));
        utf8_data[0] = 0b11000000 | (0b00011111 & (number >> 6));

        CS64UniChar character = cs64_ini_utf_8_read(utf8_data, 2, &characterByteSize);

        if(number != 0x80) {
            if(character != CS64_INI_BAD_OVERLONG && character != CS64_INI_BAD_NOT_UTF_8 || characterByteSize != 0) {
                printf("Invalid UTF-8 Overlong Two Byte: cs64_ini_utf_8_read failed for unicode char 0x%x produced 0x%x with length %i\n", number, character, characterByteSize);
                print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));
                return 1;
            }
        }
        else if(character != 0x80 || characterByteSize != 2) {
            printf("Invalid UTF-8 Overlong Two Byte Correct Case: cs64_ini_utf_8_read failed for unicode char 0x%x produced 0x%x with length %i\n", number, character, characterByteSize);
            print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));
            return 2;
        }

        number++;
    }

    // Three byte case.
    number = 0;
    while(number <= 0x800 ) {
        utf8_data[2] = 0b10000000 | (0b00111111 & (number >>  0));
        utf8_data[1] = 0b10000000 | (0b00111111 & (number >>  6));
        utf8_data[0] = 0b11100000 | (0b00001111 & (number >> 12));

        CS64UniChar character = cs64_ini_utf_8_read(utf8_data, 3, &characterByteSize);

        if(number != 0x800) {
            if(character != CS64_INI_BAD_OVERLONG && character != CS64_INI_BAD_NOT_UTF_8 || characterByteSize != 0) {
                printf("Invalid UTF-8 Overlong Three Byte: cs64_ini_utf_8_read failed for unicode char 0x%x produced 0x%x with length %i\n", number, character, characterByteSize);
                print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));
                return 3;
            }
        }
        else if(character != 0x800 || characterByteSize != 3) {
            printf("Invalid UTF-8 Overlong Three Byte Correct Case: cs64_ini_utf_8_read failed for unicode char 0x%x produced 0x%x with length %i\n", number, character, characterByteSize);
            print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));
            return 4;
        }

        number++;
    }

    // Four byte case.
    number = 0;
    while(number <= 0x10000 ) {
        utf8_data[3] = 0b10000000 | (0b00111111 & (number >>  0));
        utf8_data[2] = 0b10000000 | (0b00111111 & (number >>  6));
        utf8_data[1] = 0b10000000 | (0b00111111 & (number >> 12));
        utf8_data[0] = 0b11110000 | (0b00000111 & (number >> 18));

        CS64UniChar character = cs64_ini_utf_8_read(utf8_data, 4, &characterByteSize);

        if(number != 0x10000) {
            if(character != CS64_INI_BAD_OVERLONG && character != CS64_INI_BAD_NOT_UTF_8 || characterByteSize != 0) {
                printf("Invalid UTF-8 Overlong Four Byte: cs64_ini_utf_8_read failed for unicode char 0x%x produced 0x%x with length %i\n", number, character, characterByteSize);
                print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));
                return 5;
            }
        }
        else if(character != 0x10000 || characterByteSize != 4) {
            printf("Invalid UTF-8 Overlong Four Byte Correct Case: cs64_ini_utf_8_read failed for unicode char 0x%x produced 0x%x with length %i\n", number, character, characterByteSize);
            print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));
            return 6;
        }

        number++;
    }

    return 0;
}

int invalid_utf8_oversize_test() {
    CS64UTF8 utf8_data[8] = {0};
    CS64Size characterByteSize = 0;
    CS64UniChar number = 0x10ffff;

    while(number < 0x200000 ) {
        utf8_data[3] = 0b10000000 | (0b00111111 & (number >>  0));
        utf8_data[2] = 0b10000000 | (0b00111111 & (number >>  6));
        utf8_data[1] = 0b10000000 | (0b00111111 & (number >> 12));
        utf8_data[0] = 0b11110000 | (0b00000111 & (number >> 18));

        CS64UniChar character = cs64_ini_utf_8_read(utf8_data, 4, &characterByteSize);

        if(number != 0x10ffff) {
            if(character != CS64_INI_BAD_TOO_BIG && character != CS64_INI_BAD_NOT_UTF_8 || characterByteSize != 0) {
                printf("Invalid UTF-8 Oversize: cs64_ini_utf_8_read failed for unicode char 0x%x produced 0x%x with length %i\n", number, character, characterByteSize);
                print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));
                return 1;
            }
        }
        else if(character != 0x10ffff || characterByteSize != 4) {
            printf("Invalid UTF-8 Oversize Correct Case: cs64_ini_utf_8_read failed for unicode char 0x%x produced 0x%x with length %i\n", number, character, characterByteSize);
            print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));
            return 2;
        }

        number++;
    }

    return 0;
}

int reencoding_test() {
    CS64UTF8 utf8_data[8];
    CS64Size characterByteSize = 1;
    CS64UniChar c;

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
            print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));

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
            print_bytes("Bytes", utf8_data, sizeof(utf8_data) / sizeof(utf8_data[0]));

            return 9;
        }
        c++;
    }

    return 0;
}

void print_bytes(const char* const pName, CS64UTF8 *pUTF8Data, int size) {
    int i = 0;

    printf("%s: 0x", pName);

    while(i < size) {
        printf("%02x", pUTF8Data[i]);
        i++;
    }

    printf("\n");
}

struct {
    CS64UniChar unicodePoint;
    unsigned    length;
    CS64UTF8    utf8Buffer[4];
} expectedValues[] = {
    {0x000001, 1, {0x01, 0x00, 0x00, 0x00}},
    {0x000002, 1, {0x02, 0x00, 0x00, 0x00}},
    {0x000004, 1, {0x04, 0x00, 0x00, 0x00}},
    {0x000008, 1, {0x08, 0x00, 0x00, 0x00}},
    {0x000010, 1, {0x10, 0x00, 0x00, 0x00}},
    {0x000020, 1, {0x20, 0x00, 0x00, 0x00}},
    {0x000040, 1, {0x40, 0x00, 0x00, 0x00}},
    {0x000080, 2, {0xc2, 0x80, 0x00, 0x00}},
    {0x000100, 2, {0xc4, 0x80, 0x00, 0x00}},
    {0x000200, 2, {0xc8, 0x80, 0x00, 0x00}},
    {0x000400, 2, {0xd0, 0x80, 0x00, 0x00}},
    {0x000800, 3, {0xe0, 0xa0, 0x80, 0x00}},
    {0x001000, 3, {0xe1, 0x80, 0x80, 0x00}},
    {0x002000, 3, {0xe2, 0x80, 0x80, 0x00}},
    {0x004000, 3, {0xe4, 0x80, 0x80, 0x00}},
    {0x008000, 3, {0xe8, 0x80, 0x80, 0x00}},
    {0x010000, 4, {0xf0, 0x90, 0x80, 0x80}},
    {0x020000, 4, {0xf0, 0xa0, 0x80, 0x80}},
    {0x040000, 4, {0xf1, 0x80, 0x80, 0x80}},
    {0x080000, 4, {0xf2, 0x80, 0x80, 0x80}},
    {0x100000, 4, {0xf4, 0x80, 0x80, 0x80}}
};

int utf8_verify_test() {
    CS64UTF8 utf8_data[8] = {0};
    CS64Size length;
    unsigned index;

    index = 0;
    while(index < sizeof(expectedValues) / sizeof(expectedValues[0])) {
        CS64UniChar character = cs64_ini_utf_8_read(expectedValues[index].utf8Buffer, expectedValues[index].length, &length);

        if(character != expectedValues[index].unicodePoint || expectedValues[index].length != length) {
            printf("UTF-8 Verification Problem: cs64_ini_utf_8_read did not decode 0x%x character with length %i properly. Instead it got 0x%x with length %i\n", character, length, expectedValues[index].unicodePoint, expectedValues[index].length);

            return 1;
        }

        unsigned underMemIndex = 0;
        while(underMemIndex < length) {
            character = cs64_ini_utf_8_read(expectedValues[index].utf8Buffer, expectedValues[index].length - (1 + underMemIndex), &length);

            if(character != CS64_INI_BAD_LACK_SPACE || length != 0) {
                printf("UTF-8 Verification Size Check: cs64_ini_utf_8_read did not handle 0x%x character with length %i properly. Instead it got error code 0x%x with length %i\n", character, length, expectedValues[index].unicodePoint, expectedValues[index].length);

                return 2;
            }
            underMemIndex++;
        }

        length = cs64_ini_utf_8_write(utf8_data, expectedValues[index].length, expectedValues[index].unicodePoint);

        if( utf8_data[0] != expectedValues[index].utf8Buffer[0] ||
            utf8_data[1] != expectedValues[index].utf8Buffer[1] ||
            utf8_data[2] != expectedValues[index].utf8Buffer[2] ||
            utf8_data[3] != expectedValues[index].utf8Buffer[3] ||
            expectedValues[index].length != length)
        {
            printf("UTF-8 Verification Problem: cs64_ini_utf_8_write did not produce expected bytes with length of %i.\n", length, expectedValues[index].unicodePoint, expectedValues[index].length);
            print_bytes("Expected Bytes", expectedValues[index].utf8Buffer, expectedValues[index].length);
            print_bytes("Decoded  Bytes", utf8_data, length);

            return 3;
        }

        underMemIndex = 0;
        while(underMemIndex < length) {
            length = cs64_ini_utf_8_write(utf8_data, expectedValues[index].length - (1 + underMemIndex), expectedValues[index].unicodePoint);

            if(length != 0) {
                printf("UTF-8 Verification Size Check: cs64_ini_utf_8_write did not handle 0x%x character with length %i properly. It got a length of %i\n",  expectedValues[index].unicodePoint, expectedValues[index].length - (1 + underMemIndex), length);

                return 4;
            }
            underMemIndex++;
        }

        index++;
    }

    return 0;
}
