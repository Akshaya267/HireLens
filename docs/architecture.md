# HireLens — Architecture

## 1. Design Goal

Keep **all resume-analysis intelligence inside a single C program** —
no NLP libraries, no external packages, no online AI/LLM APIs — while
still delivering a real, browsable web application for a live demo.

The simplest architecture that satisfies both constraints is a
**three-layer local pipeline**:

```
┌─────────────────────────────────────────────────────────────────┐
│                         Browser (UI Layer)                       │
│   frontend/index.html + css/style.css + js/app.js                │
│   Vanilla HTML/CSS/JS. No frameworks, no build step.              │
│   Dashboard · JD input · Resume upload · Analyze · Ranking ·      │
│   Candidate detail · Export                                       │
└───────────────────────────────┬───────────────────────────────────┘
                                 │ fetch() -> JSON over HTTP
┌───────────────────────────────▼───────────────────────────────────┐
│                    server.py  (Glue / Orchestration Layer)         │
│   Python standard library ONLY: http.server, subprocess, json.     │
│   Zero analysis logic lives here. Responsibilities:                │
│     - serve the static frontend                                    │
│     - persist JD text + resume text files to data/                 │
│     - invoke bin/hirelens_engine as a subprocess                   │
│     - return the JSON it produced back to the browser              │
└───────────────────────────────┬───────────────────────────────────┘
                                 │ subprocess.run([...])
┌───────────────────────────────▼───────────────────────────────────┐
│                 bin/hirelens_engine  (C Intelligence Engine)       │
│   Pure C, standard library only (stdio, string, ctype, dirent).    │
│   This is where 100% of the "intelligence" lives.                  │
└─────────────────────────────────────────────────────────────────┘
```

The C engine is also a **fully standalone CLI tool** — it can be run
directly with no web server at all:

```
./bin/hirelens_engine data/jd/job_description.txt data/resumes output
```

This matters for a COE assessment: a grader can compile and run the
engine in isolation, read its plain C source top to bottom, and verify
that no analysis decision is made anywhere except inside `src/*.c`.

## 2. C Engine Internal Pipeline

```
 JD file             Resume files (*.txt)
    │                      │
    ▼                      ▼
jd_parser.c          resume_parser.c
 (structured                (semi-structured
  KEY: value                 KEY: value +
  format)                    multi-line PROJECTS/
    │                        SUMMARY sections)
    │                      │
    └─────────┬────────────┘
              ▼
        text_utils.c            synonyms.c
   (lowercase, trim,      (ML<->Machine Learning,
    punctuation cleanup,   DSA<->Data Structures,
    CSV token splitting,   DBMS<->Database Mgmt Sys,
    case-insensitive       K8s<->Kubernetes, ~90
    substring search)      dictionary entries)
              │                  │
              └────────┬─────────┘
                        ▼
                   matcher.c
     (required/preferred skill matching with
      synonym awareness, education matching,
      experience gating, project-evidence
      scanning, keyword matching)
                        │
                        ▼
                   scorer.c
     (configurable weighted model combines
      the 5 sub-scores into overall_score)
                        │
                        ▼
                   ranker.c
        (qsort by overall_score desc,
         assigns 1..N rank)
                        │
                        ▼
                   explain.c
     (turns scores + matched/missing lists
      into strengths[], gaps[], status,
      and a recommendation sentence)
                        │
              ┌─────────┴─────────┐
              ▼                   ▼
       json_writer.c          report.c
   (ranking.json +        (report.txt, human
    candidate_N.json,      readable, exportable)
    hand-rolled, no
    external JSON lib)
```

## 3. Data Contracts

### Job Description file (`data/jd/job_description.txt`)

A structured `KEY: value` text format, parsed line-by-line by
`jd_parser.c`:

```
TITLE: Backend Software Engineer
REQUIRED_SKILLS: Java, Python, SQL, DSA, DBMS
PREFERRED_SKILLS: Docker, Kubernetes, AWS, ML
MIN_EXPERIENCE_YEARS: 2
EDUCATION: B.Tech Computer Science, BCA
KEYWORDS: scalable, microservices, agile, teamwork
```

`EDUCATION` is treated as a list of **accepted alternatives** (any one
qualifies), not a set of simultaneously-required degrees.

### Resume file (`data/resumes/*.txt`)

A semi-structured format with the same `KEY: value` convention, where
`PROJECTS` and `SUMMARY` may span multiple lines (accumulated until
the next recognized key or end of file):

```
NAME: Aisha Sharma
EMAIL: aisha.sharma@example.com
PHONE: 9876543210
EXPERIENCE_YEARS: 3
EDUCATION: B.Tech Computer Science, XYZ University, 2021
SKILLS: Java, Python, Machine Learning, SQL, Git
PROJECTS: Built a scalable microservices backend using Java and
  REST APIs. Developed an ML model for churn prediction in Python.
SUMMARY: Backend engineer with 3 years experience, strong in DSA
  and DBMS, agile team player.
```

Choosing a lightweight structured format (rather than attempting to
parse arbitrary PDF/Word resumes with zero libraries) keeps the
matching **reliable and explainable** — exactly what a rule-based
engine should optimize for — while `PROJECTS`/`SUMMARY` remain free
text that is tokenized and scanned for evidence.

### Output (`output/ranking.json`, `output/candidate_N.json`, `output/report.txt`)

Hand-written by `json_writer.c` / `report.c` — see `docs/scoring.md`
for the full field reference.

## 4. Why This Split Is "Simplest Reliable"

- **No web framework needed.** `http.server` from the Python standard
  library is sufficient for a local single-user demo; adding Flask/
  FastAPI would violate the "no external packages" constraint anyway.
- **No IPC complexity.** The C engine reads/writes plain files; the
  Python layer just shells out to it and reads the files back. No
  sockets, no shared memory, no custom protocol to debug.
- **Engine is independently testable.** `tests/run_tests.sh` invokes
  the C binary directly, never through the web layer, so engine
  correctness is verified in isolation from the UI.
- **Engine is independently demoable.** A grader with no browser can
  still run the CLI and read `output/report.txt`.
