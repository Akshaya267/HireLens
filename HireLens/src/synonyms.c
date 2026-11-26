/* ============================================================
 * synonyms.c
 * Hard coded synonym / abbreviation dictionary that lets the
 * matching engine treat equivalent skill names as identical,
 * e.g. "ML" == "Machine Learning", "DSA" == "Data Structures".
 *
 * This is deliberately a plain static lookup table (no external
 * NLP/thesaurus libraries) -- pure rule based normalization.
 * ============================================================ */
#include <string.h>
#include "synonyms.h"
#include "text_utils.h"

typedef struct {
    const char *variant;
    const char *canonical;
} SynonymEntry;

/* variant (any casing, matched case-insensitively) -> canonical form */
static const SynonymEntry SYNONYM_TABLE[] = {
    /* --- Core CS subjects --- */
    {"ml",          "machine learning"},
    {"machine learning", "machine learning"},
    {"dsa",         "data structures and algorithms"},
    {"data structures", "data structures and algorithms"},
    {"data structures and algorithms", "data structures and algorithms"},
    {"algorithms",  "data structures and algorithms"},
    {"dbms",        "database management system"},
    {"database management system", "database management system"},
    {"database",    "database management system"},
    {"databases",   "database management system"},
    {"rdbms",       "relational database"},
    {"relational database", "relational database"},
    {"oop",         "object oriented programming"},
    {"object oriented programming", "object oriented programming"},
    {"oops",        "object oriented programming"},
    {"os",          "operating systems"},
    {"operating system", "operating systems"},
    {"operating systems", "operating systems"},
    {"cn",          "computer networks"},
    {"computer network", "computer networks"},
    {"computer networks", "computer networks"},
    {"networking",  "computer networks"},
    {"se",          "software engineering"},
    {"software engineering", "software engineering"},
    {"ai",          "artificial intelligence"},
    {"artificial intelligence", "artificial intelligence"},
    {"dl",          "deep learning"},
    {"deep learning", "deep learning"},
    {"nlp",         "natural language processing"},
    {"natural language processing", "natural language processing"},
    {"cv",          "computer vision"},
    {"computer vision", "computer vision"},

    /* --- Languages --- */
    {"py",          "python"},
    {"python",      "python"},
    {"js",          "javascript"},
    {"javascript",  "javascript"},
    {"ts",          "typescript"},
    {"typescript",  "typescript"},
    {"c++",         "c++"},
    {"cpp",         "c++"},
    {"c#",          "c#"},
    {"csharp",      "c#"},
    {"golang",      "go"},
    {"go",          "go"},

    /* --- Web / frameworks --- */
    {"reactjs",     "react"},
    {"react.js",    "react"},
    {"react",       "react"},
    {"nodejs",      "node.js"},
    {"node",        "node.js"},
    {"node.js",     "node.js"},
    {"vuejs",       "vue"},
    {"vue.js",      "vue"},
    {"vue",         "vue"},
    {"expressjs",   "express"},
    {"express.js",  "express"},
    {"html5",       "html"},
    {"html",        "html"},
    {"css3",        "css"},
    {"css",         "css"},
    {"restapi",     "rest api"},
    {"rest apis",   "rest api"},
    {"restful api", "rest api"},
    {"rest api",    "rest api"},
    {"api",         "rest api"},

    /* --- Data / query --- */
    {"sql",         "structured query language"},
    {"structured query language", "structured query language"},
    {"nosql",       "nosql database"},
    {"mongodb",     "mongodb"},
    {"mysql",       "mysql"},
    {"postgres",    "postgresql"},
    {"postgresql",  "postgresql"},

    /* --- Cloud / DevOps --- */
    {"aws",         "amazon web services"},
    {"amazon web services", "amazon web services"},
    {"gcp",         "google cloud platform"},
    {"google cloud", "google cloud platform"},
    {"google cloud platform", "google cloud platform"},
    {"azure",       "microsoft azure"},
    {"k8s",         "kubernetes"},
    {"kubernetes",  "kubernetes"},
    {"docker",      "docker"},
    {"ci/cd",       "continuous integration and deployment"},
    {"cicd",        "continuous integration and deployment"},
    {"ci cd",       "continuous integration and deployment"},

    /* --- Practices --- */
    {"agile",       "agile methodology"},
    {"scrum",       "agile methodology"},
    {"agile methodology", "agile methodology"},
    {"oop design",  "object oriented programming"},
    {"vcs",         "version control"},
    {"git",         "version control"},
    {"github",      "version control"},
    {"version control", "version control"},

    /* --- Education abbreviations --- */
    {"btech",       "bachelor of technology"},
    {"b.tech",      "bachelor of technology"},
    {"be",          "bachelor of engineering"},
    {"b.e",         "bachelor of engineering"},
    {"bachelor of engineering", "bachelor of engineering"},
    {"bca",         "bachelor of computer applications"},
    {"mca",         "master of computer applications"},
    {"mtech",       "master of technology"},
    {"m.tech",      "master of technology"},
    {"me",          "master of engineering"},
    {"msc",         "master of science"},
    {"bsc",         "bachelor of science"},
};

static const int SYNONYM_TABLE_SIZE = (int)(sizeof(SYNONYM_TABLE) / sizeof(SYNONYM_TABLE[0]));

void normalize_term(const char *raw, char *out, int out_size) {
    char buf[256];
    safe_strcpy(buf, raw, (int)sizeof(buf));
    str_trim(buf);
    str_to_lower(buf);

    for (int i = 0; i < SYNONYM_TABLE_SIZE; i++) {
        if (strcmp(buf, SYNONYM_TABLE[i].variant) == 0) {
            safe_strcpy(out, SYNONYM_TABLE[i].canonical, out_size);
            return;
        }
    }
    /* No dictionary entry: canonical form is just the cleaned text itself */
    safe_strcpy(out, buf, out_size);
}

int is_known_synonym(const char *raw) {
    char buf[256];
    safe_strcpy(buf, raw, (int)sizeof(buf));
    str_trim(buf);
    str_to_lower(buf);

    for (int i = 0; i < SYNONYM_TABLE_SIZE; i++) {
        if (strcmp(buf, SYNONYM_TABLE[i].variant) == 0) {
            return strcmp(SYNONYM_TABLE[i].variant, SYNONYM_TABLE[i].canonical) != 0;
        }
    }
    return 0;
}
