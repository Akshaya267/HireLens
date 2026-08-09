#!/usr/bin/env python3
"""
Known-value regression test: asserts specific expected outcomes for
the bundled sample JD + 5 sample resumes in data/. If someone edits
the sample data OR breaks the scoring logic, this test will catch it.
Test-harness only -- performs no resume analysis itself.
"""
import json
import sys


def fail(msg):
    print(f"ASSERTION FAILED: {msg}")
    sys.exit(1)


def main():
    with open(sys.argv[1], "r", encoding="utf-8") as f:
        data = json.load(f)

    by_name = {c["name"]: c for c in data["candidates"]}

    expected_names = {"Aisha Sharma", "Rohan Verma", "Priya Nair", "Karthik Iyer", "Sneha Reddy"}
    if set(by_name.keys()) != expected_names:
        fail(f"unexpected candidate set: {set(by_name.keys())}")

    # Karthik (5 yrs, matches all required + most preferred incl. via
    # synonyms K8s/API/ML/DSA/DBMS) should rank #1 and be Highly Suitable.
    karthik = by_name["Karthik Iyer"]
    if karthik["rank"] != 1:
        fail(f"expected Karthik Iyer to rank #1, got #{karthik['rank']}")
    if karthik["status"] != "Highly Suitable":
        fail(f"expected Karthik Iyer to be Highly Suitable, got {karthik['status']}")
    if karthik["overall_score"] < 85:
        fail(f"expected Karthik Iyer overall_score >= 85, got {karthik['overall_score']}")

    # Aisha (3 yrs, strong direct skill matches) should also be Highly Suitable.
    aisha = by_name["Aisha Sharma"]
    if aisha["status"] != "Highly Suitable":
        fail(f"expected Aisha Sharma to be Highly Suitable, got {aisha['status']}")

    # Priya (Physics background, no relevant skills) should score the lowest
    # and be Not Suitable.
    priya = by_name["Priya Nair"]
    if priya["rank"] != 5:
        fail(f"expected Priya Nair to rank last (#5), got #{priya['rank']}")
    if priya["status"] != "Not Suitable":
        fail(f"expected Priya Nair to be Not Suitable, got {priya['status']}")

    # Synonym mapping check: Karthik lists "K8s", "API", "ML", "DSA", "DBMS"
    # as raw skills; these must match the JD's fully-spelled-out requirements
    # via synonym normalization.
    synonym_matches = [
        m for group in (karthik["matched_required"], karthik["matched_preferred"])
        for m in group if m["via_synonym"]
    ]
    if len(synonym_matches) < 2:
        fail(f"expected several synonym-based matches for Karthik Iyer, got {len(synonym_matches)}")

    # Experience gating check: Priya has 0 years experience against a JD
    # requiring 2 -> experience_score must be 0.
    if priya["experience_score"] != 0.0:
        fail(f"expected Priya Nair experience_score == 0, got {priya['experience_score']}")

    print("  all known-value assertions passed for sample dataset")
    print(f"  ranking: {[ (c['rank'], c['name'], c['overall_score']) for c in data['candidates'] ]}")


if __name__ == "__main__":
    main()
