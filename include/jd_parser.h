#ifndef JD_PARSER_H
#define JD_PARSER_H

#include "hirelens.h"

/* Loads and parses a structured Job Description text file into `jd`.
 * Returns 0 on success, -1 on failure (e.g. file not found). */
int load_job_description(const char *filepath, JobDescription *jd);

#endif /* JD_PARSER_H */
