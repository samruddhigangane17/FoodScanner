/* main.c - Member C (integration)
 * Menu-driven Food Label Scanner. Run from the project root folder. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

static void stripQuotes(char *s) {            /* drag-and-drop paths may have quotes */
    size_t n = strlen(s);
    if (n >= 2 && s[0] == '"' && s[n - 1] == '"') { memmove(s, s + 1, n - 2); s[n - 2] = '\0'; }
}

static void analyseText(char *text, unsigned int profile) {
    char product[NAME_LEN];
    static char items[MAX_ITEMS][NAME_LEN];

    cleanText(text);
    printf("\nCleaned text: %s\n", text[0] ? text : "(empty)");
    int n = splitIngredients(text, items, MAX_ITEMS);
    if (n == 0) { printf("No ingredients could be found in that text.\n"); return; }
    printf("Found %d ingredient(s).\n", n);

    readLine("Product name (optional): ", product, sizeof(product));
    if (!trim(product)[0]) strcpy(product, "Unnamed product");
    runAnalysis(trim(product), items, n, profile);
}

static void scanImage(unsigned int profile) {
    char path[300];
    static char text[MAX_TEXT];
    readLine("Path of the ingredient label image: ", path, sizeof(path));
    stripQuotes(trim(path));
    printf("Reading image with OCR...\n");
    if (!runOCR(trim(path), text, MAX_TEXT)) {
        printf("OCR failed (is the file path correct and Tesseract installed?).\n"
               "Use option 2 to type the ingredients instead.\n");
        return;
    }
    printf("\nOCR text:\n%s\n", text);
    analyseText(text, profile);
}

static void typeIngredients(unsigned int profile) {
    static char text[MAX_TEXT];
    readLine("Type or paste the ingredient list:\n> ", text, sizeof(text));
    analyseText(text, profile);
}

/* non-interactive mode used by server.py:
 *   scanner --json <profile> <product> <text>
 *   scanner --json-ocr <profile> <product> <imagePath>
 *   scanner --history-json                                  */
static int cliMode(int argc, char **argv) {
    static char t[MAX_TEXT];
    if (strcmp(argv[1], "--history-json") == 0) { printHistoryJson(); return 0; }
    if (argc >= 5) {
        unsigned int p = (unsigned int)strtoul(argv[2], NULL, 10) & ALL_FLAGS;
        if (strcmp(argv[1], "--json") == 0) { jsonAnalyse(argv[3], argv[4], p, 0); return 0; }
        if (strcmp(argv[1], "--json-ocr") == 0) {
            if (!runOCR(argv[4], t, MAX_TEXT)) {
                printf("{\"ok\":false,\"error\":\"OCR failed. Check that Tesseract is installed and the photo is clear.\"}\n");
                return 0;
            }
            jsonAnalyse(argv[3], t, p, 1);
            return 0;
        }
    }
    printf("{\"ok\":false,\"error\":\"bad arguments\"}\n");
    return 1;
}

int main(int argc, char **argv) {
    char choice[16];
    int loaded = loadDB(DB_FILE);
    if (loaded == 0) {
        printf("Could not load %s. Run the program from the project root folder.\n", DB_FILE);
        return 1;
    }
    unsigned int profile = loadProfile();
    loadHistory();
    if (argc >= 2 && strncmp(argv[1], "--", 2) == 0) return cliMode(argc, argv);

    printf("=========================================\n");
    printf("        FOOD LABEL SCANNER\n");
    printf("  %d ingredients in database\n", loaded);
    printf("=========================================\n");
    printf("Informational tool only - always check the real label.\n");
    printProfile(profile);

    for (;;) {
        if (feof(stdin)) { freeDB(); freeHistory(); return 0; }
        printf("\n1. Scan label image (OCR)\n2. Type ingredients\n3. Edit profile\n"
               "4. Scan history\n5. Exit\n");
        readLine("Choice: ", choice, sizeof(choice));
        switch (atoi(choice)) {
            case 1: scanImage(profile); break;
            case 2: typeIngredients(profile); break;
            case 3: profile = editProfile(profile); printProfile(profile); break;
            case 4: showHistory(); break;
            case 5: freeDB(); freeHistory(); printf("Goodbye!\n"); return 0;
            default: printf("Please enter 1 to 5.\n");
        }
    }
}
