/* ============================================================
 * resume_parser.c
 * Parses a semi-structured resume text file of the form:
 *
 *   NAME: Aisha Sharma
 *   EMAIL: aisha@example.com
 *   PHONE: 9876543210
 *   EXPERIENCE_YEARS: 3
 *   EDUCATION: B.Tech Computer Science, XYZ University, 2021
 *   SKILLS: Java, Python, Machine Learning, SQL, Git
 *   PROJECTS: Built a scalable e-commerce backend using Java ...
 *             (may continue across multiple lines)
 *   SUMMARY: Backend engineer with 3 years experience ...
 *
 * Multi-line fields (PROJECTS / SUMMARY) are accumulated until the
 * next recognized KEY: line or end of file. Also provides directory
 * scanning (via POSIX dirent.h, a standard C library facility) to
 * batch-load every resume in data/resumes/.
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "resume_parser.h"
#include "text_utils.h"
#include "synonyms.h"

typedef enum { FIELD_NONE, FIELD_PROJECTS, FIELD_SUMMARY } MultiLineField;

static void append_text(char *dst, int dst_size, const char *addition) {
    int cur_len = (int)strlen(dst);
    if (cur_len > 0 && cur_len < dst_size - 1) {
        strcat(dst, " ");
        cur_len++;
    }
    int space_left = dst_size - cur_len - 1;
    if (space_left > 0) {
        strncat(dst, addition, (size_t)space_left);
    }
}

int load_resume(const char *filepath, Resume *resume) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "[ERROR] Could not open resume file: %s\n", filepath);
        return -1;
    }

    memset(resume, 0, sizeof(Resume));
    safe_strcpy(resume->name, "Unknown Candidate", MAX_NAME);
    safe_strcpy(resume->source_file, filepath, sizeof(resume->source_file));

    MultiLineField active_field = FIELD_NONE;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        char key[64];
        const char *value_start = NULL;

        if (is_key_line(line, key, sizeof(key), &value_start)) {
            char value[MAX_LINE];
            safe_strcpy(value, value_start, sizeof(value));
            str_trim(value);
            active_field = FIELD_NONE;

            if (strcmp(key, "NAME") == 0) {
                safe_strcpy(resume->name, value, MAX_NAME);
            } else if (strcmp(key, "EMAIL") == 0) {
                safe_strcpy(resume->email, value, MAX_NAME);
            } else if (strcmp(key, "PHONE") == 0) {
                safe_strcpy(resume->phone, value, sizeof(resume->phone));
            } else if (strcmp(key, "EXPERIENCE_YEARS") == 0) {
                resume->experience_years = atoi(value);
            } else if (strcmp(key, "EDUCATION") == 0) {
                char tokens[MAX_EDU][128];
                int n = split_and_trim(value, ',', tokens, MAX_EDU);
                for (int i = 0; i < n; i++) {
                    safe_strcpy(resume->education[resume->education_count], tokens[i], MAX_SKILL_LEN);
                    resume->education_count++;
                }
            } else if (strcmp(key, "SKILLS") == 0) {
                char tokens[MAX_SKILLS][128];
                int n = split_and_trim(value, ',', tokens, MAX_SKILLS);
                for (int i = 0; i < n && resume->skill_count < MAX_SKILLS; i++) {
                    safe_strcpy(resume->skills[resume->skill_count].raw, tokens[i], MAX_SKILL_LEN);
                    normalize_term(tokens[i], resume->skills[resume->skill_count].canonical, MAX_SKILL_LEN);
                    resume->skill_count++;
                }
            } else if (strcmp(key, "PROJECTS") == 0) {
                active_field = FIELD_PROJECTS;
                append_text(resume->projects_text, MAX_TEXT, value);
            } else if (strcmp(key, "SUMMARY") == 0) {
                active_field = FIELD_SUMMARY;
                append_text(resume->summary_text, MAX_TEXT, value);
            }
            /* unknown key -> ignored, forward compatible */
        } else {
            /* Continuation line for a multi-line field */
            str_trim(line);
            if (strlen(line) == 0) continue;

            if (active_field == FIELD_PROJECTS) {
                append_text(resume->projects_text, MAX_TEXT, line);
            } else if (active_field == FIELD_SUMMARY) {
                append_text(resume->summary_text, MAX_TEXT, line);
            }
            /* else: stray text before any recognized field -> ignored */
        }
    }

    fclose(fp);

    if (resume->skill_count == 0) {
        fprintf(stderr, "[WARN] Resume '%s' has no SKILLS listed.\n", filepath);
    }
    return 0;
}

int load_resumes_from_dir(const char *dir_path, Resume *resumes, int max_resumes) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "[ERROR] Could not open resumes directory: %s\n", dir_path);
        return -1;
    }

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && count < max_resumes) {
        const char *name = entry->d_name;
        size_t nlen = strlen(name);
        /* only process *.txt files, skip . and .. and hidden files */
        if (nlen < 5) continue;
        if (name[0] == '.') continue;
        if (strcmp(name + nlen - 4, ".txt") != 0) continue;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, name);

        if (load_resume(full_path, &resumes[count]) == 0) {
            count++;
        }
    }
    closedir(dir);
    return count;
}
