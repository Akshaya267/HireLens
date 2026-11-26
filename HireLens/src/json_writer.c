/* ============================================================
 * json_writer.c
 * A minimal, hand-rolled JSON serializer (no external JSON
 * library) used to export engine results for the web dashboard
 * and for saved reports.
 * ============================================================ */
#include <string.h>
#include "json_writer.h"

void json_write_string(FILE *fp, const char *s) {
    fputc('"', fp);
    if (s) {
        for (const char *p = s; *p; p++) {
            unsigned char c = (unsigned char)*p;
            switch (c) {
                case '"':  fputs("\\\"", fp); break;
                case '\\': fputs("\\\\", fp); break;
                case '\n': fputs("\\n", fp); break;
                case '\r': fputs("\\r", fp); break;
                case '\t': fputs("\\t", fp); break;
                default:
                    if (c < 0x20) {
                        fprintf(fp, "\\u%04x", c);
                    } else {
                        fputc(c, fp);
                    }
            }
        }
    }
    fputc('"', fp);
}

static void write_matched_array(FILE *fp, const MatchedSkill *arr, int count, const char *pad) {
    fputs("[\n", fp);
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s  { \"skill\": ", pad);
        json_write_string(fp, arr[i].skill);
        fprintf(fp, ", \"via_synonym\": %s }%s\n", arr[i].via_synonym ? "true" : "false",
                (i < count - 1) ? "," : "");
    }
    fprintf(fp, "%s]", pad);
}

static void write_string_array(FILE *fp, char arr[][MAX_SKILL_LEN], int count, const char *pad) {
    fputs("[\n", fp);
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s  ", pad);
        json_write_string(fp, arr[i]);
        fprintf(fp, "%s\n", (i < count - 1) ? "," : "");
    }
    fprintf(fp, "%s]", pad);
}

static void write_note_array(FILE *fp, char arr[][MAX_NOTE_LEN], int count, const char *pad) {
    fputs("[\n", fp);
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s  ", pad);
        json_write_string(fp, arr[i]);
        fprintf(fp, "%s\n", (i < count - 1) ? "," : "");
    }
    fprintf(fp, "%s]", pad);
}

void json_write_candidate(FILE *fp, const CandidateResult *r, int indent) {
    char pad[16] = "";
    for (int i = 0; i < indent && i < 15; i++) pad[i] = ' ';
    pad[indent < 15 ? indent : 15] = '\0';

    fprintf(fp, "%s{\n", pad);
    fprintf(fp, "%s  \"rank\": %d,\n", pad, r->rank);
    fprintf(fp, "%s  \"name\": ", pad); json_write_string(fp, r->resume.name); fputs(",\n", fp);
    fprintf(fp, "%s  \"email\": ", pad); json_write_string(fp, r->resume.email); fputs(",\n", fp);
    fprintf(fp, "%s  \"phone\": ", pad); json_write_string(fp, r->resume.phone); fputs(",\n", fp);
    fprintf(fp, "%s  \"source_file\": ", pad); json_write_string(fp, r->resume.source_file); fputs(",\n", fp);
    fprintf(fp, "%s  \"experience_years\": %d,\n", pad, r->resume.experience_years);

    fprintf(fp, "%s  \"overall_score\": %.2f,\n", pad, r->overall_score);
    fprintf(fp, "%s  \"skill_score\": %.2f,\n", pad, r->skill_score);
    fprintf(fp, "%s  \"experience_score\": %.2f,\n", pad, r->experience_score);
    fprintf(fp, "%s  \"education_score\": %.2f,\n", pad, r->education_score);
    fprintf(fp, "%s  \"project_score\": %.2f,\n", pad, r->project_score);
    fprintf(fp, "%s  \"keyword_score\": %.2f,\n", pad, r->keyword_score);

    fprintf(fp, "%s  \"status\": ", pad); json_write_string(fp, r->status); fputs(",\n", fp);
    fprintf(fp, "%s  \"recommendation\": ", pad); json_write_string(fp, r->recommendation); fputs(",\n", fp);

    fprintf(fp, "%s  \"matched_required\": ", pad);
    write_matched_array(fp, r->matched_required, r->matched_required_count, pad);
    fputs(",\n", fp);

    fprintf(fp, "%s  \"matched_preferred\": ", pad);
    write_matched_array(fp, r->matched_preferred, r->matched_preferred_count, pad);
    fputs(",\n", fp);

    fprintf(fp, "%s  \"missing_required\": ", pad);
    write_string_array(fp, (char (*)[MAX_SKILL_LEN])r->missing_required, r->missing_required_count, pad);
    fputs(",\n", fp);

    fprintf(fp, "%s  \"missing_preferred\": ", pad);
    write_string_array(fp, (char (*)[MAX_SKILL_LEN])r->missing_preferred, r->missing_preferred_count, pad);
    fputs(",\n", fp);

    fprintf(fp, "%s  \"strengths\": ", pad);
    write_note_array(fp, (char (*)[MAX_NOTE_LEN])r->strengths, r->strengths_count, pad);
    fputs(",\n", fp);

    fprintf(fp, "%s  \"gaps\": ", pad);
    write_note_array(fp, (char (*)[MAX_NOTE_LEN])r->gaps, r->gaps_count, pad);
    fputs("\n", fp);

    fprintf(fp, "%s}", pad);
}

void json_write_ranking(FILE *fp, const JobDescription *jd, CandidateResult *results, int count) {
    fputs("{\n", fp);
    fputs("  \"job_description\": {\n", fp);
    fputs("    \"title\": ", fp); json_write_string(fp, jd->title); fputs(",\n", fp);
    fprintf(fp, "    \"required_skill_count\": %d,\n", jd->required_count);
    fprintf(fp, "    \"preferred_skill_count\": %d,\n", jd->preferred_count);
    fprintf(fp, "    \"min_experience_years\": %d\n", jd->min_experience_years);
    fputs("  },\n", fp);
    fprintf(fp, "  \"candidate_count\": %d,\n", count);
    fputs("  \"candidates\": [\n", fp);
    for (int i = 0; i < count; i++) {
        json_write_candidate(fp, &results[i], 4);
        fputs((i < count - 1) ? ",\n" : "\n", fp);
    }
    fputs("  ]\n", fp);
    fputs("}\n", fp);
}
