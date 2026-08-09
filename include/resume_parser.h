#ifndef RESUME_PARSER_H
#define RESUME_PARSER_H

#include "hirelens.h"

/* Loads and parses a single structured resume text file into `resume`.
 * Returns 0 on success, -1 on failure. */
int load_resume(const char *filepath, Resume *resume);

/* Scans `dir_path` for *.txt resume files and loads each one into
 * `resumes` (capacity `max_resumes`). Returns the number successfully
 * loaded, or -1 if the directory could not be opened. */
int load_resumes_from_dir(const char *dir_path, Resume *resumes, int max_resumes);

#endif /* RESUME_PARSER_H */
