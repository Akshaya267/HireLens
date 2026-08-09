#ifndef SYNONYMS_H
#define SYNONYMS_H

/* Normalizes a raw skill/term (e.g. "ML", "Dsa", "node.js") into its
 * canonical lowercase form (e.g. "machine learning", "data structures
 * and algorithms", "node.js") using a hard-coded synonym dictionary.
 * If no synonym entry exists, the canonical form is simply the
 * lowercased & trimmed input. Result written into `out` (size out_size). */
void normalize_term(const char *raw, char *out, int out_size);

/* Returns 1 if `raw` has a distinct dictionary entry mapping it to a
 * canonical form different from its own trimmed/lowercased text
 * (i.e. this was a genuine synonym expansion, not an identity match). */
int is_known_synonym(const char *raw);

#endif /* SYNONYMS_H */
