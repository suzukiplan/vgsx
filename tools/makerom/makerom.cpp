#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <vector>

static uint8_t* loadBinary(const char* path, int* size)
{
    try {
        std::ifstream ifs(path, std::ios::binary);
        ifs.seekg(0, std::ios::end);
        *size = (int)ifs.tellg();
        ifs.seekg(0);
        uint8_t* program = new uint8_t[*size];
        ifs.read((char*)program, *size);
        return program;
    } catch (...) {
        return nullptr;
    }
}

class Data
{
  public:
    char name[4];
    void* ptr;
    int size;

    Data(const char* name, void* ptr, int size)
    {
        memset(this->name, 0, sizeof(this->name));
        strcpy(this->name, name);
        this->ptr = ptr;
        this->size = size;
    }
};

static std::vector<Data*> _data;

static void put_usage(void)
{
    puts("usage: makerom  -o /path/to/output.rom");
    puts("                -e /path/to/program.elf");
    puts("               [-c /path/to/palette.bin]");
    puts("               [-g /path/to/pattern.chr ...]");
    puts("               [-b /path/to/bgm.vgm ...]");
    puts("               [-s /path/to/sfx.wav ...]");
    exit(1);
}

int main(int argc, char* argv[])
{
    const char* outputPath = nullptr;
    bool programSpecified = false;

    for (int i = 1; i < argc; i++) {
        if ('-' == argv[i][0]) {
            switch (tolower(argv[i][1])) {
                case 'g': {
                    if (argc <= ++i) { put_usage(); }
                    int size;
                    void* bin;
                    bin = loadBinary(argv[i], &size);
                    _data.push_back(new Data("CHR", bin, size));
                    while (i + 1 < argc && argv[i + 1][0] != '-') {
                        bin = loadBinary(argv[++i], &size);
                        _data.push_back(new Data("CHR", bin, size));
                    }
                    break;
                }
                case 'c': {
                    if (argc <= ++i) { put_usage(); }
                    int size;
                    void* bin;
                    bin = loadBinary(argv[i], &size);
                    _data.push_back(new Data("PAL", bin, size));
                    break;
                }
                case 'b': {
                    if (argc <= ++i) { put_usage(); }
                    int size;
                    void* bin;
                    bin = loadBinary(argv[i], &size);
                    _data.push_back(new Data("VGM", bin, size));
                    while (i + 1 < argc && argv[i + 1][0] != '-') {
                        bin = loadBinary(argv[++i], &size);
                        _data.push_back(new Data("VGM", bin, size));
                    }
                    break;
                }
                case 's': {
                    if (argc <= ++i) { put_usage(); }
                    int size;
                    void* bin;
                    bin = loadBinary(argv[i], &size);
                    _data.push_back(new Data("WAV", bin, size));
                    while (i + 1 < argc && argv[i + 1][0] != '-') {
                        bin = loadBinary(argv[++i], &size);
                        _data.push_back(new Data("WAV", bin, size));
                    }
                    break;
                }
                case 'e': {
                    if (programSpecified) { put_usage(); }
                    if (argc <= ++i) { put_usage(); }
                    int size;
                    void* bin;
                    bin = loadBinary(argv[i], &size);
                    _data.push_back(new Data("ELF", bin, size));
                    programSpecified = true;
                    break;
                }
                case 'o': {
                    if (argc <= ++i) { put_usage(); }
                    outputPath = argv[i];
                    break;
                }
                default:
                    put_usage();
            }
        } else {
            put_usage();
        }
    }
    if (!outputPath || !programSpecified) {
        put_usage();
    }

    FILE* fp = fopen(outputPath, "wb");
    if (!fp) {
        printf("File open error: %s\n", outputPath);
        exit(255);
    }
    fprintf(fp, "VGSX");
    fputc(0x00, fp);
    fputc(0x7F, fp);
    fputc(0x01, fp); // ROM version
    fputc(0x00, fp);

    for (auto data : _data) {
        fwrite(data->name, 1, 4, fp);
        fwrite(&data->size, 1, 4, fp);
        fwrite(data->ptr, 1, data->size, fp);
        delete data;
    }
    fclose(fp);
    return 0;
}