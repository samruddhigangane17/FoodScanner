/* jsonout.c - JSON output mode used by the web frontend (all logic stays in C) */
#include <stdio.h>
#include <string.h>
#include "common.h"

void jsonString(const char *s) {
    putchar('"');
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\') { putchar('\\'); putchar(c); }
        else if (c == '\n') fputs("\\n", stdout);
        else if (c >= 32) putchar(c);
    }
    putchar('"');
}

static void reasons(unsigned int conflict) {
    int first = 1;
    putchar('[');
    for (int i = 0; i < NUM_FLAGS; i++)
        if (conflict & FLAG_LIST[i]) { if (!first) putchar(','); jsonString(FLAG_REASONS[i]); first = 0; }
    putchar(']');
}

void jsonAnalyse(const char *product, const char *rawText, unsigned int profile, int showRaw) {
    static char buf[MAX_TEXT];
    static char items[MAX_ITEMS][NAME_LEN];
    Warning w[MAX_ITEMS];
    int nw = 0, unknown = 0;

    strncpy(buf, rawText, MAX_TEXT - 1); buf[MAX_TEXT - 1] = '\0';
    cleanText(buf);
    int n = splitIngredients(buf, items, MAX_ITEMS);

    printf("{\"ok\":true,");
    if (showRaw) { printf("\"ocr_text\":"); jsonString(rawText); putchar(','); }
    printf("\"cleaned\":"); jsonString(buf);
    printf(",\"items\":[");
    for (int i = 0; i < n; i++) {
        Ingredient *ing = searchIngredient(items[i]);          /* hash table */
        int corrected = 0;
        if (!ing) { ing = closestMatch(items[i], 2); corrected = (ing != NULL); }  /* edit distance */
        if (i) putchar(',');
        if (!ing) { printf("{\"name\":"); jsonString(items[i]); printf(",\"known\":false}"); unknown++; continue; }
        unsigned int conflict = checkWarnings(ing, profile);   /* bitmask AND */
        printf("{\"name\":"); jsonString(ing->name);
        printf(",\"known\":true,\"read_as\":"); jsonString(corrected ? items[i] : "");
        printf(",\"explanation\":"); jsonString(ing->explanation);
        printf(",\"conflicts\":"); reasons(conflict); putchar('}');
        if (conflict) {
            strncpy(w[nw].name, ing->name, NAME_LEN - 1); w[nw].name[NAME_LEN - 1] = '\0';
            w[nw].conflict = conflict;
            w[nw].severity = (conflict & (NUTS | DAIRY | GLUTEN | SOY)) ? 2 : 1;
            nw++;
        }
    }
    mergeSortWarnings(w, nw);                                  /* merge sort */
    printf("],\"warnings\":[");
    for (int i = 0; i < nw; i++) {
        if (i) putchar(',');
        printf("{\"name\":"); jsonString(w[i].name);
        printf(",\"severity\":%d,\"reasons\":", w[i].severity); reasons(w[i].conflict); putchar('}');
    }
    printf("],\"unknown\":%d,\"count\":%d}\n", unknown, n);
    addScan(product, nw);                                      /* linked list history */
}
