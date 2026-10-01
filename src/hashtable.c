/* hashtable.c - Member A
 * Ingredient database stored as a hash table with chaining. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

Ingredient *dbTable[TABLE_SIZE];
static int dbSize = 0;

/* djb2 string hash */
static unsigned int hashStr(const char *s) {
    unsigned int h = 5381;
    while (*s) h = h * 33u + (unsigned char)(*s++);
    return h % TABLE_SIZE;
}

Ingredient *searchIngredient(const char *name) {
    Ingredient *p = dbTable[hashStr(name)];
    while (p) {
        if (strcmp(p->name, name) == 0) return p;
        p = p->next;
    }
    return NULL;
}

static void insertIngredient(const char *name, unsigned int tags, const char *expl) {
    if (searchIngredient(name)) return;               /* ignore duplicates */
    Ingredient *n = (Ingredient *)malloc(sizeof(Ingredient));
    if (!n) return;
    strncpy(n->name, name, NAME_LEN - 1);   n->name[NAME_LEN - 1] = '\0';
    strncpy(n->explanation, expl, EXPL_LEN - 1); n->explanation[EXPL_LEN - 1] = '\0';
    n->tags = tags & ALL_FLAGS;
    unsigned int h = hashStr(n->name);
    n->next = dbTable[h];                             /* insert at head of bucket */
    dbTable[h] = n;
    dbSize++;
}

/* CSV line format:  name,tags,explanation   (explanation may contain commas) */
int loadDB(const char *file) {
    FILE *fp = fopen(file, "r");
    if (!fp) return 0;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char *s = trim(line);
        if (*s == '\0' || *s == '#') continue;
        char *c1 = strchr(s, ',');
        if (!c1) continue;
        char *c2 = strchr(c1 + 1, ',');
        if (!c2) continue;
        *c1 = '\0'; *c2 = '\0';
        char *name = trim(s);
        for (char *q = name; *q; q++) *q = (char)tolower((unsigned char)*q);
        unsigned int tags = (unsigned int)strtoul(c1 + 1, NULL, 10);
        insertIngredient(name, tags, trim(c2 + 1));
    }
    fclose(fp);
    return dbSize;
}

int dbCount(void) { return dbSize; }

void freeDB(void) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Ingredient *p = dbTable[i];
        while (p) { Ingredient *t = p->next; free(p); p = t; }
        dbTable[i] = NULL;
    }
    dbSize = 0;
}
