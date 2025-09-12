/* 8bit Bitmap to FCS80 pattern table */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
    unsigned int pal256[256];
    struct DatHead dh;
    unsigned char bh, bl;
    unsigned char mh[4];
    int i, j, k, y, x, a;

    /* 引数チェック */
    rc++;
    if (argc < 3) {
        fprintf(stderr, "usage: bmp2pal input.bmp palette.dat\n");
        goto ENDPROC;
    }

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

    /* パレットを読み込む */
    rc++;
    if (sizeof(pal256) != fread(pal256, 1, sizeof(pal256), fpR)) {
        fprintf(stderr, "ERROR: Could not read palette data.\n");
        goto ENDPROC;
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
    return rc;
}