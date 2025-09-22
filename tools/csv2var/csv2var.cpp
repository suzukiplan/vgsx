#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>

std::vector<int> binary;

void put_usage()
{
    puts("usage: csv2var input.csv [u8|u16]");
}

int main(int argc, char* argv[])
{
    char buf[65536];
    FILE* in;

    if (argc < 2) {
        put_usage();
        return 1;
    }
    int isU8 = 1;
    if (3 <= argc) {
        if (0 == strcasecmp("u8", argv[2])) {
            isU8 = 1;
        } else if (0 == strcasecmp("u16", argv[2])) {
            isU8 = 0;
        } else {
            put_usage();
            return 1;
        }
    }

    if (NULL == (in = fopen(argv[1], "rt"))) {
        puts("cannot open csv file");
        return 1;
    }

    while (fgets(buf, sizeof(buf), in)) {
        char* cp = buf;
        while (*cp) {
            while (*cp && !isdigit(*cp)) cp++;
            if (!*cp) break;
            binary.push_back(atoi(cp));
            while (isdigit(*cp)) cp++;
        }
    }
    fclose(in);

    char varName[1024];
    char* cp = strrchr(argv[1], '/');
    if (cp) {
        strcpy(varName, cp + 1);
    } else {
        cp = strrchr(argv[1], '\\');
        if (cp) {
            strcpy(varName, cp + 1);
        } else {
            strcpy(varName, argv[1]);
        }
    }
    cp = strchr(varName, '.');
    if (cp) *cp = 0;
    int size = (int)binary.size();
    int offset = 0;
    printf("const unsigned %s csv_%s[%d] = {\n", isU8 ? "char" : "short", varName, size);
    bool firstLine = true;
    while (1) {
        unsigned char buf[16];
        if (size <= offset) {
            printf("\n");
            break;
        }
        if (firstLine) {
            firstLine = false;
        } else {
            printf(",\n");
        }
        printf("    ");
        int readMax = isU8 ? 16 : 8;
        int readSize = size - offset < readMax ? size - offset : readMax;
        for (int i = 0; i < readSize; i++) {
            if (i) {
                if (isU8) {
                    printf(", 0x%02X", (unsigned char)binary[offset++]);
                } else {
                    printf(", 0x%04X", (unsigned short)binary[offset++]);
                }
            } else {
                if (isU8) {
                    printf("0x%02X", (unsigned char)binary[offset++]);
                } else {
                    printf("0x%04X", (unsigned short)binary[offset++]);
                }
            }
        }
    }
    printf("};\n");

    return 0;
}