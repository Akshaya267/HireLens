#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include <stdio.h>
#include "hirelens.h"

/* Writes a properly-escaped JSON string literal (including quotes) to fp */
void json_write_string(FILE *fp, const char *s);

/* Writes one candidate's full result as a JSON object to fp */
void json_write_candidate(FILE *fp, const CandidateResult *r, int indent);

/* Writes the full ranking array (all candidates) + JD summary as one
 * JSON document to fp */
void json_write_ranking(FILE *fp, const JobDescription *jd,
                         CandidateResult *results, int count);

#endif /* JSON_WRITER_H */
