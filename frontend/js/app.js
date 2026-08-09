/* ==================================================================
 * HireLens frontend application logic.
 * Vanilla JS only -- no frameworks, no build step. Talks to the
 * Python glue server, which in turn talks to the C engine.
 * ================================================================== */

const SAMPLE_JD = `TITLE: Backend Software Engineer (Junior-Mid Level)
REQUIRED_SKILLS: Java, Python, SQL, DSA, DBMS, REST API, Git
PREFERRED_SKILLS: Docker, Kubernetes, AWS, ML, Agile
MIN_EXPERIENCE_YEARS: 2
EDUCATION: B.Tech Computer Science, BCA, B.E Computer Science
KEYWORDS: scalable, microservices, agile, teamwork, problem solving, backend, performance`;

const SAMPLE_RESUME_NAMES = [
  "aisha_sharma.txt", "rohan_verma.txt", "priya_nair.txt", "karthik_iyer.txt", "sneha_reddy.txt"
];

let state = {
  jdSet: false,
  resumes: [],
  ranking: null,
};

/* ---------------- Navigation ---------------- */
function showView(name) {
  document.querySelectorAll(".view").forEach(v => v.classList.add("hidden"));
  document.getElementById("view-" + name).classList.remove("hidden");
  document.querySelectorAll(".nav-item").forEach(b => b.classList.toggle("active", b.dataset.view === name));
  if (name === "ranking") renderRankingTable();
  if (name === "candidate") renderCandidateSelect();
  if (name === "analyze") refreshAnalyzeSummary();
}

document.querySelectorAll(".nav-item").forEach(btn => {
  btn.addEventListener("click", () => showView(btn.dataset.view));
});
document.querySelectorAll("[data-goto]").forEach(btn => {
  btn.addEventListener("click", () => showView(btn.dataset.goto));
});

/* ---------------- Toast helper ---------------- */
function showToast(id, message, ok = true) {
  const el = document.getElementById(id);
  el.textContent = message;
  el.className = "toast show " + (ok ? "ok" : "err");
  setTimeout(() => el.classList.remove("show"), 4000);
}

/* ---------------- API helpers ---------------- */
async function api(path, options = {}) {
  const res = await fetch(path, options);
  let data = {};
  try { data = await res.json(); } catch (e) { /* non-json response */ }
  if (!res.ok) throw new Error(data.error || `Request failed (${res.status})`);
  return data;
}

/* ---------------- JD view ---------------- */
const jdTextarea = document.getElementById("jdTextarea");

document.getElementById("loadSampleJD").addEventListener("click", () => {
  jdTextarea.value = SAMPLE_JD;
});

document.getElementById("uploadJDBtn").addEventListener("click", () => {
  document.getElementById("jdFileInput").click();
});
document.getElementById("jdFileInput").addEventListener("change", async (e) => {
  const file = e.target.files[0];
  if (!file) return;
  jdTextarea.value = await file.text();
});

document.getElementById("saveJDBtn").addEventListener("click", async () => {
  try {
    await api("/api/jd", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ content: jdTextarea.value }),
    });
    state.jdSet = jdTextarea.value.trim().length > 0;
    showToast("jdToast", "Job description saved.", true);
    updateDashboardStats();
  } catch (err) {
    showToast("jdToast", err.message, false);
  }
});

/* ---------------- Resumes view ---------------- */
const resumeGrid = document.getElementById("resumeGrid");

async function refreshResumeList() {
  const data = await api("/api/resumes");
  state.resumes = data.resumes || [];
  renderResumeGrid();
  updateDashboardStats();
}

function renderResumeGrid() {
  if (state.resumes.length === 0) {
    resumeGrid.innerHTML = `<p class="empty-state">No resumes loaded yet. Upload files or load the sample set.</p>`;
    return;
  }
  resumeGrid.innerHTML = state.resumes.map(r => `
    <div class="resume-card">
      <strong>${escapeHtml(r.filename)}</strong>
      <span>${r.size} chars</span>
      <button data-name="${escapeHtml(r.filename)}">Remove</button>
    </div>
  `).join("");
  resumeGrid.querySelectorAll("button[data-name]").forEach(btn => {
    btn.addEventListener("click", async () => {
      await api(`/api/resume?name=${encodeURIComponent(btn.dataset.name)}`, { method: "DELETE" });
      refreshResumeList();
    });
  });
}

document.getElementById("uploadResumeBtn").addEventListener("click", () => {
  document.getElementById("resumeFileInput").click();
});
document.getElementById("resumeFileInput").addEventListener("change", async (e) => {
  const files = Array.from(e.target.files);
  for (const file of files) {
    const content = await file.text();
    await api("/api/resume", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ filename: file.name, content }),
    });
  }
  showToast("resumeToast", `Uploaded ${files.length} resume(s).`, true);
  refreshResumeList();
});

document.getElementById("loadSampleResumes").addEventListener("click", async () => {
  try {
    for (const name of SAMPLE_RESUME_NAMES) {
      const res = await fetch(`/data/resumes/${name}`);
      if (!res.ok) throw new Error("Could not load bundled sample resumes.");
      const content = await res.text();
      await api("/api/resume", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ filename: name, content }),
      });
    }
    showToast("resumeToast", "Loaded 5 sample resumes.", true);
    refreshResumeList();
  } catch (err) {
    showToast("resumeToast", err.message, false);
  }
});

document.getElementById("clearResumesBtn").addEventListener("click", async () => {
  for (const r of state.resumes) {
    await api(`/api/resume?name=${encodeURIComponent(r.filename)}`, { method: "DELETE" });
  }
  showToast("resumeToast", "Cleared all resumes.", true);
  refreshResumeList();
});

/* ---------------- Analyze view ---------------- */
async function refreshAnalyzeSummary() {
  const jd = await api("/api/jd");
  state.jdSet = (jd.content || "").trim().length > 0;
  document.getElementById("analyzeJDStatus").textContent = state.jdSet ? "JD set ✓" : "JD not set";
  document.getElementById("analyzeResumeStatus").textContent = `${state.resumes.length} resumes`;
}

document.getElementById("runAnalysisBtn").addEventListener("click", async () => {
  const log = document.getElementById("engineLog");
  log.textContent = "Running C engine…";
  try {
    const data = await api("/api/analyze", { method: "POST" });
    log.textContent = data.engine_log || "Analysis complete.";
    state.ranking = data.ranking;
    updateDashboardStats();
    showView("ranking");
  } catch (err) {
    log.textContent = "ERROR: " + err.message;
  }
});

/* ---------------- Ranking view ---------------- */
function statusClass(status) {
  if (status === "Highly Suitable") return "status-highly";
  if (status === "Suitable") return "status-suitable";
  if (status === "Partially Suitable") return "status-partially";
  return "status-not";
}

async function renderRankingTable() {
  if (!state.ranking) {
    try { state.ranking = await api("/api/results"); } catch (e) { /* not run yet */ }
  }
  const tbody = document.getElementById("rankingTbody");
  if (!state.ranking || !state.ranking.candidates || state.ranking.candidates.length === 0) {
    tbody.innerHTML = `<tr><td colspan="10" class="empty-state">Run an analysis to see ranked candidates.</td></tr>`;
    return;
  }
  tbody.innerHTML = state.ranking.candidates.map(c => `
    <tr>
      <td><span class="rank-badge ${c.rank === 1 ? 'top1' : ''}">${c.rank}</span></td>
      <td>${escapeHtml(c.name)}</td>
      <td class="score-cell">${c.overall_score.toFixed(1)}</td>
      <td>${c.skill_score.toFixed(0)}</td>
      <td>${c.experience_score.toFixed(0)}</td>
      <td>${c.education_score.toFixed(0)}</td>
      <td>${c.project_score.toFixed(0)}</td>
      <td>${c.keyword_score.toFixed(0)}</td>
      <td><span class="status-pill ${statusClass(c.status)}">${c.status}</span></td>
      <td><button class="view-btn" data-rank="${c.rank}">View →</button></td>
    </tr>
  `).join("");

  tbody.querySelectorAll("button[data-rank]").forEach(btn => {
    btn.addEventListener("click", () => {
      showView("candidate");
      document.getElementById("candidateSelect").value = btn.dataset.rank;
      loadCandidate(btn.dataset.rank);
    });
  });
}

/* ---------------- Candidate detail view ---------------- */
function renderCandidateSelect() {
  const select = document.getElementById("candidateSelect");
  if (!state.ranking || !state.ranking.candidates) {
    select.innerHTML = `<option>No candidates yet</option>`;
    return;
  }
  select.innerHTML = state.ranking.candidates.map(c =>
    `<option value="${c.rank}">#${c.rank} — ${escapeHtml(c.name)} (${c.overall_score.toFixed(1)})</option>`
  ).join("");
  select.onchange = () => loadCandidate(select.value);
  loadCandidate(select.value || state.ranking.candidates[0].rank);
}

async function loadCandidate(rank) {
  const container = document.getElementById("candidateDetail");
  try {
    const c = await api(`/api/candidate?rank=${rank}`);
    container.innerHTML = renderCandidateHtml(c);
  } catch (err) {
    container.innerHTML = `<p class="empty-state">${escapeHtml(err.message)}</p>`;
  }
}

function meterRow(label, value) {
  return `
    <div class="score-meter">
      <div class="label"><span>${label}</span><span class="value">${value.toFixed(0)}</span></div>
      <div class="meter-track"><div class="meter-fill" style="width:${Math.min(100, Math.max(0, value))}%"></div></div>
    </div>`;
}

function chipList(items, cls) {
  if (!items || items.length === 0) return `<p class="empty-state" style="padding:4px 0;">None</p>`;
  return `<div class="chip-list">${items.map(i => {
    if (typeof i === "string") return `<span class="chip ${cls}">${escapeHtml(i)}</span>`;
    return `<span class="chip ${cls} ${i.via_synonym ? 'synonym' : ''}">${escapeHtml(i.skill)}</span>`;
  }).join("")}</div>`;
}

function renderCandidateHtml(c) {
  return `
    <div class="cand-header">
      <div class="cand-identity">
        <h2>#${c.rank} ${escapeHtml(c.name)}</h2>
        <p>${escapeHtml(c.email)} · ${escapeHtml(c.phone)} · ${c.experience_years} yr(s) experience · ${escapeHtml(c.source_file)}</p>
      </div>
      <span class="status-pill ${statusClass(c.status)}" style="font-size:0.9rem;padding:8px 16px;">${c.status} — ${c.overall_score.toFixed(1)}/100</span>
    </div>

    <div class="score-breakdown">
      ${meterRow("Skills · 40%", c.skill_score)}
      ${meterRow("Experience · 20%", c.experience_score)}
      ${meterRow("Education · 15%", c.education_score)}
      ${meterRow("Projects · 15%", c.project_score)}
      ${meterRow("Keywords · 10%", c.keyword_score)}
    </div>

    <div class="detail-grid">
      <div class="detail-block">
        <h3>Matched Required Skills</h3>
        ${chipList(c.matched_required, "")}
      </div>
      <div class="detail-block">
        <h3>Missing Required Skills</h3>
        ${chipList(c.missing_required, "missing")}
      </div>
      <div class="detail-block">
        <h3>Matched Preferred Skills</h3>
        ${chipList(c.matched_preferred, "")}
      </div>
      <div class="detail-block">
        <h3>Missing Preferred Skills</h3>
        ${chipList(c.missing_preferred, "missing")}
      </div>
      <div class="detail-block">
        <h3>Strengths</h3>
        <ul class="note-list">${(c.strengths || []).map(s => `<li>${escapeHtml(s)}</li>`).join("")}</ul>
      </div>
      <div class="detail-block">
        <h3>Gaps</h3>
        <ul class="note-list gaps">${(c.gaps || []).map(s => `<li>${escapeHtml(s)}</li>`).join("")}</ul>
      </div>
    </div>

    <div class="recommendation-box">
      <strong>Recommendation.</strong> ${escapeHtml(c.recommendation)}
    </div>
  `;
}

/* ---------------- Dashboard stats ---------------- */
async function updateDashboardStats() {
  const jd = await api("/api/jd").catch(() => ({ content: "" }));
  state.jdSet = (jd.content || "").trim().length > 0;
  document.getElementById("statJD").textContent = state.jdSet ? "Configured" : "Not set";

  const resumesData = await api("/api/resumes").catch(() => ({ resumes: [] }));
  state.resumes = resumesData.resumes || [];
  document.getElementById("statResumes").textContent = state.resumes.length;

  let ranking = state.ranking;
  if (!ranking) {
    ranking = await api("/api/results").catch(() => null);
    if (ranking) state.ranking = ranking;
  }

  const candidates = ranking && ranking.candidates ? ranking.candidates : [];
  document.getElementById("statAnalyzed").textContent = candidates.length;
  document.getElementById("statHighly").textContent = candidates.filter(c => c.status === "Highly Suitable").length;

  const top = candidates[0];
  const gaugeFill = document.getElementById("heroGaugeFill");
  const gaugeValue = document.getElementById("heroGaugeValue");
  const circumference = 2 * Math.PI * 70;
  if (top) {
    const frac = Math.max(0, Math.min(100, top.overall_score)) / 100;
    gaugeFill.style.strokeDasharray = `${circumference * frac} ${circumference}`;
    gaugeValue.textContent = top.overall_score.toFixed(0);
  } else {
    gaugeFill.style.strokeDasharray = `0 ${circumference}`;
    gaugeValue.textContent = "—";
  }
}

/* ---------------- Utilities ---------------- */
function escapeHtml(str) {
  if (str === undefined || str === null) return "";
  return String(str)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;");
}

/* ---------------- Init ---------------- */
(async function init() {
  await updateDashboardStats();
  await refreshResumeList();
})();
