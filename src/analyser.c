/* analyser.c - Member C
 * Looks up every ingredient, explains it, and checks it against the profile. */
#include <stdio.h>
#include <string.h>
#include "common.h"

/* ONE bitwise AND decides whether the ingredient clashes with the user */
unsigned int checkWarnings(const Ingredient *ing, unsigned int profile) {
    return ing->tags & profile;
}

int severityOf(unsigned int conflict) {
    if (conflict & (NUTS | DAIRY | GLUTEN | SOY)) return 2;   /* allergy type */
    return conflict ? 1 : 0;                                  /* diet type    */
}

static void describeConflict(unsigned int conflict, char *buf, size_t size) {
    buf[0] = '\0';
    for (int i = 0; i < NUM_FLAGS; i++) {
        if (!(conflict & FLAG_LIST[i])) continue;
        if (buf[0]) strncat(buf, ", ", size - strlen(buf) - 1);
        strncat(buf, FLAG_REASONS[i], size - strlen(buf) - 1);
    }
}

void runAnalysis(const char *product, char items[][NAME_LEN], int n, unsigned int profile) {
    Warning warns[MAX_ITEMS];
    int nw = 0, unknown = 0;
    char reason[128];

    printf("\n==================== RESULT: %s ====================\n", product);
    for (int i = 0; i < n; i++) {
        Ingredient *ing = searchIngredient(items[i]);
        int corrected = 0;
        if (!ing) { ing = closestMatch(items[i], 2); corrected = (ing != NULL); }

        if (!ing) {
            printf("%2d. %s\n    Not in our database - please check this one on the label yourself.\n",
                   i + 1, items[i]);
            unknown++;
            continue;
        }
        if (corrected)
            printf("%2d. %s   (read as '%s', matched to this)\n", i + 1, ing->name, items[i]);
        else
            printf("%2d. %s\n", i + 1, ing->name);
        printf("    %s\n", ing->explanation);

        unsigned int conflict = checkWarnings(ing, profile);
        if (conflict) {
            describeConflict(conflict, reason, sizeof(reason));
            printf("    [!] WARNING: %s\n", reason);
            strncpy(warns[nw].name, ing->name, NAME_LEN - 1);
            warns[nw].name[NAME_LEN - 1] = '\0';
            warns[nw].conflict = conflict;
            warns[nw].severity = severityOf(conflict);
            nw++;
        }
    }

    mergeSortWarnings(warns, nw);            /* most serious first */

    printf("\n---------------------- SUMMARY ----------------------\n");
    if (profile == 0)
        printf("Your profile has no restrictions, so no warnings can be given.\n"
               "Use 'Edit profile' to add your allergies or diet.\n");
    else if (nw == 0)
        printf("No conflicts found with your profile.\n");
    else {
        printf("%d ingredient(s) conflict with your profile. NOT suitable for you:\n", nw);
        for (int i = 0; i < nw; i++) {
            describeConflict(warns[i].conflict, reason, sizeof(reason));
            printf("  %s %-22s - %s\n", warns[i].severity == 2 ? "[ALLERGY]" : "[DIET]   ",
                   warns[i].name, reason);
        }
    }
    if (unknown)
        printf("%d ingredient(s) could not be identified - check them manually.\n", unknown);
    printf("\nNote: informational only. Always read the actual label; with severe\n"
           "allergies consult a doctor.\n");

    addScan(product, nw);
}
