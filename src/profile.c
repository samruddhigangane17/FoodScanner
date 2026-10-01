/* profile.c - Member C
 * User restriction profile stored as ONE integer (bitmask). */
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

const unsigned int FLAG_LIST[NUM_FLAGS] = { NUTS, DAIRY, GLUTEN, SOY, NON_VEG, SUGAR };

const char *FLAG_NAMES[NUM_FLAGS] = {
    "Nut allergy (peanuts and tree nuts)",
    "Dairy-free / lactose intolerant",
    "Gluten-free (wheat, barley, rye)",
    "Soy allergy",
    "Vegetarian / vegan (no meat, fish or animal-derived items)",
    "Diabetic / avoiding sugar"
};

const char *FLAG_REASONS[NUM_FLAGS] = {
    "contains nuts", "contains dairy", "contains gluten",
    "contains soy", "not vegetarian", "contains sugar"
};

unsigned int loadProfile(void) {
    FILE *fp = fopen(PROFILE_FILE, "r");
    unsigned int v = 0;
    if (fp) {
        if (fscanf(fp, "%u", &v) != 1) v = 0;
        fclose(fp);
    }
    return v & ALL_FLAGS;
}

void saveProfile(unsigned int profile) {
    FILE *fp = fopen(PROFILE_FILE, "w");
    if (!fp) { printf("Could not save profile.\n"); return; }
    fprintf(fp, "%u\n", profile & ALL_FLAGS);
    fclose(fp);
}

void printProfile(unsigned int profile) {
    printf("\nYour profile:\n");
    int any = 0;
    for (int i = 0; i < NUM_FLAGS; i++)
        if (profile & FLAG_LIST[i]) { printf("  [x] %s\n", FLAG_NAMES[i]); any = 1; }
    if (!any) printf("  (no restrictions selected)\n");
}

unsigned int editProfile(unsigned int profile) {
    char buf[32];
    for (;;) {
        printf("\n=== Edit profile (type a number to turn it on/off) ===\n");
        for (int i = 0; i < NUM_FLAGS; i++)
            printf("  %d. [%c] %s\n", i + 1, (profile & FLAG_LIST[i]) ? 'x' : ' ', FLAG_NAMES[i]);
        printf("  0. Save and go back\n");
        readLine("Choice: ", buf, sizeof(buf));
        if (feof(stdin)) break;
        int c = atoi(buf);
        if (c == 0 && buf[0] == '0') break;
        if (c >= 1 && c <= NUM_FLAGS) profile ^= FLAG_LIST[c - 1];   /* toggle bit */
        else printf("Please enter 0 to %d.\n", NUM_FLAGS);
    }
    saveProfile(profile);
    return profile;
}
