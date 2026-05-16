import { getResults, clearResults } from "../utils/storage";
import { mbtiTypes } from "../data/mbti";
import { discProfiles } from "../data/disc";
import { enneagramTypes } from "../data/enneagram";
import { domainColors } from "../data/clifton";
import { useState } from "react";

const TEST_LABELS = { mbti: "Myers-Briggs", disc: "DISC", clifton: "Clifton Strengths", enneagram: "Enneagram" };
const COLORS = { mbti: "#3498db", disc: "#e74c3c", clifton: "#8e44ad", enneagram: "#16a085" };

function getResultSummary(testId, result) {
  switch (testId) {
    case "mbti": return { main: result.type, sub: mbtiTypes[result.type]?.name || "" };
    case "disc": return { main: result.primary, sub: discProfiles[result.primary]?.name || "" };
    case "clifton": return { main: result.primaryDomain, sub: result.top5?.map(s => s.theme).join(", ") || "" };
    case "enneagram": return { main: `Type ${result.primaryType}`, sub: enneagramTypes[result.primaryType]?.name || "" };
    default: return { main: "—", sub: "" };
  }
}

function formatDate(iso) {
  try {
    return new Date(iso).toLocaleDateString(undefined, { month: "short", day: "numeric", year: "numeric" });
  } catch { return ""; }
}

export default function Leaderboard({ onHome }) {
  const [filter, setFilter] = useState("all");
  const [confirmClear, setConfirmClear] = useState(false);
  const allResults = getResults();

  const filtered = filter === "all" ? allResults : allResults.filter(r => r.testId === filter);
  const sorted = [...filtered].sort((a, b) => new Date(b.completedAt) - new Date(a.completedAt));

  const stats = { mbti: 0, disc: 0, clifton: 0, enneagram: 0 };
  allResults.forEach(r => { if (stats[r.testId] !== undefined) stats[r.testId]++; });
  const uniqueUsers = new Set(allResults.map(r => r.name.toLowerCase())).size;

  function handleClear() {
    if (confirmClear) { clearResults(); window.location.reload(); }
    else setConfirmClear(true);
  }

  return (
    <div className="board-container">
      <div className="board-card">
        <div className="board-header">
          <div>
            <h2 className="board-title">Leaderboard</h2>
            <p className="board-subtitle">All completed assessments</p>
          </div>
          <button className="btn-secondary" onClick={onHome}>New Assessment</button>
        </div>

        <div className="stats-row">
          <div className="stat-box">
            <div className="stat-num">{allResults.length}</div>
            <div className="stat-label">Total Tests</div>
          </div>
          <div className="stat-box">
            <div className="stat-num">{uniqueUsers}</div>
            <div className="stat-label">Unique Users</div>
          </div>
          {Object.entries(stats).map(([id, count]) => (
            <div key={id} className="stat-box" style={{ borderBottom: `3px solid ${COLORS[id]}` }}>
              <div className="stat-num">{count}</div>
              <div className="stat-label">{TEST_LABELS[id]}</div>
            </div>
          ))}
        </div>

        <div className="filter-tabs">
          {[["all", "All Tests"], ["mbti", "MBTI"], ["disc", "DISC"], ["clifton", "Clifton"], ["enneagram", "Enneagram"]].map(([val, label]) => (
            <button
              key={val}
              className={`filter-tab ${filter === val ? "filter-active" : ""}`}
              onClick={() => setFilter(val)}
              style={filter === val && val !== "all" ? { background: COLORS[val], borderColor: COLORS[val], color: "#fff" } : {}}
            >
              {label}
            </button>
          ))}
        </div>

        {sorted.length === 0 ? (
          <div className="empty-state">
            <div className="empty-icon">&#128202;</div>
            <p>No results yet. Take your first assessment!</p>
            <button className="btn-primary" onClick={onHome}>Start Now</button>
          </div>
        ) : (
          <div className="board-table-wrap">
            <table className="board-table">
              <thead>
                <tr>
                  <th>#</th>
                  <th>Name</th>
                  <th>Assessment</th>
                  <th>Result</th>
                  <th>Profile</th>
                  <th>Date</th>
                </tr>
              </thead>
              <tbody>
                {sorted.map((r, i) => {
                  const summary = getResultSummary(r.testId, r.result);
                  return (
                    <tr key={r.id} className={i % 2 === 0 ? "row-even" : "row-odd"}>
                      <td className="rank-cell">{i + 1}</td>
                      <td className="name-cell">{r.name}</td>
                      <td>
                        <span className="test-badge" style={{ background: COLORS[r.testId] }}>
                          {TEST_LABELS[r.testId]}
                        </span>
                      </td>
                      <td className="result-main">{summary.main}</td>
                      <td className="result-sub">{summary.sub}</td>
                      <td className="date-cell">{formatDate(r.completedAt)}</td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
          </div>
        )}

        {allResults.length > 0 && (
          <div className="clear-section">
            <button className={`btn-danger ${confirmClear ? "confirm" : ""}`} onClick={handleClear}>
              {confirmClear ? "Confirm Clear All Data" : "Clear All Results"}
            </button>
            {confirmClear && (
              <button className="btn-secondary" onClick={() => setConfirmClear(false)}>Cancel</button>
            )}
          </div>
        )}
      </div>
    </div>
  );
}
