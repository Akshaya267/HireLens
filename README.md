# HireLens — Intelligent Resume Analyzer

> A **rule-based**, explainable resume matching & ranking system with
> its core intelligence implemented entirely in **C**. Built for a
> college COE assessment under a strict constraint: **no external
> packages, no third-party APIs, no online AI/LLM services** — every
> matching, scoring and ranking decision is made by hand-written C
> logic using only the standard library.

---

## 1. Problem

Reviewing a stack of resumes against a job description by hand is
slow, inconsistent, and hard to justify to candidates or stakeholders
("why did we reject this person?"). Recruiters need a way to:

- Compare **many** resumes to **one** job description quickly
- Get a **defensible, explainable** score, not a gut feeling
- See exactly **which** requirements were met and which weren't
- Handle the reality that people describe the same skill differently
  ("ML" vs "Machine Learning", "DSA" vs "Data Structures")

## 2. Solution

HireLens parses a structured Job Description and a batch of structured
resumes, then runs them through a **weighted, rule-based matching
engine written in C**:

- Tokenizes and normalizes text (lowercasing, punctuation cleanup)
- Maps ~90 common skill/degree abbreviations to canonical forms
  (synonym mapping)
- Matches required vs. preferred skills, education, experience and
  project evidence separately
- Combines them into one configurable weighted score
- Ranks all candidates and generates a plain-English explanation —
  strengths, gaps and a recommendation — for each one

It is **not** an ML or LLM system. It does not "understand" resumes
semantically. It applies deterministic, inspectable rules — which is
exactly what makes every score traceable and defensible.

## 3. Features

- ✅ Multi-resume batch analysis against one JD
- ✅ Required vs. preferred skill handling with independent weighting
- ✅ Synonym/abbreviation mapping (ML, DSA, DBMS, OOP, K8s, AWS, …)
- ✅ Configurable weighted scoring model (skills/experience/education/
  projects/keywords)
- ✅ Experience gating, education-alternatives matching, project
  evidence scanning, keyword matching
- ✅ Candidate ranking (stable sort, tie-break by skill then experience)
- ✅ Full explainability: matched/missing skills, strengths, gaps,
  synonym-match indicators, and a generated recommendation sentence
- ✅ Suitability status bands (Highly Suitable / Suitable / Partially
  Suitable / Not Suitable)
- ✅ JSON + plain-text report export
- ✅ A full 7-screen web dashboard (Home, JD input, Resume upload,
  Analyze, Ranking, Candidate detail, Export)
- ✅ Automated test suite with structural validation, known-value
  regression checks, and edge-case fixtures

## 4. Architecture

```
Browser (HTML/CSS/JS)  →  server.py (Python stdlib glue)  →  bin/hirelens_engine (C)
```

The C binary is also a standalone CLI tool — no web server required:

```bash
./bin/hirelens_engine data/jd/job_description.txt data/resumes output
```

See **[docs/architecture.md](docs/architecture.md)** for the full
pipeline diagram and data contracts, and
**[docs/scoring.md](docs/scoring.md)** for the complete scoring
methodology and synonym dictionary.

### Why C is the real engine, not just a script

Every one of these is implemented as C code operating on `struct`s,
`char*` buffers and arrays — no shortcuts:

| Requirement                  | Implementation |
|-------------------------------|----------------|
| Text preprocessing/normalization | `src/text_utils.c` |
| Tokenization / CSV splitting  | `src/text_utils.c` (`split_and_trim`) |
| Synonym mapping               | `src/synonyms.c` (static dictionary + lookup) |
| JD & resume parsing            | `src/jd_parser.c`, `src/resume_parser.c` |
| Keyword/phrase matching        | `src/matcher.c` |
| Required vs. preferred handling | `src/matcher.c`, `src/scorer.c` |
| Weighted scoring                | `src/scorer.c`, `include/config.h` |
| Candidate ranking (`qsort`)     | `src/ranker.c` |
| Explainability / recommendation | `src/explain.c` |
| JSON export (hand-rolled)       | `src/json_writer.c` |
| Report export                   | `src/report.c` |
| File I/O, directory scanning    | `src/resume_parser.c` (`dirent.h`) |

Python (`server.py`) is used **only** as a thin, standard-library-only
HTTP glue layer that shells out to the compiled binary — it contains
zero resume-analysis logic. The frontend is plain HTML/CSS/JS with no
frameworks or build tooling.

## 5. Algorithm Summary

1. **Parse** JD (`TITLE`, `REQUIRED_SKILLS`, `PREFERRED_SKILLS`,
   `MIN_EXPERIENCE_YEARS`, `EDUCATION`, `KEYWORDS`) and every resume in
   `data/resumes/*.txt`.
2. **Normalize** every skill/degree token through the synonym
   dictionary into a canonical lowercase form.
3. **Match**: required skills, preferred skills, education
   (any-of-alternatives), experience (gated), project text (evidence
   scan), keyword text (soft-skill scan).
4. **Score** each component 0–100, then combine with configured
   weights into `overall_score`.
5. **Rank** all candidates descending by `overall_score` (`qsort`,
   tie-broken by skill score then experience score).
6. **Explain**: generate `strengths[]`, `gaps[]`, a `status` band, and
   a human-readable `recommendation` sentence directly from the scores
   and matched/missing lists — no separate hidden logic.
7. **Export**: `output/ranking.json`, one `output/candidate_N.json`
   per candidate, and a formatted `output/report.txt`.

Full methodology and worked examples: **[docs/scoring.md](docs/scoring.md)**.

## 6. Project Structure

```
HireLens/
├── src/                 # C engine implementation (.c)
├── include/              # C engine headers (.h) — structs, config, prototypes
├── data/
│   ├── jd/                job_description.txt (sample JD)
│   └── resumes/            5 sample fictional resumes (.txt)
├── output/                generated ranking.json / candidate_N.json / report.txt
├── tests/                 automated test suite + edge-case fixtures
├── docs/                  architecture.md, scoring.md
├── screenshots/           UI screenshots (add after running locally)
├── frontend/               HTML/CSS/JS web dashboard (served by server.py)
├── server.py               Python stdlib-only glue server
├── Makefile                 builds the C engine
├── README.md
└── .gitignore
```

## 7. Setup & Run Instructions

### Prerequisites

- `gcc` (or any C11-compliant compiler) and `make`
- `python3` (standard library only — no `pip install` needed, ever)
- A POSIX-like environment (Linux/macOS/WSL) — the engine uses
  `dirent.h` for directory scanning

### Build the C engine

```bash
cd HireLens
make
```

This produces `bin/hirelens_engine`.

### Option A — Run as a CLI tool (no web server)

```bash
./bin/hirelens_engine data/jd/job_description.txt data/resumes output
cat output/report.txt
```

### Option B — Run the full web dashboard

```bash
python3 server.py
# then open http://localhost:8000 in a browser
```

In the UI:
1. **Dashboard** — overview and quick-start
2. **Job Description** — load the sample JD or paste/upload your own
3. **Resumes** — load the 5 sample resumes or upload your own `.txt` files
4. **Analyze** — runs the C engine as a subprocess
5. **Ranking** — sortable table of all candidates with sub-scores
6. **Candidate Detail** — full explainable breakdown per candidate
7. **Export** — download `ranking.json` or `report.txt`

The web layer never re-implements analysis logic; it always calls
`bin/hirelens_engine` fresh on every "Analyze" click.

## 8. Testing

```bash
bash tests/run_tests.sh
```

The suite:
1. Builds the engine from a clean state
2. Runs it against the bundled sample dataset and validates the JSON
   structure (`tests/validate_output.py`)
3. Runs it against edge-case fixtures in `tests/fixtures/`
   (empty skills, zero experience, blank optional fields, mixed-case
   synonym-heavy input) to confirm the engine degrades gracefully
   instead of crashing
4. Runs known-value regression assertions
   (`tests/assert_known_values.py`) against the sample dataset — e.g.
   asserting the strongest sample candidate ranks #1 and is "Highly
   Suitable", and that synonym-based matches are actually detected

## 9. Sample Data

`data/jd/job_description.txt` defines a Backend Software Engineer role.
`data/resumes/` contains 5 fictional candidates spanning the full
suitability spectrum:

| Candidate      | Profile                                      | Typical Result |
|-----------------|-----------------------------------------------|----------------|
| Karthik Iyer     | Senior, uses abbreviations (K8s, API, ML, DSA) | Highly Suitable |
| Aisha Sharma     | Mid-level, direct skill matches, strong projects | Highly Suitable |
| Sneha Reddy      | Junior-mid, missing a couple of required skills | Partially Suitable |
| Rohan Verma      | Fresh graduate, limited backend exposure       | Not Suitable |
| Priya Nair       | Unrelated background (Physics), no matching skills | Not Suitable |

Run the engine or the web UI to see the exact scores — this table is
illustrative, not hard-coded.

## 10. Limitations

- **Not NLP/ML.** Matching is exact/substring-based after synonym
  normalization — it does not understand semantics, context, or
  sentence structure. A resume that says "no experience with SQL"
  would still register "SQL" as a keyword hit; there is no negation
  handling.
- **Structured input required.** Resumes and JDs must follow the
  `KEY: value` format. HireLens deliberately does not attempt to parse
  arbitrary PDF/Word resumes with zero external libraries, since that
  would make matching unreliable — a directly stated design trade-off,
  not an oversight.
- **Synonym dictionary is finite.** ~90 hard-coded entries cover
  common CS/software terms; an unlisted abbreviation falls back to
  literal string matching.
- **Single-process, single-user.** `server.py` is a demo-grade local
  server, not a production deployment (no auth, no concurrency
  hardening beyond basic threading).
- **English only.** No multilingual support.

## 11. Future Enhancements

- Configurable synonym dictionary loaded from a data file instead of
  compiled-in constants, so it can be extended without rebuilding
- Fuzzy/edit-distance matching for minor spelling variations
- PDF/DOCX ingestion (would require adding a parsing library, which
  the current assessment constraints intentionally exclude)
- Per-JD custom scoring weights via the UI (currently compile-time)
- Bulk export (all candidates) to a single combined PDF report
- Candidate comparison view (side-by-side breakdown of 2+ candidates)

## 12. Academic Integrity Note

This project is explicitly a **rule-based, deterministic C program**.
It does not call OpenAI, Gemini, HuggingFace, or any other AI/LLM
service, and does not use spaCy, NLTK, or any NLP library. All
"intelligence" — parsing, synonym normalization, matching, scoring,
ranking and explanation generation — is hand-implemented in standard
C, as required by the assessment brief.
