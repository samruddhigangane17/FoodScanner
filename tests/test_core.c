/* test_core.c - quick automated checks for every module.
 * Run from the project root:  make test */
#include <stdio.h>
#include <string.h>
#include "common.h"

static int passed = 0, failed = 0;
#define CHECK(cond, msg) do { if (cond) { passed++; } else { failed++; printf("FAIL: %s\n", msg); } } while (0)

int main(void) {
    /* ---- Member A : database + fuzzy match ---- */
    int n = loadDB(DB_FILE);
    CHECK(n >= 150, "database loads 150+ entries");
    Ingredient *s = searchIngredient("sugar");
    CHECK(s && (s->tags & SUGAR), "sugar found with SUGAR tag");
    CHECK(searchIngredient("unobtainium") == NULL, "unknown ingredient returns NULL");
    CHECK(editDistance("kitten", "sitting") == 3, "edit distance kitten/sitting = 3");
    CHECK(editDistance("sugar", "sugar") == 0, "edit distance of equal words = 0");
    Ingredient *c = closestMatch("sugr", 2);
    CHECK(c && strcmp(c->name, "sugar") == 0, "'sugr' corrected to sugar");
    CHECK(closestMatch("xyzqwk", 2) == NULL, "garbage word not matched");

    /* ---- Member B : cleaner + tokenizer ---- */
    char text[MAX_TEXT] = "INGREDIENTS: Wheat Flour, Sug-\nar; Lecithin (E322)\nNutrition Information per 100g";
    cleanText(text);
    CHECK(strcmp(text, "wheat flour, sugar, lecithin (e322)") == 0, "cleanText output");

    static char items[MAX_ITEMS][NAME_LEN];
    int k = splitIngredients("wheat flour (gluten), sugar (14%), emulsifier (ins 322, e471), contains: milk.",
                             items, MAX_ITEMS);
    CHECK(k == 7, "tokenizer finds 7 items");
    CHECK(k == 7 && strcmp(items[1], "gluten") == 0, "bracket content split out");
    CHECK(k == 7 && strcmp(items[2], "sugar") == 0, "percentage removed");
    CHECK(k == 7 && strcmp(items[4], "e322") == 0, "'ins 322' becomes e322");
    CHECK(k == 7 && strcmp(items[6], "milk") == 0, "'contains:' prefix removed");

    /* ---- Member C : bitmask warnings ---- */
    Ingredient *milk = searchIngredient("milk powder");
    Ingredient *lec  = searchIngredient("lecithin");
    unsigned int profile = DAIRY | SUGAR;
    CHECK(milk && checkWarnings(milk, profile) == DAIRY, "milk powder conflicts with dairy-free");
    CHECK(lec && checkWarnings(lec, profile) == 0, "lecithin fine for dairy/sugar profile");
    CHECK(lec && checkWarnings(lec, SOY) == SOY, "lecithin conflicts with soy allergy");
    Ingredient *soy = searchIngredient("soy sauce");
    CHECK(soy && checkWarnings(soy, GLUTEN | SOY) == (GLUTEN | SOY), "soy sauce hits two flags");

    Warning w[4] = { {"a",1,1}, {"b",2,2}, {"c",1,1}, {"d",2,2} };
    mergeSortWarnings(w, 4);
    CHECK(w[0].severity == 2 && w[1].severity == 2 && w[2].severity == 1, "sorted by severity");
    CHECK(strcmp(w[0].name, "b") == 0 && strcmp(w[2].name, "a") == 0, "sort is stable");

    freeDB();
    printf("\n%d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
