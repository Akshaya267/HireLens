#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

/* Lowercase a string in place */
void str_to_lower(char *s);

/* Trim leading/trailing whitespace in place */
void str_trim(char *s);

/* Replace any character that is not alnum/space/hyphen/plus/hash with a space.
 * Keeps + and # so "C++" / "C#" survive normalization. */
void str_clean_punct(char *s);

/* Case-insensitive substring search. Returns 1 if needle found in haystack. */
int str_contains_ci(const char *haystack, const char *needle);

/* Case-insensitive exact string comparison. Returns 1 if equal. */
int str_equals_ci(const char *a, const char *b);

/* Split `input` on `delim`, trims each token, writes up to `max` tokens
 * into out[][MAX_SKILL_LEN]. Returns number of tokens written. */
int split_and_trim(const char *input, char delim, char out[][128], int max);

/* Returns 1 if the line looks like a "KEY: value" header line
 * (all-caps/underscore key followed by a colon). */
int is_key_line(const char *line, char *key_out, int key_out_size, const char **value_start);

/* Safe string copy that always null terminates */
void safe_strcpy(char *dst, const char *src, int dst_size);

#endif /* TEXT_UTILS_H */
