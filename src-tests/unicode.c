#include <stddef.h>
#include <stdint.h>

#define CS64_INI_LIBRARY_IMP
#include "cs64_ini.h"

#include <stdio.h>
#include <string.h>

int reencoding_test();

int main() {
    int testResult = reencoding_test();

    return testResult;
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

        CS64UniChar character = cs64_ini_ascii_read(utf8_data, writeResult, &characterByteSize);

        if(character != c) {
            printf("Reencoding ASCII: cs64_ini_ascii_read failed for unicode char 0x%x produced 0x%x\n", c, character);
            printf("Bytes: 0x");

            int i = 0;

            while(utf8_data[i] != 0) {
                printf("%02x", utf8_data[i]);
                i++;
            }

            printf("\n");

            return 2;
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

            return 3;
        }

        CS64UniChar character = cs64_ini_utf_8_read(utf8_data, writeResult, &characterByteSize);

        if(character != c) {
            printf("Reencoding UTF-8: cs64_ini_utf_8_read failed for unicode char 0x%x produced 0x%x\n", c, character);
            printf("Bytes: 0x");

            int i = 0;

            while(utf8_data[i] != 0) {
                printf("%02x", utf8_data[i]);
                i++;
            }

            printf("\n");

            return 4;
        }
        c++;
    }

    return 0;
}
