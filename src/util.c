/* util.c - small helpers shared by all modules */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

/* remove leading/trailing whitespace (in place, returns pointer into s) */
char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
    return s;
}

/* print a prompt and read one line safely (no newline kept) */
void readLine(const char *prompt, char *buf, size_t size) {
    if (prompt) { printf("%s", prompt); fflush(stdout); }
    if (!fgets(buf, (int)size, stdin)) { buf[0] = '\0'; return; }
    size_t n = strlen(buf);
    if (n > 0 && buf[n - 1] == '\n') buf[n - 1] = '\0';
    else { int c; while ((c = getchar()) != '\n' && c != EOF) ; }  /* drop long rest */
}
