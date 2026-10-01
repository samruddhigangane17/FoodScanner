/* common.h - shared definitions for the Food Label Scanner
 * Agreed by all members on Day 1. Change only after team discussion. */
#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

/* ---------- sizes and file paths ---------- */
#define TABLE_SIZE 101
#define NAME_LEN   50
#define EXPL_LEN   200
#define MAX_ITEMS  100
#define MAX_TEXT   4096

#define DB_FILE      "data/ingredients.csv"
#define PROFILE_FILE "data/profile.txt"
#define HISTORY_FILE "data/history.txt"
#define OCR_BASE     "ocr_out"          /* tesseract writes ocr_out.txt */

/* ---------- restriction bit flags (bitmask) ---------- */
#define NUTS     (1u << 0)
#define DAIRY    (1u << 1)
#define GLUTEN   (1u << 2)
#define SOY      (1u << 3)
#define NON_VEG  (1u << 4)
#define SUGAR    (1u << 5)
#define NUM_FLAGS 6
#define ALL_FLAGS (NUTS | DAIRY | GLUTEN | SOY | NON_VEG | SUGAR)

/* ---------- data structures ---------- */
typedef struct Ingredient {          /* hash table node (chaining) */
    char name[NAME_LEN];
    char explanation[EXPL_LEN];
    unsigned int tags;               /* restriction bitmask */
    struct Ingredient *next;
} Ingredient;

typedef struct Warning {             /* one warning shown to the user */
    char name[NAME_LEN];
    unsigned int conflict;           /* bits that clash with the profile */
    int severity;                    /* 2 = allergy type, 1 = diet type */
} Warning;

typedef struct Scan {                /* history linked list node */
    char product[NAME_LEN];
    int warnings;
    char when[32];
    struct Scan *next;
} Scan;

extern Ingredient *dbTable[TABLE_SIZE];
extern const unsigned int FLAG_LIST[NUM_FLAGS];
extern const char *FLAG_NAMES[NUM_FLAGS];     /* shown in profile menu */
extern const char *FLAG_REASONS[NUM_FLAGS];   /* shown in warnings */

/* ================= Member A : hashtable.c / editdistance.c ================= */
int         loadDB(const char *file);                    /* returns entries loaded */
Ingredient *searchIngredient(const char *name);          /* exact match or NULL */
int         dbCount(void);
void        freeDB(void);
int         editDistance(const char *a, const char *b);  /* Levenshtein */
Ingredient *closestMatch(const char *name, int maxDist); /* fuzzy match or NULL */

/* ================= Member B : ocr.c / cleaner.c / tokenizer.c ============== */
int  runOCR(const char *imagePath, char *out, int maxLen);   /* 1 = ok, 0 = fail */
void cleanText(char *text);                                  /* in place */
int  splitIngredients(const char *text, char items[][NAME_LEN], int maxItems);

/* ================= Member C : profile / analyser / sort / history ========== */
unsigned int loadProfile(void);
void         saveProfile(unsigned int profile);
void         printProfile(unsigned int profile);
unsigned int editProfile(unsigned int profile);              /* interactive menu */
unsigned int checkWarnings(const Ingredient *ing, unsigned int profile);
int          severityOf(unsigned int conflict);              /* 2 allergy, 1 diet, 0 none */
void         runAnalysis(const char *product, char items[][NAME_LEN], int n,
                         unsigned int profile);
void         mergeSortWarnings(Warning *w, int n);           /* severity, high first */
void         addScan(const char *product, int warnings);
void         loadHistory(void);
void         showHistory(void);
void         freeHistory(void);
void         historyJson(void);                              /* history as JSON (web UI) */

/* ================= web frontend bridge : api.c ============================= */
int  apiMain(int argc, char **argv);     /* scanner --api ... prints JSON */
void jsonString(const char *s);

void         printHistoryJson(void);

/* ================= JSON mode (jsonout.c) ================================== */
void jsonString(const char *s);
void jsonAnalyse(const char *product, const char *rawText, unsigned int profile, int showRaw);

/* ================= shared helpers : util.c ================================= */
void  readLine(const char *prompt, char *buf, size_t size);
char *trim(char *s);

#endif
