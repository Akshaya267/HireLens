/* ============================================================
 * main.c
 * HireLens Engine - CLI entry point.
 *
 * Pipeline:
 *   Load JD -> Load Resumes -> Match -> Score -> Rank -> Explain
 *   -> Write JSON (per candidate + full ranking) -> Write text report
 *
 * Usage:
 *   hirelens_engine <jd_file> <resumes_dir> <output_dir>
 *
 * Example:
 *   ./bin/hirelens_engine data/jd/job_description.txt data/resumes output
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "hirelens.h"
#include "jd_parser.h"
#include "resume_parser.h"
#include "matcher.h"
#include "scorer.h"
#include "ranker.h"
#include "explain.h"
#include "json_writer.h"
#include "report.h"

static void ensure_dir(const char *path) {
#if defined(_WIN32)
    mkdir(path);
#else
    mkdir(path, 0777);
#endif
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "HireLens Resume Analysis Engine\n");
        fprintf(stderr, "Usage: %s <jd_file> <resumes_dir> <output_dir>\n", argv[0]);
        return 1;
    }

    const char *jd_file = argv[1];
    const char *resumes_dir = argv[2];
    const char *output_dir = argv[3];

    ensure_dir(output_dir);

    /* ---- 1. Load Job Description ---- */
    JobDescription jd;
    if (load_job_description(jd_file, &jd) != 0) {
        fprintf(stderr, "[FATAL] Failed to load job description.\n");
        return 1;
    }
    printf("[OK] Loaded JD: %s (%d required, %d preferred skills)\n",
           jd.title, jd.required_count, jd.preferred_count);

    /* ---- 2. Load Resumes ---- */
    static Resume resumes[MAX_CANDIDATES];
    int resume_count = load_resumes_from_dir(resumes_dir, resumes, MAX_CANDIDATES);
    if (resume_count <= 0) {
        fprintf(stderr, "[FATAL] No resumes could be loaded from %s\n", resumes_dir);
        return 1;
    }
    printf("[OK] Loaded %d resume(s) from %s\n", resume_count, resumes_dir);

    /* ---- 3. Match + Score + Explain each candidate ---- */
    static CandidateResult results[MAX_CANDIDATES];
    for (int i = 0; i < resume_count; i++) {
        results[i].resume = resumes[i];
        run_matching(&jd, &results[i]);
        compute_scores(&jd, &results[i]);
        generate_explanation(&jd, &results[i]);
    }

    /* ---- 4. Rank ---- */
    rank_candidates(results, resume_count);

    /* ---- 5. Write outputs ---- */
    char path[512];

    snprintf(path, sizeof(path), "%s/ranking.json", output_dir);
    FILE *rank_fp = fopen(path, "w");
    if (rank_fp) {
        json_write_ranking(rank_fp, &jd, results, resume_count);
        fclose(rank_fp);
        printf("[OK] Wrote %s\n", path);
    } else {
        fprintf(stderr, "[ERROR] Could not write %s\n", path);
    }

    for (int i = 0; i < resume_count; i++) {
        snprintf(path, sizeof(path), "%s/candidate_%d.json", output_dir, results[i].rank);
        FILE *cand_fp = fopen(path, "w");
        if (cand_fp) {
            json_write_candidate(cand_fp, &results[i], 0);
            fputc('\n', cand_fp);
            fclose(cand_fp);
        }
    }
    printf("[OK] Wrote %d individual candidate JSON file(s)\n", resume_count);

    snprintf(path, sizeof(path), "%s/report.txt", output_dir);
    write_text_report(path, &jd, results, resume_count);
    printf("[OK] Wrote %s\n", path);

    /* ---- 6. Console summary ---- */
    printf("\n================ CANDIDATE RANKING ================\n");
    printf("%-5s %-24s %-8s %-20s\n", "Rank", "Candidate", "Score", "Status");
    printf("-----------------------------------------------------\n");
    for (int i = 0; i < resume_count; i++) {
        printf("%-5d %-24s %-8.1f %-20s\n",
               results[i].rank, results[i].resume.name, results[i].overall_score, results[i].status);
    }
    printf("=====================================================\n");

    return 0;
}
