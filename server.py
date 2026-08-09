#!/usr/bin/env python3
"""
==================================================================
 HireLens - Web Application Server
==================================================================
IMPORTANT: This file contains ZERO resume-analysis intelligence.
It is a thin orchestration/glue layer that:
  1. Serves the static HTML/CSS/JS dashboard
  2. Accepts the JD text and resume text files from the browser
  3. Invokes the compiled C engine (bin/hirelens_engine) as a
     subprocess to do ALL actual analysis
  4. Returns the JSON/report files the C engine produced

It uses ONLY the Python standard library (http.server, json,
os, subprocess, urllib) -- no Flask, Django, FastAPI, or any
third-party package, satisfying the "no external
packages/libraries" constraint end-to-end.
==================================================================
"""

import http.server
import socketserver
import json
import os
import re
import subprocess
import sys
import urllib.parse

# ---------------------------------------------------------------
# Paths / configuration
# ---------------------------------------------------------------
ROOT_DIR = os.path.dirname(os.path.abspath(__file__))
FRONTEND_DIR = os.path.join(ROOT_DIR, "frontend")
DATA_DIR = os.path.join(ROOT_DIR, "data")
JD_DIR = os.path.join(DATA_DIR, "jd")
RESUME_DIR = os.path.join(DATA_DIR, "resumes")
OUTPUT_DIR = os.path.join(ROOT_DIR, "output")
ENGINE_BIN = os.path.join(
    ROOT_DIR,
    "bin",
    "hirelens_engine.exe" if os.name == "nt" else "hirelens_engine"
)
JD_FILE = os.path.join(ROOT_DIR, "data", "jd", "job_description.txt")
PORT = int(os.environ.get("HIRELENS_PORT", "8000"))

for d in (JD_DIR, RESUME_DIR, OUTPUT_DIR):
    os.makedirs(d, exist_ok=True)


def safe_filename(name: str) -> str:
    """Strip path separators / unsafe chars from a client-supplied filename."""
    name = os.path.basename(name or "resume")
    name = re.sub(r"[^A-Za-z0-9._-]", "_", name)
    if not name.lower().endswith(".txt"):
        name += ".txt"
    return name


class HireLensHandler(http.server.BaseHTTPRequestHandler):
    server_version = "HireLens/1.0"

    # ---------- helpers ----------
    def _send_json(self, obj, status=200):
        body = json.dumps(obj, indent=2).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(body)

    def _send_file(self, path, content_type):
        if not os.path.isfile(path):
            self._send_json({"error": f"File not found: {os.path.basename(path)}"}, 404)
            return
        with open(path, "rb") as f:
            body = f.read()
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _read_json_body(self):
        length = int(self.headers.get("Content-Length", 0))
        if length == 0:
            return {}
        raw = self.rfile.read(length)
        try:
            return json.loads(raw.decode("utf-8"))
        except Exception:
            return {}

    def _static_content_type(self, path):
        if path.endswith(".html"):
            return "text/html; charset=utf-8"
        if path.endswith(".css"):
            return "text/css; charset=utf-8"
        if path.endswith(".js"):
            return "application/javascript; charset=utf-8"
        if path.endswith(".json"):
            return "application/json"
        if path.endswith(".svg"):
            return "image/svg+xml"
        return "application/octet-stream"

    def log_message(self, fmt, *args):
        sys.stderr.write("[server] " + (fmt % args) + "\n")

    # ---------- routing ----------
    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

    def do_GET(self):
        parsed = urllib.parse.urlparse(self.path)
        path = parsed.path

        if path == "/" or path == "":
            self._send_file(os.path.join(FRONTEND_DIR, "index.html"), "text/html; charset=utf-8")
            return

        if path.startswith("/api/"):
            self._handle_api_get(path, urllib.parse.parse_qs(parsed.query))
            return

        # static assets (css/js) served from /frontend
        static_path = os.path.normpath(os.path.join(FRONTEND_DIR, path.lstrip("/")))
        if static_path.startswith(FRONTEND_DIR) and os.path.isfile(static_path):
            self._send_file(static_path, self._static_content_type(static_path))
            return

        # bundled sample data (JD + sample resumes), served read-only so the
        # UI's "load sample" buttons can fetch them directly
        if path.startswith("/data/"):
            data_path = os.path.normpath(os.path.join(ROOT_DIR, path.lstrip("/")))
            if data_path.startswith(DATA_DIR) and os.path.isfile(data_path):
                self._send_file(data_path, "text/plain; charset=utf-8")
                return

        self._send_json({"error": "Not found"}, 404)

    def do_POST(self):
        parsed = urllib.parse.urlparse(self.path)
        path = parsed.path
        body = self._read_json_body()

        if path == "/api/jd":
            content = body.get("content", "")
            with open(JD_FILE, "w", encoding="utf-8") as f:
                f.write(content)
            self._send_json({"status": "ok", "message": "Job description saved."})
            return

        if path == "/api/resume":
            filename = safe_filename(body.get("filename", "resume.txt"))
            content = body.get("content", "")
            with open(os.path.join(RESUME_DIR, filename), "w", encoding="utf-8") as f:
                f.write(content)
            self._send_json({"status": "ok", "filename": filename})
            return

        if path == "/api/analyze":
            self._run_analysis()
            return

        if path == "/api/reset":
            self._reset_workspace()
            return

        self._send_json({"error": "Unknown endpoint"}, 404)

    def do_DELETE(self):
        parsed = urllib.parse.urlparse(self.path)
        path = parsed.path
        qs = urllib.parse.parse_qs(parsed.query)

        if path == "/api/resume":
            name = safe_filename(qs.get("name", [""])[0])
            fpath = os.path.join(RESUME_DIR, name)
            if os.path.isfile(fpath):
                os.remove(fpath)
                self._send_json({"status": "ok"})
            else:
                self._send_json({"error": "not found"}, 404)
            return

        self._send_json({"error": "Unknown endpoint"}, 404)

    # ---------- API implementations ----------
    def _handle_api_get(self, path, qs):
        if path == "/api/jd":
            content = ""
            if os.path.isfile(JD_FILE):
                with open(JD_FILE, "r", encoding="utf-8") as f:
                    content = f.read()
            self._send_json({"content": content})
            return

        if path == "/api/resumes":
            files = sorted(f for f in os.listdir(RESUME_DIR) if f.endswith(".txt"))
            listing = []
            for f in files:
                with open(os.path.join(RESUME_DIR, f), "r", encoding="utf-8", errors="replace") as fh:
                    text = fh.read()
                listing.append({"filename": f, "size": len(text)})
            self._send_json({"resumes": listing})
            return

        if path == "/api/results":
            ranking_path = os.path.join(OUTPUT_DIR, "ranking.json")
            if not os.path.isfile(ranking_path):
                self._send_json({"error": "No analysis has been run yet."}, 404)
                return
            with open(ranking_path, "r", encoding="utf-8") as f:
                self._send_json(json.load(f))
            return

        if path == "/api/candidate":
            rank = qs.get("rank", ["1"])[0]
            cand_path = os.path.join(OUTPUT_DIR, f"candidate_{rank}.json")
            if not os.path.isfile(cand_path):
                self._send_json({"error": "Candidate not found"}, 404)
                return
            with open(cand_path, "r", encoding="utf-8") as f:
                self._send_json(json.load(f))
            return

        if path == "/api/export/json":
            self._send_file(os.path.join(OUTPUT_DIR, "ranking.json"), "application/json")
            return

        if path == "/api/export/report":
            self._send_file(os.path.join(OUTPUT_DIR, "report.txt"), "text/plain; charset=utf-8")
            return

        self._send_json({"error": "Unknown endpoint"}, 404)

    def _run_analysis(self):
        if not os.path.isfile(JD_FILE):
            self._send_json({"error": "No job description has been submitted yet."}, 400)
            return

        resumes = [f for f in os.listdir(RESUME_DIR) if f.endswith(".txt")]
        if not resumes:
            self._send_json({"error": "No resumes have been uploaded yet."}, 400)
            return

        if not os.path.isfile(ENGINE_BIN):
            self._send_json({
                "error": "C engine binary not found. Run 'make' in the project root first."
            }, 500)
            return

        # clear previous outputs so stale candidate_N.json files can't leak through
        for f in os.listdir(OUTPUT_DIR):
            if f.endswith(".json") or f.endswith(".txt"):
                os.remove(os.path.join(OUTPUT_DIR, f))

        try:
            proc = subprocess.run(
                [ENGINE_BIN, JD_FILE, RESUME_DIR, OUTPUT_DIR],
                capture_output=True, text=True, timeout=30
            )
        except Exception as e:
            self._send_json({"error": f"Failed to run analysis engine: {e}"}, 500)
            return

        ranking_path = os.path.join(OUTPUT_DIR, "ranking.json")
        if proc.returncode != 0 or not os.path.isfile(ranking_path):
            self._send_json({
                "error": "Analysis engine failed.",
                "engine_stdout": proc.stdout,
                "engine_stderr": proc.stderr,
            }, 500)
            return

        with open(ranking_path, "r", encoding="utf-8") as f:
            ranking = json.load(f)

        self._send_json({
            "status": "ok",
            "engine_log": proc.stdout,
            "ranking": ranking,
        })

    def _reset_workspace(self):
        for f in os.listdir(RESUME_DIR):
            os.remove(os.path.join(RESUME_DIR, f))
        for f in os.listdir(OUTPUT_DIR):
            if f.endswith(".json") or f.endswith(".txt"):
                os.remove(os.path.join(OUTPUT_DIR, f))
        if os.path.isfile(JD_FILE):
            os.remove(JD_FILE)
        self._send_json({"status": "ok"})


class ThreadingHTTPServer(socketserver.ThreadingMixIn, http.server.HTTPServer):
    daemon_threads = True
    allow_reuse_address = True


def main():
    print("DEBUG ROOT_DIR:", ROOT_DIR)
    print("DEBUG ENGINE_BIN:", ENGINE_BIN)
    print("DEBUG EXISTS:", os.path.isfile(ENGINE_BIN))
    with ThreadingHTTPServer(("0.0.0.0", PORT), HireLensHandler) as httpd:
        print(f"HireLens server running at http://localhost:{PORT}")
        print(f"Engine binary: {ENGINE_BIN}")
        print("Press Ctrl+C to stop.")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down.")


if __name__ == "__main__":
    main()
