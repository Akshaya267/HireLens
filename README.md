# HireLens — Intelligent Resume Analyzer

> A **rule-based**, explainable resume matching & ranking system with its core intelligence implemented entirely in **C**. No external packages, third-party APIs, or online AI/LLM services are used — every matching, scoring, and ranking decision is made through hand-written C logic using only the standard library.

---
## 1. Problem
Reviewing a stack of resumes against a job description by hand is slow, inconsistent, and difficult to justify ("why did we reject this candidate?"). Recruiters need a way to:
* Compare **many resumes against one job description**
* Get a **defensible, explainable score**
* See exactly **which requirements were met and which were missing**
* Handle different ways of describing the same skill, such as `"ML"` vs `"Machine Learning"` or `"DSA"` vs `"Data Structures"`

## 2. Solution
HireLens parses a structured Job Description and a batch of structured resumes, then processes them through a **weighted, rule-based matching engine written entirely in C**.
The system:
* Tokenizes and normalizes text
* Cleans punctuation and converts text to lowercase
* Maps ~90 common skill and degree abbreviations to canonical forms
* Matches required and preferred skills separately
* Evaluates education and experience
* Scans project descriptions for relevant evidence
* Combines multiple components into a configurable weighted score
* Ranks candidates
* Generates plain-English explanations containing strengths, gaps, and recommendations
HireLens is **not an ML or LLM system**. It does not perform semantic understanding of resumes. Instead, it applies deterministic and inspectable rules, making every score and ranking decision traceable.

## 3. Features
* ✅ Multi-resume batch analysis against one JD
* ✅ Required vs. preferred skill handling
* ✅ Synonym and abbreviation mapping
* ✅ Configurable weighted scoring model
* ✅ Experience gating
* ✅ Education-alternative matching
* ✅ Project evidence scanning
* ✅ Keyword matching
* ✅ Candidate ranking using stable sorting
* ✅ Tie-breaking by skill score and experience score
* ✅ Explainable matched and missing skills
* ✅ Synonym-match indicators
* ✅ Strength and gap generation
* ✅ Human-readable recommendation
* ✅ Suitability status bands:
  * Highly Suitable
  * Suitable
  * Partially Suitable
  * Not Suitable
* ✅ JSON and plain-text report export
* ✅ Full 7-screen web dashboard
* ✅ Automated test suite
* ✅ Structural validation
* ✅ Known-value regression testing
* ✅ Edge-case testing

## 4. Architecture
```text
Browser (HTML/CSS/JS)
        ↓
server.py
(Python standard-library HTTP glue)
        ↓
bin/hirelens_engine
(C rule-based analysis engine)
        ↓
JSON / Text Reports
```
The C binary is also available as a standalone CLI tool:

```bash
./bin/hirelens_engine data/jd/job_description.txt data/resumes output
```
See `docs/architecture.md` for the complete architecture and data contracts.

See `docs/scoring.md` for the scoring methodology and synonym dictionary.

### Why C is the Real Engine
The analysis is implemented directly in C using structures, character buffers, arrays, and standard-library functionality.
| Requirement                        | Implementation                     |
| ---------------------------------- | ---------------------------------- |
| Text preprocessing / normalization | `src/text_utils.c`                 |
| Tokenization / CSV splitting       | `src/text_utils.c`                 |
| Synonym mapping                    | `src/synonyms.c`                   |
| JD parsing                         | `src/jd_parser.c`                  |
| Resume parsing                     | `src/resume_parser.c`              |
| Keyword / phrase matching          | `src/matcher.c`                    |
| Required / preferred handling      | `src/matcher.c`, `src/scorer.c`    |
| Weighted scoring                   | `src/scorer.c`, `include/config.h` |
| Candidate ranking                  | `src/ranker.c`                     |
| Explainability                     | `src/explain.c`                    |
| JSON export                        | `src/json_writer.c`                |
| Text report generation             | `src/report.c`                     |
| File and directory handling        | `src/resume_parser.c`              |

### Web Layer
`server.py` is only a thin Python standard-library HTTP layer.
It:
* Serves the frontend
* Receives user input
* Manages temporary input files
* Executes the C engine
* Reads generated output
* Returns results to the browser
It contains **zero resume-analysis or scoring logic**.
The frontend is implemented using plain:
* HTML
* CSS
* JavaScript
No frontend framework or build tooling is required.

## 5. Algorithm Summary
### Step 1 — Parse
The system reads the structured JD:

```text
TITLE
REQUIRED_SKILLS
PREFERRED_SKILLS
MIN_EXPERIENCE_YEARS
EDUCATION
KEYWORDS
```
It then loads every resume from:
```text
data/resumes/*.txt
```
### Step 2 — Normalize
Skills, degrees, and relevant text are normalized through the built-in synonym dictionary.
Examples:
```text
ML      → machine learning
DSA     → data structures and algorithms
DBMS    → database management systems
OOP     → object oriented programming
K8s     → kubernetes
```
### Step 3 — Match
Each candidate is evaluated across multiple dimensions:
* Required skills
* Preferred skills
* Education
* Experience
* Project evidence
* Keywords
### Step 4 — Score
Each component receives a score from `0–100`.
The configured weights are then used to calculate the candidate's overall score.
### Step 5 — Rank
Candidates are sorted in descending order of overall score.
Tie-breaking uses:
1. Skill score
2. Experience score
### Step 6 — Explain
The engine generates:
```text
strengths[]
gaps[]
status
recommendation
```
directly from the calculated scores and matched/missing lists.
There is no separate hidden ranking or explanation mechanism.
### Step 7 — Export
The engine generates:
```text
output/
├── ranking.json
├── candidate_1.json
├── candidate_2.json
├── ...
└── report.txt
```
## 6. Project Structure
```text
HireLens/
├── src/                  # C engine implementation
├── include/              # C headers, structures and configuration
├── data/
│   ├── jd/
│   │   └── job_description.txt
│   └── resumes/
│       └── *.txt
├── output/               # Generated analysis reports
├── tests/                # Automated tests and fixtures
├── docs/
│   ├── architecture.md
│   └── scoring.md
├── screenshots/          # UI screenshots
├── frontend/             # HTML/CSS/JS dashboard
├── server.py             # Python standard-library web server
├── Makefile              # C build configuration
├── README.md
└── .gitignore
```
## 7. Setup & Run
### Prerequisites
* GCC or another C11-compatible compiler
* Make
* Python 3
* A POSIX-compatible environment such as Linux, macOS, or WSL
The Python web layer uses only the standard library. No `pip install` is required.
### Build the C Engine
```bash
cd HireLens
make
```
This produces:
```text
bin/hirelens_engine
```
### Option A — CLI Mode
Run the engine directly:
```bash
./bin/hirelens_engine data/jd/job_description.txt data/resumes output
```
View the generated report:

```bash
cat output/report.txt
```
### Option B — Web Dashboard

Start the server:

```bash
python3 server.py
```

Then open:

```text
http://localhost:8000
```

### Dashboard Flow

1. **Dashboard** — system overview and quick start
2. **Job Description** — load or enter a JD
3. **Resumes** — load or upload resumes
4. **Analyze** — execute the C matching engine
5. **Ranking** — view ranked candidates and sub-scores
6. **Candidate Detail** — inspect the complete explanation
7. **Export** — download generated reports
The web interface always executes the C engine for analysis instead of duplicating the scoring logic in JavaScript or Python.

## 8. Testing

Run the complete test suite:

```bash
bash tests/run_tests.sh
```

The test suite:

1. Builds the engine from a clean state
2. Runs the bundled sample dataset
3. Validates the generated JSON structure
4. Tests edge-case fixtures
5. Tests empty skills and optional fields
6. Tests zero-experience candidates
7. Tests mixed-case and synonym-heavy input
8. Performs known-value regression checks
9. Verifies candidate ranking
10. Verifies synonym-based matching
The tests are designed to ensure that the engine produces deterministic and stable results without crashing on unusual inputs.

## 9. Sample Data

The bundled dataset contains five fictional candidates for a Backend Software Engineer role.

| Candidate    | Profile                                                           | Typical Result     |
| ------------ | ----------------------------------------------------------------- | ------------------ |
| Karthik Iyer | Senior profile with strong backend skills and abbreviations       | Highly Suitable    |
| Aisha Sharma | Mid-level candidate with direct skill matches and strong projects | Highly Suitable    |
| Sneha Reddy  | Junior-mid profile with some missing requirements                 | Partially Suitable |
| Rohan Verma  | Fresh graduate with limited backend exposure                      | Not Suitable       |
| Priya Nair   | Unrelated background with minimal matching skills                 | Not Suitable       |

The results shown above are illustrative. The actual scores and ranking are calculated dynamically by the C engine.
## 10. Limitations

### No Semantic Understanding
HireLens is rule-based rather than ML/NLP-based.
Matching is primarily exact or substring-based after synonym normalization.

For example, a resume containing:

```text
No experience with SQL
```
may still register `SQL` as a match because the current engine does not implement contextual negation detection.

### Structured Input Required
Resumes and Job Descriptions must follow the expected `KEY: value` format.
The current system does not attempt to extract information from arbitrary PDF or Word documents.
This keeps the core system dependency-free and deterministic.

### Finite Synonym Dictionary
The built-in dictionary contains approximately 90 common technical terms and abbreviations.
Unknown abbreviations fall back to literal matching.

### Local Single-Process Server
`server.py` is designed as a lightweight local application server rather than a production deployment.
It does not provide:
* Authentication
* Production-grade security
* Distributed processing
* Persistent user management

### English Only
The current matching and synonym system is designed for English-language resumes and job descriptions.

## 11. Future Enhancements
Potential future improvements include:
* Configurable synonym dictionary loaded from an external data file
* Fuzzy and edit-distance matching
* PDF and DOCX resume ingestion
* Per-JD scoring configuration through the UI
* Combined candidate report export
* Side-by-side candidate comparison
* Additional configurable matching rules
* Expanded synonym and skill dictionaries
* Multilingual resume support

## 12. Design Philosophy
HireLens focuses on three principles:

### Deterministic
The same input always produces the same result.

### Explainable
Every score can be traced back to explicit matching rules.

### Inspectable
The complete matching, scoring, ranking, and explanation logic exists in the source code.
There is no hidden model or external service making decisions.
---
## License
This project is available for educational and demonstration purposes.
