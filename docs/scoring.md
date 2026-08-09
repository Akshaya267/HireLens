# HireLens — Scoring Methodology

All weights below live in `include/config.h` as `#define` constants —
change them and rebuild (`make`) to re-tune the model without touching
any analysis logic.

## 1. Weighted Scoring Model

| Component        | Weight | Computed in    |
|-------------------|-------:|----------------|
| Skills            |    40% | `matcher.c` + `scorer.c` |
| Experience        |    20% | `matcher.c`    |
| Education         |    15% | `matcher.c`    |
| Projects          |    15% | `matcher.c`    |
| Keywords          |    10% | `matcher.c`    |

```
overall_score = skill_score      * 0.40
              + experience_score * 0.20
              + education_score  * 0.15
              + project_score    * 0.15
              + keyword_score    * 0.10
```

Every component is normalized to a **0–100** scale before weighting.

## 2. Skill Score (40%)

Required and preferred skills are scored separately, then combined:

```
required_pct = matched_required_count / jd.required_count * 100
preferred_pct = matched_preferred_count / jd.preferred_count * 100

skill_score = required_pct * 0.70 + preferred_pct * 0.30
```

A skill "matches" if its **canonicalized** form (after synonym
normalization — see below) is identical between the JD and the
resume's `SKILLS` field. This is a set-membership check, not a fuzzy
score, which keeps the result explainable: every match or miss can be
named exactly.

## 3. Experience Score (20%)

```
if candidate_years >= jd.min_experience_years: 100
else: (candidate_years / jd.min_experience_years) * 100
```

A candidate with more experience than required is not penalized, but
also doesn't get bonus points beyond 100 — this is a *gate*, not an
open-ended reward, matching how most JDs actually state requirements
("2+ years").

## 4. Education Score (15%)

The JD's `EDUCATION` list is treated as a set of **accepted
alternatives** (e.g. "B.Tech CS, BCA, B.E CS" means any one
qualifies). The candidate scores 100 if their listed education matches
(exactly, or via synonym normalization / substring match) **any one**
alternative, otherwise 0.

## 5. Project Score (15%)

Rewards *practical evidence* over a bare skill listing: the engine
scans the candidate's free-text `PROJECTS` field for mentions of each
required/preferred skill (canonical form or raw form, case
insensitive):

```
project_score = (skills_mentioned_in_projects / total_required_and_preferred) * 100
```

A candidate who lists "Machine Learning" as a skill but never mentions
it in any project gets full skill-score credit but a lower
project-score — a real signal a human reviewer would also apply.

## 6. Keyword Score (10%)

The JD's `KEYWORDS` list (typically soft skills / domain language —
"scalable", "agile", "teamwork") is searched for, case-insensitively,
across the candidate's combined `SUMMARY` + `PROJECTS` text:

```
keyword_score = (keywords_found / jd.keyword_count) * 100
```

## 7. Suitability Thresholds

| Overall Score | Status              |
|---------------:|---------------------|
| 85 – 100        | **Highly Suitable**  |
| 70 – 84.99      | **Suitable**         |
| 50 – 69.99      | **Partially Suitable** |
| 0 – 49.99       | **Not Suitable**     |

Thresholds are `#define`s in `include/config.h`
(`THRESHOLD_HIGHLY_SUITABLE`, etc.) — easy to retune per role level or
hiring bar.

## 8. Synonym Mapping (Rule-Based, Not ML)

`src/synonyms.c` contains a static lookup table (~90 entries) mapping
common abbreviations/variants to a single canonical form, e.g.:

| Variant(s)            | Canonical form                         |
|------------------------|-----------------------------------------|
| `ML`, `Machine Learning` | `machine learning`                   |
| `DSA`, `Data Structures` | `data structures and algorithms`     |
| `DBMS`, `Database`      | `database management system`         |
| `K8s`                   | `kubernetes`                          |
| `API`, `REST API`, `RESTful API` | `rest api`                    |
| `B.Tech`, `BTech`       | `bachelor of technology`              |

Every skill and education token, from both the JD and every resume, is
passed through `normalize_term()` before comparison. This is what lets
a JD asking for "DSA" correctly credit a resume that only lists "Data
Structures and Algorithms", or vice versa — **without any statistical
or machine-learning model**, purely a deterministic dictionary lookup.
The matching engine also records *whether* a given match only worked
because of this normalization (`via_synonym` flag), which the UI
surfaces as an explicit "matched via synonym" indicator for full
transparency.

## 9. Explainability

Nothing in the scoring pipeline is a black box:

- Every skill match/miss is listed by name (`matched_required`,
  `missing_required`, `matched_preferred`, `missing_preferred`).
- `strengths[]` and `gaps[]` are generated directly from the same
  numbers shown in the score breakdown — no separate hidden model.
- `recommendation` is a single templated sentence built from the
  candidate's own scores and counts, not free-generated text.

This is a deliberate design choice: **a rule-based system's main
advantage over an LLM/ML system is that every output is traceable back
to an exact rule**, and HireLens preserves that traceability all the
way to the UI.
