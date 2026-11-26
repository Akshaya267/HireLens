/* ============================================================
 * jd_parser.c
 * Parses a structured Job Description file of the form:
 *
 *   TITLE: Backend Software Engineer
 *   REQUIRED_SKILLS: Java, Python, SQL, DSA, DBMS
 *   PREFERRED_SKILLS: Docker, Kubernetes, AWS, ML
 *   MIN_EXPERIENCE_YEARS: 2
 *   EDUCATION: B.Tech Computer Science, BCA
 *   KEYWORDS: scalable, microservices, agile, teamwork
 *
 * into a JobDescription struct, applying synonym normalization
 * to every skill/education token as it is parsed.
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jd_parser.h"
#include "text_utils.h"
#include "synonyms.h"

static void add_skill_list(Skill *arr, int *count, const char *csv, int max) {
    char tokens[MAX_SKILLS][128];
    int n = split_and_trim(csv, ',', tokens, max);
    for (int i = 0; i < n && *count < max; i++) {
        safe_strcpy(arr[*count].raw, tokens[i], MAX_SKILL_LEN);
        normalize_term(tokens[i], arr[*count].canonical, MAX_SKILL_LEN);
        (*count)++;
    }
}

int load_job_description(const char *filepath, JobDescription *jd) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "[ERROR] Could not open JD file: %s\n", filepath);
        return -1;
    }

    memset(jd, 0, sizeof(JobDescription));
    safe_strcpy(jd->title, "Untitled Position", MAX_NAME);

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        /* strip newline */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        char key[64];
        const char *value_start = NULL;
        if (!is_key_line(line, key, sizeof(key), &value_start)) {
            continue; /* blank/comment/non-key lines are ignored */
        }

        char value[MAX_LINE];
        safe_strcpy(value, value_start, sizeof(value));
        str_trim(value);

        if (strcmp(key, "TITLE") == 0) {
            safe_strcpy(jd->title, value, MAX_NAME);
        } else if (strcmp(key, "REQUIRED_SKILLS") == 0) {
            add_skill_list(jd->required_skills, &jd->required_count, value, MAX_SKILLS);
        } else if (strcmp(key, "PREFERRED_SKILLS") == 0) {
            add_skill_list(jd->preferred_skills, &jd->preferred_count, value, MAX_SKILLS);
        } else if (strcmp(key, "MIN_EXPERIENCE_YEARS") == 0) {
            jd->min_experience_years = atoi(value);
        } else if (strcmp(key, "EDUCATION") == 0) {
            char tokens[MAX_EDU][128];
            int n = split_and_trim(value, ',', tokens, MAX_EDU);
            for (int i = 0; i < n; i++) {
                safe_strcpy(jd->education[jd->education_count], tokens[i], MAX_SKILL_LEN);
                jd->education_count++;
            }
        } else if (strcmp(key, "KEYWORDS") == 0) {
            char tokens[MAX_KEYWORDS][128];
            int n = split_and_trim(value, ',', tokens, MAX_KEYWORDS);
            for (int i = 0; i < n; i++) {
                safe_strcpy(jd->keywords[jd->keyword_count], tokens[i], MAX_SKILL_LEN);
                jd->keyword_count++;
            }
        }
        /* Unknown keys are silently ignored -> forward compatible format */
    }

    fclose(fp);

    if (jd->required_count == 0) {
        fprintf(stderr, "[WARN] JD file '%s' has no REQUIRED_SKILLS specified.\n", filepath);
    }
    return 0;
}
