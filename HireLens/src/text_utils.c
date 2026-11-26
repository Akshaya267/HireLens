/* ============================================================
 * text_utils.c
 * Low level text normalization primitives used throughout the
 * engine: lowercasing, trimming, punctuation cleanup, CSV style
 * splitting, and simple case-insensitive matching.
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "text_utils.h"

void str_to_lower(char *s) {
    if (!s) return;
    for (int i = 0; s[i]; i++) {
        s[i] = (char)tolower((unsigned char)s[i]);
    }
}

void str_trim(char *s) {
    if (!s) return;
    int len = (int)strlen(s);
    int start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;

    int end = len - 1;
    while (end >= start && isspace((unsigned char)s[end])) end--;

    int new_len = end - start + 1;
    if (new_len < 0) new_len = 0;

    if (start > 0) memmove(s, s + start, (size_t)new_len);
    s[new_len] = '\0';
}

void str_clean_punct(char *s) {
    if (!s) return;
    for (int i = 0; s[i]; i++) {
        unsigned char c = (unsigned char)s[i];
        if (isalnum(c) || c == ' ' || c == '-' || c == '+' || c == '#') {
            continue;
        }
        s[i] = ' ';
    }
}

int str_contains_ci(const char *haystack, const char *needle) {
    if (!haystack || !needle || !*needle) return 0;

    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    if (nlen > hlen) return 0;

    char *h = (char *)malloc(hlen + 1);
    char *n = (char *)malloc(nlen + 1);
    if (!h || !n) { free(h); free(n); return 0; }

    strcpy(h, haystack);
    strcpy(n, needle);
    str_to_lower(h);
    str_to_lower(n);

    int found = strstr(h, n) != NULL;
    free(h);
    free(n);
    return found;
}

int str_equals_ci(const char *a, const char *b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

int split_and_trim(const char *input, char delim, char out[][128], int max) {
    if (!input) return 0;
    int count = 0;
    char buf[4096];
    safe_strcpy(buf, input, (int)sizeof(buf));

    char *token = strtok(buf, (char[]){delim, '\0'});
    while (token && count < max) {
        char tmp[128];
        safe_strcpy(tmp, token, (int)sizeof(tmp));
        str_trim(tmp);
        if (strlen(tmp) > 0) {
            safe_strcpy(out[count], tmp, 128);
            count++;
        }
        token = strtok(NULL, (char[]){delim, '\0'});
    }
    return count;
}

int is_key_line(const char *line, char *key_out, int key_out_size, const char **value_start) {
    if (!line) return 0;
    const char *colon = strchr(line, ':');
    if (!colon) return 0;

    int key_len = (int)(colon - line);
    if (key_len <= 0 || key_len >= key_out_size) return 0;

    /* A valid "key" is composed only of uppercase letters and underscores */
    for (int i = 0; i < key_len; i++) {
        char c = line[i];
        if (!(isupper((unsigned char)c) || c == '_')) {
            return 0;
        }
    }

    strncpy(key_out, line, (size_t)key_len);
    key_out[key_len] = '\0';
    if (value_start) *value_start = colon + 1;
    return 1;
}

void safe_strcpy(char *dst, const char *src, int dst_size) {
    if (!dst || dst_size <= 0) return;
    if (!src) { dst[0] = '\0'; return; }
    strncpy(dst, src, (size_t)(dst_size - 1));
    dst[dst_size - 1] = '\0';
}
