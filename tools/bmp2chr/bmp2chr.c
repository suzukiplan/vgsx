/* 8bit Bitmap to FCS80 pattern table */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"

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

static int is_png_file(const char* path)
{
    size_t len = strlen(path);
    if (len < 4) {
        return 0;
    }

    const char* ext = path + len - 4;
    return ext[0] == '.' && tolower((unsigned char)ext[1]) == 'p' && tolower((unsigned char)ext[2]) == 'n' && tolower((unsigned char)ext[3]) == 'g';
}

int main(int argc, char* argv[])
{
    FILE* fpR = NULL;
    FILE* fpW = NULL;
    int rc = 0;
    char fh[14];
    unsigned int pal256[256];
    unsigned int pal16[16];
    struct DatHead dh;
    int i, j, y, x;
    char* bmp = NULL;
    char* tmp = NULL;
    unsigned char* ptn = NULL;
    unsigned char* png_data = NULL;
    int is_png = 0;
    int png_comp = 0;

    /* 引数チェック */
    rc++;
    if (argc < 3) {
        fprintf(stderr, "usage: bmp2chr input.bmp|input.png output.chr\n");
        goto ENDPROC;
    }

    is_png = is_png_file(argv[1]);
    if (is_png) {
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
        dh.bits = 8;
        dh.ctype = 0;
        dh.cnum = 0;
        dh.inum = 0;
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

    printf("INPUT: width=%d, height=%d, bits=%d\n", dh.width, dh.height, (int)dh.bits);

    /* 8の倍数 */
    rc++;
    if (dh.width % 8 || dh.height % 8 || !dh.width || !dh.height) {
        fprintf(stderr, "ERROR: Invalid input bitmap size. (128x128 only)");
        goto ENDPROC;
    }

    /* 8ビットカラーと4ビットカラー以外は弾く */
    rc++;
    if (8 != dh.bits && 4 != dh.bits) {
        fprintf(stderr, "ERROR: This program supports only 4bit or 8bit color.\n");
        goto ENDPROC;
    }

    /* 無圧縮以外は弾く */
    rc++;
    if (dh.ctype) {
        fprintf(stderr, "ERROR: This program supports only none-compress type.\n");
        goto ENDPROC;
    }

    if (is_png) {
        rc++;
        png_data = stbi_load(argv[1], &dh.width, &dh.height, &png_comp, 1);
        if (!png_data) {
            fprintf(stderr, "ERROR: Could not load png: %s\n", stbi_failure_reason());
            goto ENDPROC;
        }
        bmp = (char*)png_data;
        for (i = 0; i < dh.width * dh.height; i++) {
            bmp[i] &= 0x0F;
        }
    } else {
        /* 読み込みメモリを確保 */
        bmp = (char*)malloc(dh.width * dh.height);
        if (!bmp) {
            fprintf(stderr, "ERROR: Out of memory.\n");
            goto ENDPROC;
        }

        rc++;
        if (dh.bits == 8) {
            /* パレットを読み飛ばす */
            if (sizeof(pal256) != fread(pal256, 1, sizeof(pal256), fpR)) {
                fprintf(stderr, "ERROR: Could not read palette data.\n");
                goto ENDPROC;
            }
            /* 画像データを上下反転しながら読み込む */
            rc++;
            for (i = dh.height - 1; 0 <= i; i--) {
                if (dh.width != fread(&bmp[i * dh.width], 1, dh.width, fpR)) {
                    fprintf(stderr, "ERROR: Could not read graphic data.\n");
                    goto ENDPROC;
                }
                /* 色情報を mod 16 (0~15) にしておく*/
                for (j = i * dh.width; j < (i + 1) * dh.width; j++) {
                    bmp[j] &= 0x0F;
                }
            }
        } else {
            /* パレットを読み飛ばす */
            if (sizeof(pal16) != fread(pal16, 1, sizeof(pal16), fpR)) {
                fprintf(stderr, "ERROR: Could not read palette data.\n");
                goto ENDPROC;
            }
            /* 画像データを上下反転しながら読み込む */
            rc++;
            tmp = (char*)malloc(dh.width / 2);
            if (!tmp) {
                fprintf(stderr, "ERROR: Out of memory.\n");
                goto ENDPROC;
            }
            for (i = dh.height - 1; 0 <= i; i--) {
                if (dh.width / 2 != fread(tmp, 1, dh.width / 2, fpR)) {
                    fprintf(stderr, "ERROR: Could not read graphic data.\n");
                    goto ENDPROC;
                }
                for (int j = 0; j < dh.width; j++) {
                    bmp[i * dh.width + j] = j & 1 ? tmp[j / 2] & 0x0F : (tmp[j / 2] & 0xF0) >> 4;
                }
            }
            free(tmp);
            tmp = NULL;
        }
    }

    /* Bitmap を パターン形式 に変換 */
    ptn = (unsigned char*)malloc(dh.width * dh.height / 2);
    if (!ptn) {
        fprintf(stderr, "ERROR: Out of memory.\n");
        goto ENDPROC;
    }
    unsigned char* ptr = ptn;
    for (y = 0; y < dh.height / 8; y++) {
        for (x = 0; x < dh.width / 8; x++) {
            for (j = 0; j < 8; j++) {
                for (i = 0; i < 4; i++) {
                    char* bp = &bmp[y * dh.width * 8 + x * 8 + j * dh.width + i * 2];
                    unsigned char c = bp[0] & 0x0F;
                    c <<= 4;
                    c |= bp[1] & 0x0F;
                    *ptr = c;
                    ptr++;
                }
            }
        }
    }

    /* 書き込みファイルをオープンしてchrを書き込む */
    rc++;
    if (NULL == (fpW = fopen(argv[2], "wb"))) {
        fprintf(stderr, "ERROR: Could not open: %s\n", argv[2]);
        goto ENDPROC;
    }
    if (dh.width * dh.height / 2 != fwrite(ptn, 1, dh.width * dh.height / 2, fpW)) {
        fprintf(stderr, "ERROR: File write error: %s\n", argv[2]);
        goto ENDPROC;
    }

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
    if (!is_png && bmp) {
        free(bmp);
    }
    if (png_data) {
        stbi_image_free(png_data);
    }
    if (ptn) {
        free(ptn);
    }
    if (tmp) {
        free(tmp);
    }
    return rc;
}
