#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void put_usage(void)
{
    fprintf(stderr, "bin2var /path/to/binary.rom [u8|u16|u16l|u16b]\n");
}

uint16_t tobe(uint16_t n)
{
    uint8_t w[2];
    memcpy(w, &n, 2);
    uint8_t ww = w[0];
    w[0] = w[1];
    w[1] = ww;
    memcpy(&n, w, 2);
    return n;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        put_usage();
        return -1;
    }
    bool isU8 = true;
    bool isLE = true;
    if (3 <= argc) {
        if (0 == strcasecmp(argv[2], "u8")) {
            isU8 = true;
        } else if (0 == strcasecmp(argv[2], "u16") || 0 == strcasecmp(argv[2], "u16l")) {
            isU8 = false;
            isLE = true;
        } else if (0 == strcasecmp(argv[2], "u16b")) {
            isU8 = false;
            isLE = false;
        } else {
            put_usage();
            return -1;
        }
    }
    FILE* fp = fopen(argv[1], "rb");
    if (!fp) {
        fprintf(stderr, "file open error\n");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int size = (int)ftell(fp);
    fseek(fp, 0, SEEK_SET);
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
    printf("#include <vgs.h>\n\n");
    if (isU8) {
        printf("const uint8_t rom_%s[%d] = {\n", varName, size);
        bool firstLine = true;
        while (1) {
            unsigned char buf[16];
            int readSize = (int)fread(buf, 1, sizeof(buf), fp);
            if (readSize < 1) {
                printf("\n");
                break;
            }
            if (firstLine) {
                firstLine = false;
            } else {
                printf(",\n");
            }
            printf("    ");
            for (int i = 0; i < readSize; i++) {
                if (i) {
                    printf(", 0x%02X", buf[i]);
                } else {
                    printf("0x%02X", buf[i]);
                }
            }
        }
    } else {
        printf("const uint16_t rom_%s[%d] = {\n", varName, size / 2);
        bool firstLine = true;
        while (1) {
            unsigned short buf[8];
            int readSize = (int)fread(buf, 1, sizeof(buf), fp);
            readSize /= 2;
            if (readSize < 1) {
                printf("\n");
                break;
            }
            if (firstLine) {
                firstLine = false;
            } else {
                printf(",\n");
            }
            printf("    ");
            for (int i = 0; i < readSize; i++) {
                if (i) {
                    printf(", 0x%04X", isLE ? buf[i] : tobe(buf[i]));
                } else {
                    printf("0x%04X", isLE ? buf[i] : tobe(buf[i]));
                }
            }
        }
    }
    printf("};\n");
    fclose(fp);
    return 0;
}
