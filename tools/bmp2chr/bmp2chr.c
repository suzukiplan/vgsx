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

static unsigned int read_be32(const unsigned char* p)
{
    return ((unsigned int)p[0] << 24) | ((unsigned int)p[1] << 16) | ((unsigned int)p[2] << 8) | (unsigned int)p[3];
}

static unsigned char paeth(unsigned char a, unsigned char b, unsigned char c)
{
    int p = (int)a + (int)b - (int)c;
    int pa = p > a ? p - a : a - p;
    int pb = p > b ? p - b : b - p;
    int pc = p > c ? p - c : c - p;
    if (pa <= pb && pa <= pc) {
        return a;
    }
    if (pb <= pc) {
        return b;
    }
    return c;
}

static int load_indexed_png(const char* path, struct DatHead* dh, unsigned char** out_pixels, unsigned int* palette, int* palette_size)
{
    static const unsigned char sig[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    unsigned char plte[256 * 3];
    unsigned char header[13];
    unsigned char alpha[256];
    unsigned char* idata = NULL;
    int idata_len = 0;
    unsigned char* inflated = NULL;
    int inflated_len = 0;
    int bit_depth = 0;
    int color_type = 0;
    int interlace = 0;
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }

    unsigned char sigbuf[8];
    if (fread(sigbuf, 1, 8, fp) != 8 || memcmp(sigbuf, sig, 8)) {
        fclose(fp);
        return 0;
    }

    memset(alpha, 0xFF, sizeof(alpha));
    *palette_size = 0;

    for (;;) {
        unsigned char lenbuf[4];
        char type[5];
        unsigned int len;

        if (fread(lenbuf, 1, 4, fp) != 4 || fread(type, 1, 4, fp) != 4) {
            break;
        }
        len = read_be32(lenbuf);
        type[4] = '\0';

        if (!memcmp(type, "IHDR", 4)) {
            if (len != 13 || fread(header, 1, 13, fp) != 13) {
                fclose(fp);
                return 0;
            }
            dh->width = (int)read_be32(header);
            dh->height = (int)read_be32(header + 4);
            bit_depth = header[8];
            color_type = header[9];
            interlace = header[12];
            if (color_type != 3 || (bit_depth != 4 && bit_depth != 8) || interlace != 0) {
                fclose(fp);
                return 0;
            }
        } else if (!memcmp(type, "PLTE", 4)) {
            if (len > 256 * 3 || (len % 3)) {
                fclose(fp);
                return 0;
            }
            if (fread(plte, 1, len, fp) != len) {
                fclose(fp);
                return 0;
            }
            *palette_size = (int)(len / 3);
            for (int i = 0; i < *palette_size; i++) {
                palette[i] = ((unsigned int)plte[i * 3] << 24) | ((unsigned int)plte[i * 3 + 1] << 16) | ((unsigned int)plte[i * 3 + 2] << 8);
            }
        } else if (!memcmp(type, "tRNS", 4)) {
            unsigned int count = len < 256 ? len : 256;
            if (fread(alpha, 1, count, fp) != count) {
                fclose(fp);
                return 0;
            }
            if (len > count && fseek(fp, (long)(len - count), SEEK_CUR)) {
                fclose(fp);
                return 0;
            }
        } else if (!memcmp(type, "IDAT", 4)) {
            unsigned char* newbuf = (unsigned char*)realloc(idata, (size_t)idata_len + len);
            if (!newbuf) {
                fclose(fp);
                free(idata);
                return 0;
            }
            idata = newbuf;
            if (fread(idata + idata_len, 1, len, fp) != len) {
                fclose(fp);
                free(idata);
                return 0;
            }
            idata_len += (int)len;
        } else {
            if (fseek(fp, (long)len, SEEK_CUR)) {
                break;
            }
        }

        if (fseek(fp, 4, SEEK_CUR)) {
            break;
        }
        if (!memcmp(type, "IEND", 4)) {
            break;
        }
    }

    fclose(fp);

    if (!idata_len || !*palette_size || !dh->width || !dh->height) {
        free(idata);
        return 0;
    }

    for (int i = 0; i < *palette_size; i++) {
        palette[i] |= alpha[i];
    }

    int row_bytes = (dh->width * bit_depth + 7) / 8;
    int expected = (row_bytes + 1) * dh->height;
    inflated = (unsigned char*)stbi_zlib_decode_malloc_guesssize_headerflag((char*)idata, idata_len, expected, &inflated_len, 1);
    free(idata);
    if (!inflated || inflated_len < expected) {
        free(inflated);
        return 0;
    }

    unsigned char* unpacked = (unsigned char*)malloc((size_t)dh->width * dh->height);
    if (!unpacked) {
        free(inflated);
        return 0;
    }

    unsigned char* prev = NULL;
    int out_pos = 0;
    unsigned char* ptr = inflated;
    for (int y = 0; y < dh->height; y++) {
        unsigned char filter = *ptr++;
        unsigned char* cur = ptr;
        unsigned char* row = cur;
        switch (filter) {
        case 0:
            break;
        case 1:
            for (int i = 0; i < row_bytes; i++) {
                unsigned char left = i ? row[i - 1] : 0;
                row[i] = (unsigned char)(row[i] + left);
            }
            break;
        case 2:
            for (int i = 0; i < row_bytes; i++) {
                unsigned char up = prev ? prev[i] : 0;
                row[i] = (unsigned char)(row[i] + up);
            }
            break;
        case 3:
            for (int i = 0; i < row_bytes; i++) {
                unsigned char left = i ? row[i - 1] : 0;
                unsigned char up = prev ? prev[i] : 0;
                row[i] = (unsigned char)(row[i] + (unsigned char)(((int)left + (int)up) / 2));
            }
            break;
        case 4:
            for (int i = 0; i < row_bytes; i++) {
                unsigned char left = i ? row[i - 1] : 0;
                unsigned char up = prev ? prev[i] : 0;
                unsigned char up_left = (prev && i) ? prev[i - 1] : 0;
                row[i] = (unsigned char)(row[i] + paeth(left, up, up_left));
            }
            break;
        default:
            free(unpacked);
            free(inflated);
            return 0;
        }

        if (bit_depth == 8) {
            for (int i = 0; i < dh->width; i++) {
                unpacked[out_pos++] = (unsigned char)(row[i] & 0x0F);
            }
        } else {
            for (int i = 0; i < dh->width; i++) {
                int byte_index = i / 2;
                unsigned char v = row[byte_index];
                unsigned char idx = (i & 1) ? (unsigned char)(v & 0x0F) : (unsigned char)((v >> 4) & 0x0F);
                unpacked[out_pos++] = idx;
            }
        }

        prev = row;
        ptr += row_bytes;
    }

    free(inflated);

    dh->bits = 4;
    dh->ctype = 0;
    dh->cnum = 0;
    dh->inum = 0;

    *out_pixels = unpacked;
    return 1;
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
    int is_png = 0;
    unsigned int png_palette[256];
    int png_palette_size = 0;

    /* 引数チェック */
    rc++;
    if (argc < 3) {
        fprintf(stderr, "usage: bmp2chr {input.bmp|input.png} output.chr\n");
        goto ENDPROC;
    }

    is_png = is_png_file(argv[1]);
    if (is_png) {
        rc++;
        if (!load_indexed_png(argv[1], &dh, (unsigned char**)&bmp, png_palette, &png_palette_size)) {
            fprintf(stderr, "ERROR: Could not load indexed png: %s\n", argv[1]);
            goto ENDPROC;
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
        /* 既に bmp に読み込み済み */
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
    if (bmp) {
        free(bmp);
    }
    if (ptn) {
        free(ptn);
    }
    if (tmp) {
        free(tmp);
    }
    return rc;
}
