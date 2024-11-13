#include <stddef.h>
#include <stdint.h>

#define CS64_INI_LIBRARY_IMP
#include "cs64_ini.h"

#include <stdio.h>
#include <string.h>

int main() {
    CS64UTF8 utf8_data[] = "Hello";
    CS64Size utf8_data_size = strlen(utf8_data);

    CS64UTF8 *pUTF8_data_head = utf8_data;

    CS64Size characterByteSize = 1;
    CS64UniChar character;

    while(1) {
        character = cs64_ini_ascii_read(pUTF8_data_head, utf8_data_size, &characterByteSize);

        if(characterByteSize == 0) {
            if(character == CS64_INI_BAD_NOT_ASCII)
                printf("\nError: Encountered non-ASCII character\n");

            break; // Loop terminates when characterByteSize is 0.
        } else
            printf("%c", character);

        pUTF8_data_head += characterByteSize;
        utf8_data_size  -= characterByteSize;
    }
    printf("\nutf8_data_size = %i\n", utf8_data_size);
    return 0;
}
