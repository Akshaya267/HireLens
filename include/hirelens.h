/* ============================================================
 * HireLens - Intelligent Resume Analyzer
 * Core data structures shared across the C analysis engine.
 * ============================================================ */
#ifndef HIRELENS_H
#define HIRELENS_H

#define MAX_LINE            2048
#define MAX_SKILLS          100
#define MAX_SKILL_LEN       128
#define MAX_NAME            128
#define MAX_TEXT            8192
#define MAX_CANDIDATES      50
#define MAX_KEYWORDS        50
#define MAX_EDU             10
#define MAX_NOTES           12
#define MAX_NOTE_LEN        256

/* A single skill token, in both its original (raw) and
 * canonicalized (synonym-normalized) form. */
typedef struct {
    char raw[MAX_SKILL_LEN];
    char canonical[MAX_SKILL_LEN];
} Skill;

typedef struct {
    char title[MAX_NAME];

    Skill required_skills[MAX_SKILLS];
    int   required_count;

    Skill preferred_skills[MAX_SKILLS];
    int   preferred_count;

    int   min_experience_years;

    char  education[MAX_EDU][MAX_SKILL_LEN];
    int   education_count;

    char  keywords[MAX_KEYWORDS][MAX_SKILL_LEN];
    int   keyword_count;
} JobDescription;

typedef struct {
    char name[MAX_NAME];
    char email[MAX_NAME];
    char phone[64];

    int  experience_years;

    char education[MAX_EDU][MAX_SKILL_LEN];
    int  education_count;

    Skill skills[MAX_SKILLS];
    int   skill_count;

    char projects_text[MAX_TEXT];
    char summary_text[MAX_TEXT];

    char source_file[256];
} Resume;

typedef struct {
    char skill[MAX_SKILL_LEN];
    int  via_synonym;   /* 1 if matched through synonym mapping, not literal text */
} MatchedSkill;

typedef struct {
    Resume resume;

    /* Component scores (0-100) */
    double skill_score;
    double experience_score;
    double education_score;
    double project_score;
    double keyword_score;
    double overall_score;

    /* Explainability data */
    MatchedSkill matched_required[MAX_SKILLS];
    int matched_required_count;

    MatchedSkill matched_preferred[MAX_SKILLS];
    int matched_preferred_count;

    char missing_required[MAX_SKILLS][MAX_SKILL_LEN];
    int  missing_required_count;

    char missing_preferred[MAX_SKILLS][MAX_SKILL_LEN];
    int  missing_preferred_count;

    char strengths[MAX_NOTES][MAX_NOTE_LEN];
    int  strengths_count;

    char gaps[MAX_NOTES][MAX_NOTE_LEN];
    int  gaps_count;

    char status[32];            /* Highly Suitable / Suitable / ... */
    char recommendation[768];   /* human readable explanation        */

    int  rank;
} CandidateResult;

#endif /* HIRELENS_H */
