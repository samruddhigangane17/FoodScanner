/* history.c - Member C
 * Scan history as a singly linked list (newest first), saved to a file. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"

static Scan *head = NULL;

static void saveHistory(void) {
    FILE *fp = fopen(HISTORY_FILE, "w");
    if (!fp) return;
    for (Scan *p = head; p; p = p->next)
        fprintf(fp, "%s|%d|%s\n", p->product, p->warnings, p->when);
    fclose(fp);
}

void addScan(const char *product, int warnings) {
    Scan *s = (Scan *)malloc(sizeof(Scan));
    if (!s) return;
    strncpy(s->product, product, NAME_LEN - 1); s->product[NAME_LEN - 1] = '\0';
    for (char *q = s->product; *q; q++) if (*q == '|') *q = '/';   /* keep file format safe */
    s->warnings = warnings;
    time_t now = time(NULL);
    strftime(s->when, sizeof(s->when), "%Y-%m-%d %H:%M", localtime(&now));
    s->next = head;                          /* insert at head : O(1) */
    head = s;
    saveHistory();
}

void loadHistory(void) {
    FILE *fp = fopen(HISTORY_FILE, "r");
    if (!fp) return;
    Scan *tail = NULL;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        char *b = strrchr(line, '|');
        if (!b) continue;
        *b = '\0';
        char *a = strrchr(line, '|');
        if (!a) continue;
        *a = '\0';
        Scan *s = (Scan *)malloc(sizeof(Scan));
        if (!s) break;
        strncpy(s->product, line, NAME_LEN - 1); s->product[NAME_LEN - 1] = '\0';
        s->warnings = atoi(a + 1);
        strncpy(s->when, b + 1, sizeof(s->when) - 1); s->when[sizeof(s->when) - 1] = '\0';
        s->next = NULL;
        if (tail) tail->next = s; else head = s;   /* file is newest-first, keep order */
        tail = s;
    }
    fclose(fp);
}

void showHistory(void) {
    printf("\n=== Scan history (newest first) ===\n");
    if (!head) { printf("  No scans yet.\n"); return; }
    int i = 1;
    for (Scan *p = head; p; p = p->next, i++)
        printf("  %2d. %-30s  %s  warnings: %d\n", i, p->product, p->when, p->warnings);
}

void freeHistory(void) {
    while (head) { Scan *t = head->next; free(head); head = t; }
}
