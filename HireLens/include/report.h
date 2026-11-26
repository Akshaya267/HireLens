#ifndef REPORT_H
#define REPORT_H

#include "hirelens.h"

/* Writes a human-readable plain text summary report (all candidates,
 * ranked, with full explainability detail) to filepath. */
void write_text_report(const char *filepath, const JobDescription *jd,
                        CandidateResult *results, int count);

#endif /* REPORT_H */
