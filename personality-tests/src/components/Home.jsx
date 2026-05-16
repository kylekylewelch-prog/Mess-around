import { useState } from "react";

const TESTS = [
  { id: "mbti", label: "Myers-Briggs (MBTI)", desc: "20 questions • Discover your 4-letter personality type across 16 possible results." },
  { id: "disc", label: "DISC Assessment", desc: "20 questions • Identify your dominant behavioral style: Dominant, Influential, Steady, or Conscientious." },
  { id: "clifton", label: "Clifton Strengths", desc: "34 questions • Uncover your top 5 personal strengths from 34 talent themes." },
  { id: "enneagram", label: "Enneagram", desc: "36 questions • Discover your core personality type among 9 interconnected types." },
];

export default function Home({ onStart }) {
  const [name, setName] = useState("");
  const [test, setTest] = useState("");
  const [error, setError] = useState("");

  function handleStart() {
    if (!name.trim()) { setError("Please enter your name."); return; }
    if (!test) { setError("Please select a personality test."); return; }
    setError("");
    onStart(name.trim(), test);
  }

  const selected = TESTS.find(t => t.id === test);

  return (
    <div className="home-container">
      <div className="home-card">
        <div className="logo-area">
          <div className="logo-icon">&#9670;</div>
          <h1>Personality Assessment Hub</h1>
          <p className="subtitle">Explore who you are through evidence-based personality frameworks</p>
        </div>

        <div className="form-section">
          <label className="field-label">Your Name</label>
          <input
            className="text-input"
            type="text"
            placeholder="Enter your full name"
            value={name}
            onChange={e => { setName(e.target.value); setError(""); }}
            onKeyDown={e => e.key === "Enter" && handleStart()}
          />

          <label className="field-label">Select Assessment</label>
          <select
            className="select-input"
            value={test}
            onChange={e => { setTest(e.target.value); setError(""); }}
          >
            <option value="">-- Choose a personality test --</option>
            {TESTS.map(t => (
              <option key={t.id} value={t.id}>{t.label}</option>
            ))}
          </select>

          {selected && (
            <div className="test-preview">
              <p>{selected.desc}</p>
            </div>
          )}

          {error && <div className="error-msg">{error}</div>}

          <button className="btn-primary" onClick={handleStart}>
            Begin Assessment
          </button>
        </div>

        <div className="tests-grid">
          {TESTS.map(t => (
            <div key={t.id} className={`test-card ${test === t.id ? "active" : ""}`} onClick={() => { setTest(t.id); setError(""); }}>
              <div className="test-card-name">{t.label}</div>
              <div className="test-card-desc">{t.desc}</div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
