/* ============================================================
 * report.c
 * Generates a human-readable, exportable plain-text report --
 * this is what a recruiter would actually print/save/share.
 * ============================================================ */
#include <stdio.h>
#include <time.h>
#include "report.h"

void write_text_report(const char *filepath, const JobDescription *jd,
                        CandidateResult *results, int count) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        fprintf(stderr, "[ERROR] Could not write report to %s\n", filepath);
        return;
    }

    time_t now = time(NULL);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(fp, "================================================================\n");
    fprintf(fp, "  HireLens - Intelligent Resume Analyzer\n");
    fprintf(fp, "  Candidate Ranking Report\n");
    fprintf(fp, "  Generated: %s\n", timebuf);
    fprintf(fp, "================================================================\n\n");

    fprintf(fp, "Job Description : %s\n", jd->title);
    fprintf(fp, "Required Skills  : %d   Preferred Skills: %d\n", jd->required_count, jd->preferred_count);
    fprintf(fp, "Min. Experience  : %d year(s)\n", jd->min_experience_years);
    fprintf(fp, "Candidates Ranked: %d\n\n", count);

    fprintf(fp, "----------------------------------------------------------------\n");
    fprintf(fp, "%-5s %-24s %-8s %-20s\n", "Rank", "Candidate", "Score", "Status");
    fprintf(fp, "----------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%-5d %-24s %-8.1f %-20s\n",
                results[i].rank, results[i].resume.name, results[i].overall_score, results[i].status);
    }
    fprintf(fp, "----------------------------------------------------------------\n\n");

    for (int i = 0; i < count; i++) {
        CandidateResult *r = &results[i];
        fprintf(fp, "================================================================\n");
        fprintf(fp, "#%d  %s  (%s)\n", r->rank, r->resume.name, r->status);
        fprintf(fp, "================================================================\n");
        fprintf(fp, "Email: %s   Phone: %s   Experience: %d yr(s)\n",
                r->resume.email, r->resume.phone, r->resume.experience_years);
        fprintf(fp, "Source: %s\n\n", r->resume.source_file);

        fprintf(fp, "Overall Score      : %.1f / 100\n", r->overall_score);
        fprintf(fp, "  Skill Score      : %.1f  (weight 40%%)\n", r->skill_score);
        fprintf(fp, "  Experience Score : %.1f  (weight 20%%)\n", r->experience_score);
        fprintf(fp, "  Education Score  : %.1f  (weight 15%%)\n", r->education_score);
        fprintf(fp, "  Project Score    : %.1f  (weight 15%%)\n", r->project_score);
        fprintf(fp, "  Keyword Score    : %.1f  (weight 10%%)\n\n", r->keyword_score);

        fprintf(fp, "Matched Required Skills (%d):\n", r->matched_required_count);
        for (int j = 0; j < r->matched_required_count; j++) {
            fprintf(fp, "  - %s%s\n", r->matched_required[j].skill,
                    r->matched_required[j].via_synonym ? "  [matched via synonym]" : "");
        }
        if (r->matched_required_count == 0) fprintf(fp, "  (none)\n");

        fprintf(fp, "Missing Required Skills (%d):\n", r->missing_required_count);
        for (int j = 0; j < r->missing_required_count; j++) {
            fprintf(fp, "  - %s\n", r->missing_required[j]);
        }
        if (r->missing_required_count == 0) fprintf(fp, "  (none)\n");

        fprintf(fp, "Matched Preferred Skills (%d):\n", r->matched_preferred_count);
        for (int j = 0; j < r->matched_preferred_count; j++) {
            fprintf(fp, "  - %s%s\n", r->matched_preferred[j].skill,
                    r->matched_preferred[j].via_synonym ? "  [matched via synonym]" : "");
        }
        if (r->matched_preferred_count == 0) fprintf(fp, "  (none)\n");

        fprintf(fp, "\nStrengths:\n");
        for (int j = 0; j < r->strengths_count; j++) fprintf(fp, "  + %s\n", r->strengths[j]);

        fprintf(fp, "\nGaps:\n");
        for (int j = 0; j < r->gaps_count; j++) fprintf(fp, "  - %s\n", r->gaps[j]);

        fprintf(fp, "\nRecommendation:\n  %s\n\n", r->recommendation);
    }

    fclose(fp);
}
