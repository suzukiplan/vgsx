/* 8bit Bitmap to FCS80 pattern table */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"

static int is_png_file(const char* path)
{
    size_t len = strlen(path);
    if (len < 4) {
        return 0;
    }

    const char* ext = path + len - 4;
    return ext[0] == '.' && tolower((unsigned char)ext[1]) == 'p' && tolower((unsigned char)ext[2]) == 'n' && tolower((unsigned char)ext[3]) == 'g';
}

/* 情報ヘッダ */
struct DatHead {
    int isize;             /* 情報ヘッダサイズ */
    int width;             /* 幅 */
    int height;            /* 高さ */
    unsigned short planes; /* プレーン数 */
    unsigned short bits;   /* 色ビット数 */
    unsigned int ctype;    /* 圧縮形式 */
    unsigned int gsize;    /* 画像データサイズ */
    int xppm;              /* X方向解像度 */
    int yppm;              /* Y方向解像度 */
    unsigned int cnum;     /* 使用色数 */
    unsigned int inum;     /* 重要色数 */
};

int main(int argc, char* argv[])
{
    FILE* fpR = NULL;
    FILE* fpW = NULL;
    int rc = 0;
    char fh[14];
    unsigned int pal256[256] = {0};
    struct DatHead dh;
    unsigned char bh, bl;
    unsigned char mh[4];
    int i, j, k, y, x, a;
    unsigned char* png_data = NULL;
    int is_png = 0;
    int png_comp = 0;
    int palette_size = 0;

    /* 引数チェック */
    rc++;
    if (argc < 3) {
        fprintf(stderr, "usage: bmp2pal {input.bmp|input.png} palette.dat\n");
        goto ENDPROC;
    }

    is_png = is_png_file(argv[1]);
    if (is_png) {
        rc++;
        if (NULL == (fpR = fopen(argv[1], "rb"))) {
            fprintf(stderr, "ERROR: Could not open: %s\n", argv[1]);
            goto ENDPROC;
        }

        const unsigned char expected_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
        unsigned char signature[8];
        rc++;
        if (sizeof(signature) != fread(signature, 1, sizeof(signature), fpR) || memcmp(signature, expected_sig, sizeof(signature))) {
            fprintf(stderr, "ERROR: Invalid png signature: %s\n", argv[1]);
            goto ENDPROC;
        }

        int palette_loaded = 0;
        int got_ihdr = 0;
        unsigned char bit_depth = 0;
        unsigned char color_type = 0;

        while (1) {
            unsigned char len_buf[4];
            unsigned char type_buf[4];
            if (sizeof(len_buf) != fread(len_buf, 1, sizeof(len_buf), fpR) || sizeof(type_buf) != fread(type_buf, 1, sizeof(type_buf), fpR)) {
                fprintf(stderr, "ERROR: Unexpected EOF while reading png chunks: %s\n", argv[1]);
                goto ENDPROC;
            }

            unsigned int chunk_len = ((unsigned int)len_buf[0] << 24) | ((unsigned int)len_buf[1] << 16) | ((unsigned int)len_buf[2] << 8) | (unsigned int)len_buf[3];

            if (!memcmp(type_buf, "IHDR", 4)) {
                unsigned char ihdr[13];
                if (chunk_len != sizeof(ihdr)) {
                    fprintf(stderr, "ERROR: Invalid IHDR length: %s\n", argv[1]);
                    goto ENDPROC;
                }
                rc++;
                if (sizeof(ihdr) != fread(ihdr, 1, sizeof(ihdr), fpR)) {
                    fprintf(stderr, "ERROR: Could not read IHDR: %s\n", argv[1]);
                    goto ENDPROC;
                }
                dh.width = ((unsigned int)ihdr[0] << 24) | ((unsigned int)ihdr[1] << 16) | ((unsigned int)ihdr[2] << 8) | (unsigned int)ihdr[3];
                dh.height = ((unsigned int)ihdr[4] << 24) | ((unsigned int)ihdr[5] << 16) | ((unsigned int)ihdr[6] << 8) | (unsigned int)ihdr[7];
                bit_depth = ihdr[8];
                color_type = ihdr[9];
                got_ihdr = 1;
                if (fseek(fpR, 4, SEEK_CUR)) {
                    fprintf(stderr, "ERROR: Could not skip IHDR CRC: %s\n", argv[1]);
                    goto ENDPROC;
                }
                continue;
            }

            if (!memcmp(type_buf, "PLTE", 4)) {
                if ((chunk_len % 3) || chunk_len > 768) {
                    fprintf(stderr, "ERROR: Invalid PLTE length: %s\n", argv[1]);
                    goto ENDPROC;
                }
                unsigned char pal_buf[768];
                if (chunk_len && chunk_len != fread(pal_buf, 1, chunk_len, fpR)) {
                    fprintf(stderr, "ERROR: Could not read PLTE: %s\n", argv[1]);
                    goto ENDPROC;
                }
                palette_size = chunk_len / 3;
                for (i = 0; i < palette_size; i++) {
                    pal256[i] = ((unsigned int)pal_buf[i * 3] << 16) | ((unsigned int)pal_buf[i * 3 + 1] << 8) | pal_buf[i * 3 + 2];
                }
                palette_loaded = 1;
            } else {
                if (chunk_len && fseek(fpR, chunk_len, SEEK_CUR)) {
                    fprintf(stderr, "ERROR: Could not skip chunk: %s\n", argv[1]);
                    goto ENDPROC;
                }
            }

            if (fseek(fpR, 4, SEEK_CUR)) { /* CRC */
                fprintf(stderr, "ERROR: Could not skip CRC: %s\n", argv[1]);
                goto ENDPROC;
            }

            if ((palette_loaded && got_ihdr) || !memcmp(type_buf, "IEND", 4)) {
                break;
            }
        }

        fclose(fpR);
        fpR = NULL;

        if (!got_ihdr) {
            fprintf(stderr, "ERROR: IHDR chunk not found: %s\n", argv[1]);
            goto ENDPROC;
        }

        if (color_type == 3) {
            if (!palette_loaded) {
                fprintf(stderr, "ERROR: Indexed PNG has no palette: %s\n", argv[1]);
                goto ENDPROC;
            }
            if (bit_depth != 8) {
                fprintf(stderr, "ERROR: Indexed PNG must be 8bit: %s\n", argv[1]);
                goto ENDPROC;
            }
            dh.bits = 8;
            dh.ctype = 0;
            dh.cnum = palette_size;
            dh.inum = 0;
            for (; palette_size < 256; palette_size++) {
                pal256[palette_size] = 0;
            }
        } else {
            rc++;
            if (!stbi_info(argv[1], &dh.width, &dh.height, &png_comp)) {
                fprintf(stderr, "ERROR: Could not read png header: %s\n", argv[1]);
                goto ENDPROC;
            }
            rc++;
            if (stbi_is_16_bit(argv[1])) {
                fprintf(stderr, "ERROR: 16bit png is not supported: %s\n", argv[1]);
                goto ENDPROC;
            }
            if (png_comp == 2 || png_comp == 4) {
                fprintf(stderr, "ERROR: PNG with alpha channel is not supported: %s\n", argv[1]);
                goto ENDPROC;
            }
            rc++;
            png_data = stbi_load(argv[1], &dh.width, &dh.height, &png_comp, 3);
            if (!png_data) {
                fprintf(stderr, "ERROR: Could not load png: %s\n", stbi_failure_reason());
                goto ENDPROC;
            }
            dh.bits = 8;
            dh.ctype = 0;
            dh.cnum = 0;
            dh.inum = 0;

            int pixel_count = dh.width * dh.height;
            const unsigned char* ptr = png_data;
            for (i = 0; i < pixel_count; i++, ptr += 3) {
                unsigned int color = ((unsigned int)ptr[0] << 16) | ((unsigned int)ptr[1] << 8) | ptr[2];
                int found = 0;
                for (j = 0; j < palette_size; j++) {
                    if (pal256[j] == color) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    if (palette_size >= 256) {
                        fprintf(stderr, "ERROR: PNG has more than 256 colors: %s\n", argv[1]);
                        goto ENDPROC;
                    }
                    pal256[palette_size] = color;
                    palette_size++;
                }
            }
            if (!palette_size) {
                fprintf(stderr, "ERROR: PNG has no color data: %s\n", argv[1]);
                goto ENDPROC;
            }
            for (; palette_size < 256; palette_size++) {
                pal256[palette_size] = 0;
            }
        }
    } else {
        /* 読み込みファイルをオープン */
        rc++;
        if (NULL == (fpR = fopen(argv[1], "rb"))) {
            fprintf(stderr, "ERROR: Could not open: %s\n", argv[1]);
            goto ENDPROC;
        }

        /* ファイルヘッダを読み込む */
        rc++;
        if (sizeof(fh) != fread(fh, 1, sizeof(fh), fpR)) {
            fprintf(stderr, "ERROR: Invalid file header.\n");
            goto ENDPROC;
        }

        /* 先頭2バイトだけ読む */
        rc++;
        if (strncmp(fh, "BM", 2)) {
            fprintf(stderr, "ERROR: Inuput file is not bitmap.\n");
            goto ENDPROC;
        }

        /* 情報ヘッダを読み込む */
        rc++;
        if (sizeof(dh) != fread(&dh, 1, sizeof(dh), fpR)) {
            fprintf(stderr, "ERROR: Invalid bitmap file header.\n");
            goto ENDPROC;
        }
    }

    printf("INPUT: width=%d, height=%d, bits=%d(%d), cmp=%d\n", dh.width, dh.height, (int)dh.bits, dh.cnum, dh.ctype);

    /* 8ビットカラー以外は弾く */
    rc++;
    if (8 != dh.bits) {
        fprintf(stderr, "ERROR: This program supports only 8bit color.\n");
        goto ENDPROC;
    }

    /* 無圧縮以外は弾く */
    rc++;
    if (dh.ctype) {
        fprintf(stderr, "ERROR: This program supports only none-compress type.\n");
        goto ENDPROC;
    }

    if (!is_png) {
        /* パレットを読み込む */
        rc++;
        if (sizeof(pal256) != fread(pal256, 1, sizeof(pal256), fpR)) {
            fprintf(stderr, "ERROR: Could not read palette data.\n");
            goto ENDPROC;
        }
    }

    /* VGS-X の Palette RAM 形式でファイルを書き出す */
    rc++;
    for (int i = 0; i < 256; i++) {
        pal256[i] = pal256[i] & 0x00FFFFFF; // mask alpha channel
    }
    if (NULL == (fpW = fopen(argv[2], "wb"))) {
        fprintf(stderr, "ERROR: Could not open: %s\n", argv[2]);
        goto ENDPROC;
    }
    fwrite(pal256, 1, sizeof(pal256), fpW);

    rc = 0;

    printf("succeed.\n");

    /* 終了処理 */
ENDPROC:
    if (fpR) {
        fclose(fpR);
    }
    if (fpW) {
        fclose(fpW);
    }
    if (png_data) {
        stbi_image_free(png_data);
    }
    return rc;
}
