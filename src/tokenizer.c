/* tokenizer.c - Member B
 * Splits a cleaned ingredient string into separate ingredient names.
 * "emulsifier (lecithin, e471), sugar (14%)"  ->  emulsifier, lecithin, e471, sugar */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

static const char *PREFIXES[] = {
    "contains ", "may contain ", "traces of ", "added ", "and ", NULL
};

/* clean one raw token into a normal ingredient name; returns 1 if usable */
static int normalise(const char *src, char *dst) {
    char tmp[256];
    size_t i = 0;
    for (; src[i] && i < sizeof(tmp) - 1; i++) {
        char c = (char)tolower((unsigned char)src[i]);
        tmp[i] = (c == ':') ? ' ' : c;
    }
    tmp[i] = '\0';

    /* "sugar 14%" -> "sugar" ; "(12%)" -> "" */
    char *pc = strchr(tmp, '%');
    if (pc) {
        int k = (int)(pc - tmp);
        while (k > 0 && (isdigit((unsigned char)tmp[k - 1]) || tmp[k - 1] == '.' || tmp[k - 1] == ' ')) k--;
        tmp[k] = '\0';
    }

    char *s = tmp;
    int changed = 1;
    while (changed) {                                  /* strip junk and prefixes */
        changed = 0;
        s = trim(s);
        size_t len = strlen(s);
        while (len > 0 && (s[len - 1] == '.' || s[len - 1] == '-' || s[len - 1] == ' ')) s[--len] = '\0';
        while (*s == '.' || *s == '-') { s++; changed = 1; }
        for (int p = 0; PREFIXES[p]; p++) {
            size_t pl = strlen(PREFIXES[p]);
            if (strncmp(s, PREFIXES[p], pl) == 0) { s += pl; changed = 1; break; }
        }
    }

    /* "ins 322" / "e 322"  ->  "e322" */
    if (strncmp(s, "ins ", 4) == 0 && isdigit((unsigned char)s[4])) {
        memmove(s + 1, s + 4, strlen(s + 4) + 1);
        s[0] = 'e';
    } else if (s[0] == 'e' && s[1] == ' ' && isdigit((unsigned char)s[2])) {
        memmove(s + 1, s + 2, strlen(s + 2) + 1);
    }

    /* must contain a letter and be at least 2 characters long */
    int letters = 0;
    for (char *q = s; *q; q++) if (isalpha((unsigned char)*q)) letters++;
    if (letters == 0 || strlen(s) < 2 || strlen(s) >= NAME_LEN) return 0;
    strcpy(dst, s);
    return 1;
}

static void addItem(char items[][NAME_LEN], int *count, int maxItems, const char *raw) {
    char name[NAME_LEN];
    if (*count >= maxItems || !normalise(raw, name)) return;
    for (int i = 0; i < *count; i++)                    /* skip duplicates */
        if (strcmp(items[i], name) == 0) return;
    strcpy(items[(*count)++], name);
}

/* split a comma separated list (used for bracket contents) */
static void addList(char items[][NAME_LEN], int *count, int maxItems, char *list) {
    if (strchr(list, '%')) return;                      /* "(min 20%)" -> ignore */
    char *p = list;
    for (;;) {
        char *comma = strchr(p, ',');
        if (comma) *comma = '\0';
        addItem(items, count, maxItems, p);
        if (!comma) break;
        p = comma + 1;
    }
}

int splitIngredients(const char *text, char items[][NAME_LEN], int maxItems) {
    int count = 0, t = 0, in = 0, depth = 0;
    char tok[256], inner[256];

    for (const char *p = text; ; p++) {
        char ch = *p;
        if (depth == 0) {
            if (ch == ',' || ch == '\0') {
                tok[t] = '\0'; addItem(items, &count, maxItems, tok); t = 0;
            } else if (ch == '(') {
                tok[t] = '\0'; addItem(items, &count, maxItems, tok); t = 0;
                depth = 1; in = 0;
            } else if (ch != ')' && t < (int)sizeof(tok) - 1) {
                tok[t++] = ch;
            }
        } else {
            if (ch == '(') { depth++; ch = ','; }
            else if (ch == ')') {
                depth--;
                if (depth == 0) {
                    inner[in] = '\0'; addList(items, &count, maxItems, inner);
                    continue;
                }
                ch = ',';
            }
            if (ch == '\0') { inner[in] = '\0'; addList(items, &count, maxItems, inner); }
            else if (in < (int)sizeof(inner) - 1) inner[in++] = ch;
        }
        if (ch == '\0') break;
    }
    return count;
}
