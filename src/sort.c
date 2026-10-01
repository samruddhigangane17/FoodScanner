/* sort.c - Member C
 * Stable merge sort: most serious warnings (highest severity) first. */
#include <stdlib.h>
#include <string.h>
#include "common.h"

static void msort(Warning *a, Warning *tmp, int lo, int hi) {   /* [lo, hi) */
    if (hi - lo < 2) return;
    int mid = lo + (hi - lo) / 2;
    msort(a, tmp, lo, mid);
    msort(a, tmp, mid, hi);
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi)
        tmp[k++] = (a[j].severity > a[i].severity) ? a[j++] : a[i++];  /* stable */
    while (i < mid) tmp[k++] = a[i++];
    while (j < hi)  tmp[k++] = a[j++];
    memcpy(a + lo, tmp + lo, (size_t)(hi - lo) * sizeof(Warning));
}

void mergeSortWarnings(Warning *w, int n) {
    if (n < 2) return;
    Warning *tmp = (Warning *)malloc((size_t)n * sizeof(Warning));
    if (!tmp) return;
    msort(w, tmp, 0, n);
    free(tmp);
}
