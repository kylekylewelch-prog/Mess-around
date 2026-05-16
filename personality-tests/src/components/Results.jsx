import { discProfiles } from "../data/disc";
import { enneagramTypes } from "../data/enneagram";
import { domainColors } from "../data/clifton";

function MBTIResult({ result }) {
  const { type, details, counts } = result;
  const pairs = [
    { a: "E", b: "I", aLabel: "Extraversion", bLabel: "Introversion" },
    { a: "S", b: "N", aLabel: "Sensing", bLabel: "Intuition" },
    { a: "T", b: "F", aLabel: "Thinking", bLabel: "Feeling" },
    { a: "J", b: "P", aLabel: "Judging", bLabel: "Perceiving" },
  ];
  return (
    <div>
      <div className="result-type-badge mbti-badge">{type}</div>
      <div className="result-type-name">{details.name}</div>
      <p className="result-desc">{details.desc}</p>
      <div className="dimension-bars">
        {pairs.map(p => {
          const total = counts[p.a] + counts[p.b];
          const pct = total > 0 ? Math.round((counts[p.a] / total) * 100) : 50;
          return (
            <div key={p.a} className="dim-row">
              <span className="dim-label">{p.aLabel} ({p.a})</span>
              <div className="dim-bar-track">
                <div className="dim-bar-fill" style={{ width: `${pct}%` }} />
              </div>
              <span className="dim-label right">{p.bLabel} ({p.b})</span>
            </div>
          );
        })}
      </div>
    </div>
  );
}

function DISCResult({ result }) {
  const { primary, totals, profile } = result;
  const max = Math.max(...Object.values(totals));
  const colors = { D: "#e74c3c", I: "#f39c12", S: "#27ae60", C: "#2980b9" };
  return (
    <div>
      <div className="result-type-badge disc-badge" style={{ background: colors[primary] }}>{primary}</div>
      <div className="result-type-name">{profile.name}</div>
      <p className="result-desc">{profile.desc}</p>
      <div className="disc-bars">
        {Object.entries(totals).map(([dim, score]) => (
          <div key={dim} className="disc-bar-row">
            <span className="disc-dim-label">{discProfiles[dim].name} ({dim})</span>
            <div className="disc-bar-track">
              <div className="disc-bar-fill" style={{ width: `${(score / max) * 100}%`, background: colors[dim] }} />
              <span className="disc-bar-score">{score}</span>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}

function CliftonResult({ result }) {
  const { top5, domainScores, primaryDomain } = result;
  return (
    <div>
      <div className="result-type-badge clifton-badge" style={{ background: domainColors[primaryDomain] }}>
        {primaryDomain}
      </div>
      <div className="result-type-name">Your Top 5 Strengths</div>
      <p className="result-desc">Your dominant domain is <strong>{primaryDomain}</strong>. Your unique strength profile below represents your most natural talents.</p>
      <div className="top5-list">
        {top5.map((s, i) => (
          <div key={s.theme} className="strength-item" style={{ borderLeft: `4px solid ${domainColors[s.domain]}` }}>
            <div className="strength-rank">#{i + 1}</div>
            <div className="strength-info">
              <div className="strength-name">{s.theme}</div>
              <div className="strength-domain" style={{ color: domainColors[s.domain] }}>{s.domain}</div>
            </div>
            <div className="strength-score">{s.score}/5</div>
          </div>
        ))}
      </div>
    </div>
  );
}

function EnneagramResult({ result }) {
  const { primaryType, totals, type, secondType } = result;
  const max = Math.max(...Object.values(totals));
  return (
    <div>
      <div className="result-type-badge ennea-badge">Type {primaryType}</div>
      <div className="result-type-name">{type.name}</div>
      <p className="result-desc">{type.desc}</p>
      <p className="virtue-line">Core Virtue: <strong>{type.virtue}</strong></p>
      <div className="ennea-bars">
        {Object.entries(totals).sort((a, b) => b[1] - a[1]).map(([t, score]) => (
          <div key={t} className="ennea-bar-row">
            <span className="ennea-type-label">Type {t}: {enneagramTypes[parseInt(t)].name}</span>
            <div className="ennea-bar-track">
              <div
                className="ennea-bar-fill"
                style={{
                  width: `${(score / max) * 100}%`,
                  background: parseInt(t) === primaryType ? "#8e44ad" : "#bdc3c7"
                }}
              />
              <span className="ennea-score">{score}</span>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}

const TEST_LABELS = { mbti: "Myers-Briggs", disc: "DISC", clifton: "Clifton Strengths", enneagram: "Enneagram" };

export default function Results({ name, testId, result, onRetake, onLeaderboard, onHome }) {
  return (
    <div className="results-container">
      <div className="results-card">
        <div className="results-header">
          <div className="results-congrats">Assessment Complete</div>
          <h2 className="results-name">{name}'s {TEST_LABELS[testId]} Results</h2>
        </div>

        <div className="results-body">
          {testId === "mbti" && <MBTIResult result={result} />}
          {testId === "disc" && <DISCResult result={result} />}
          {testId === "clifton" && <CliftonResult result={result} />}
          {testId === "enneagram" && <EnneagramResult result={result} />}
        </div>

        <div className="results-actions">
          <button className="btn-secondary" onClick={onHome}>Take Another Test</button>
          <button className="btn-outline" onClick={onRetake}>Retake This Test</button>
          <button className="btn-primary" onClick={onLeaderboard}>View Leaderboard</button>
        </div>
      </div>
    </div>
  );
}
