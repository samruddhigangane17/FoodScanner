/* ocr.c - Member B
 * Calls the Tesseract command-line tool and reads the text it produces. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#ifdef _WIN32
#define NULL_DEVICE "nul"
#else
#define NULL_DEVICE "/dev/null"
#endif

/* reject characters that could break out of the shell command */
static int safePath(const char *p) {
    if (!*p) return 0;
    for (; *p; p++)
        if (strchr("\"`$;&|<>\n\r", *p)) return 0;
    return 1;
}

int runOCR(const char *imagePath, char *out, int maxLen) {
    if (!safePath(imagePath)) return 0;
    FILE *test = fopen(imagePath, "rb");          /* does the image exist? */
    if (!test) return 0;
    fclose(test);

    char cmd[600];
    snprintf(cmd, sizeof(cmd), "tesseract \"%s\" %s --psm 6 > %s 2>&1",
             imagePath, OCR_BASE, NULL_DEVICE);
    if (system(cmd) != 0) return 0;

    FILE *fp = fopen(OCR_BASE ".txt", "r");
    if (!fp) return 0;
    size_t n = fread(out, 1, (size_t)maxLen - 1, fp);
    out[n] = '\0';
    fclose(fp);
    remove(OCR_BASE ".txt");
    return n > 0;
}
