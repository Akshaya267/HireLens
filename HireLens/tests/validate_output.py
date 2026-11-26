#!/usr/bin/env python3
"""
Validates that a ranking.json file produced by the C engine has the
expected structure. This is a test-harness assertion helper only --
it performs no resume analysis itself.
"""
import json
import sys

REQUIRED_TOP_KEYS = {"job_description", "candidate_count", "candidates"}
REQUIRED_CANDIDATE_KEYS = {
    "rank", "name", "email", "phone", "source_file", "experience_years",
    "overall_score", "skill_score", "experience_score", "education_score",
    "project_score", "keyword_score", "status", "recommendation",
    "matched_required", "matched_preferred", "missing_required",
    "missing_preferred", "strengths", "gaps",
}
VALID_STATUSES = {"Highly Suitable", "Suitable", "Partially Suitable", "Not Suitable"}


def fail(msg):
    print(f"VALIDATION FAILED: {msg}")
    sys.exit(1)


def main():
    if len(sys.argv) != 2:
        fail("usage: validate_output.py <ranking.json>")

    with open(sys.argv[1], "r", encoding="utf-8") as f:
        data = json.load(f)

    missing = REQUIRED_TOP_KEYS - data.keys()
    if missing:
        fail(f"missing top-level keys: {missing}")

    if data["candidate_count"] != len(data["candidates"]):
        fail("candidate_count does not match length of candidates array")

    prev_score = None
    prev_rank = 0
    for c in data["candidates"]:
        missing_keys = REQUIRED_CANDIDATE_KEYS - c.keys()
        if missing_keys:
            fail(f"candidate '{c.get('name','?')}' missing keys: {missing_keys}")

        if not (0.0 <= c["overall_score"] <= 100.0):
            fail(f"overall_score out of range for {c['name']}: {c['overall_score']}")

        for key in ("skill_score", "experience_score", "education_score",
                    "project_score", "keyword_score"):
            if not (0.0 <= c[key] <= 100.0):
                fail(f"{key} out of range for {c['name']}: {c[key]}")

        if c["status"] not in VALID_STATUSES:
            fail(f"invalid status for {c['name']}: {c['status']}")

        if c["rank"] != prev_rank + 1:
            fail(f"rank sequence broken: expected {prev_rank+1}, got {c['rank']}")
        prev_rank = c["rank"]

        if prev_score is not None and c["overall_score"] > prev_score + 1e-9:
            fail("candidates are not sorted in descending overall_score order")
        prev_score = c["overall_score"]

    print(f"  validated {len(data['candidates'])} candidate(s) OK")


if __name__ == "__main__":
    main()
