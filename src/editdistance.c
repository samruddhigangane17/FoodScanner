/* editdistance.c - Member A
 * Levenshtein distance (dynamic programming) to repair OCR spelling mistakes. */
#include <string.h>
#include <stdlib.h>
#include "common.h"

static int min3(int a, int b, int c) {
    int m = a < b ? a : b;
    return m < c ? m : c;
}

int editDistance(const char *a, const char *b) {
    int m = (int)strlen(a), n = (int)strlen(b);
    if (m > NAME_LEN - 1) m = NAME_LEN - 1;
    if (n > NAME_LEN - 1) n = NAME_LEN - 1;
    int dp[NAME_LEN][NAME_LEN];
    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;
    for (int i = 1; i <= m; i++)
        for (int j = 1; j <= n; j++)
            dp[i][j] = min3(dp[i - 1][j] + 1,                          /* delete  */
                            dp[i][j - 1] + 1,                          /* insert  */
                            dp[i - 1][j - 1] + (a[i - 1] != b[j - 1])); /* replace */
    return dp[m][n];
}

/* Closest database ingredient within maxDist edits, or NULL.
 * Very short words (< 4 letters) are never "corrected" to avoid false matches. */
Ingredient *closestMatch(const char *name, int maxDist) {
    int len = (int)strlen(name);
    if (len < 4) return NULL;
    Ingredient *best = NULL;
    int bestD = maxDist + 1;
    for (int i = 0; i < TABLE_SIZE; i++) {
        for (Ingredient *p = dbTable[i]; p; p = p->next) {
            int diff = (int)strlen(p->name) - len;
            if (diff < 0) diff = -diff;
            if (diff > maxDist) continue;             /* cheap pre-check */
            int d = editDistance(name, p->name);
            if (d < bestD) { bestD = d; best = p; }
        }
    }
    return best;
}
