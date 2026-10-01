/* api.c - bridge for the web frontend
 * Usage:  scanner --api --profile N (--image FILE | --stdin) [--product NAME]
 *         scanner --api --history
 * Prints ONE line of JSON. All analysis still uses the same DSA modules. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

void jsonString(const char *s) {
    putchar('"');
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"') fputs("\\\"", stdout);
        else if (c == '\\') fputs("\\\\", stdout);
        else if (c == '\n') fputs("\\n", stdout);
        else if (c == '\r') fputs("\\r", stdout);
        else if (c == '\t') fputs("\\t", stdout);
        else if (c < 0x20) putchar(' ');
        else if (c >= 0x7f) putchar('?');
        else putchar(c);
    }
    putchar('"');
}

static int fail(const char *msg) {
    printf("{\"ok\":false,\"error\":");
    jsonString(msg);
    printf("}\n");
    return 1;
}

static void printReasons(unsigned int conflict) {
    int first = 1;
    putchar('[');
    for (int i = 0; i < NUM_FLAGS; i++)
        if (conflict & FLAG_LIST[i]) {
            if (!first) putchar(',');
            jsonString(FLAG_REASONS[i]);
            first = 0;
        }
    putchar(']');
}

int apiMain(int argc, char **argv) {
    unsigned int profile = 0;
    const char *image = NULL, *product = "Web scan";
    int useStdin = 0, wantHistory = 0;

    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "--profile") && i + 1 < argc) profile = (unsigned int)strtoul(argv[++i], NULL, 10) & ALL_FLAGS;
        else if (!strcmp(argv[i], "--image") && i + 1 < argc) image = argv[++i];
        else if (!strcmp(argv[i], "--product") && i + 1 < argc) product = argv[++i];
        else if (!strcmp(argv[i], "--stdin")) useStdin = 1;
        else if (!strcmp(argv[i], "--history")) wantHistory = 1;
    }

    if (wantHistory) { loadHistory(); historyJson(); freeHistory(); return 0; }
    if (loadDB(DB_FILE) == 0) return fail("Could not load the ingredient database. Start the server from the project folder.");
    loadHistory();                 /* addScan rewrites the file, so load old scans first */

    static char text[MAX_TEXT], rawOcr[MAX_TEXT];
    static char items[MAX_ITEMS][NAME_LEN];
    rawOcr[0] = '\0';

    if (image) {
        if (!runOCR(image, text, MAX_TEXT)) return fail("OCR failed. Check that Tesseract is installed and the photo is readable.");
        strcpy(rawOcr, text);
    } else if (useStdin) {
        size_t n = fread(text, 1, MAX_TEXT - 1, stdin);
        text[n] = '\0';
    } else return fail("No input given");

    cleanText(text);
    int n = splitIngredients(text, items, MAX_ITEMS);
    if (n == 0) return fail("No ingredients could be found in that text.");

    Warning warns[MAX_ITEMS];
    int nw = 0, unknown = 0;

    printf("{\"ok\":true,\"ocr_text\":");  jsonString(rawOcr);
    printf(",\"cleaned\":");               jsonString(text);
    printf(",\"items\":[");
    for (int i = 0; i < n; i++) {
        Ingredient *ing = searchIngredient(items[i]);          /* hash table */
        int corrected = 0;
        if (!ing) { ing = closestMatch(items[i], 2); corrected = (ing != NULL); }  /* edit distance */
        if (i) putchar(',');
        printf("{\"input\":"); jsonString(items[i]);
        if (!ing) { printf(",\"found\":false}"); unknown++; continue; }
        unsigned int conflict = checkWarnings(ing, profile);   /* bitwise AND */
        printf(",\"found\":true,\"name\":");  jsonString(ing->name);
        printf(",\"corrected\":%s,\"explanation\":", corrected ? "true" : "false");
        jsonString(ing->explanation);
        printf(",\"conflicts\":"); printReasons(conflict);
        printf(",\"severity\":%d}", severityOf(conflict));
        if (conflict) {
            strncpy(warns[nw].name, ing->name, NAME_LEN - 1);
            warns[nw].name[NAME_LEN - 1] = '\0';
            warns[nw].conflict = conflict;
            warns[nw].severity = severityOf(conflict);
            nw++;
        }
    }
    mergeSortWarnings(warns, nw);                              /* merge sort */
    printf("],\"warnings\":[");
    for (int i = 0; i < nw; i++) {
        if (i) putchar(',');
        printf("{\"name\":"); jsonString(warns[i].name);
        printf(",\"severity\":%d,\"reasons\":", warns[i].severity);
        printReasons(warns[i].conflict);
        putchar('}');
    }
    printf("],\"unknown\":%d,\"count\":%d}\n", unknown, n);

    addScan(product, nw);                                      /* linked list + file */
    freeDB(); freeHistory();
    return 0;
}
