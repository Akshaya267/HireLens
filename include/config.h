/* ============================================================
 * HireLens - Configurable Scoring Model
 * Change these constants to re-tune the weighted scoring model
 * without touching any analysis logic.
 * ============================================================ */
#ifndef HIRELENS_CONFIG_H
#define HIRELENS_CONFIG_H

/* Top level weighted-scoring model. Must sum to 1.0 */
#define WEIGHT_SKILLS       0.40
#define WEIGHT_EXPERIENCE   0.20
#define WEIGHT_EDUCATION    0.15
#define WEIGHT_PROJECTS     0.15
#define WEIGHT_KEYWORDS     0.10

/* Within the Skills component: required vs preferred split. Must sum to 1.0 */
#define REQUIRED_SKILL_WEIGHT   0.70
#define PREFERRED_SKILL_WEIGHT  0.30

/* Suitability thresholds (applied to overall_score, 0-100) */
#define THRESHOLD_HIGHLY_SUITABLE   85.0
#define THRESHOLD_SUITABLE          70.0
#define THRESHOLD_PARTIALLY_SUITABLE 50.0

#endif /* HIRELENS_CONFIG_H */
