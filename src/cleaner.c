/* cleaner.c - Member B
 * Cleans raw OCR / typed text so the tokenizer gets simple input. */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

/* words that normally start the part of a label we do NOT want */
static const char *STOP_WORDS[] = {
    "nutrition", "nutritional", "best before", "manufactured", "marketed",
    "mfg", "store in", "net weight", "net wt", "use by", "allergen advice", NULL
};

void cleanText(char *text) {
    size_t n = strlen(text);
    char *buf = (char *)malloc(n + 2);
    if (!buf) return;
    size_t j = 0;

    /* pass 1: join hyphenated line breaks, lowercase, replace odd symbols */
    for (size_t i = 0; i < n; i++) {
        unsigned char ch = (unsigned char)text[i];
        if (ch == '-' && (text[i + 1] == '\n' || text[i + 1] == '\r')) {
            i++;
            while (text[i + 1] == '\n' || text[i + 1] == '\r') i++;
            continue;
        }
        ch = (unsigned char)tolower(ch);
        if (isalnum(ch) || strchr(",()%.-:", ch)) buf[j++] = (char)ch;
        else if (ch == '[') buf[j++] = '(';
        else if (ch == ']') buf[j++] = ')';
        else if (ch == ';') buf[j++] = ',';
        else buf[j++] = ' ';
    }
    buf[j] = '\0';

    /* pass 2: collapse repeated spaces */
    size_t k = 0;
    for (size_t i = 0; i < j; i++) {
        if (buf[i] == ' ' && k > 0 && buf[k - 1] == ' ') continue;
        buf[k++] = buf[i];
    }
    buf[k] = '\0';

    /* keep only the text after the word "ingredients" (if present) */
    char *start = buf;
    char *h = strstr(buf, "ingredients");
    if (h) {
        start = h + strlen("ingredients");
        while (*start == ':' || *start == '.' || *start == ' ') start++;
    }

    /* cut off at the first stop word (nutrition table, dates, etc.) */
    for (int s = 0; STOP_WORDS[s]; s++) {
        char *p = strstr(start, STOP_WORDS[s]);
        if (p) *p = '\0';
    }

    strcpy(text, trim(start));
    free(buf);
}
