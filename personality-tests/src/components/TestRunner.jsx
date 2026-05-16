import { useState } from "react";
import { mbtiQuestions, scoreMBTI } from "../data/mbti";
import { discQuestions, scoreDISC } from "../data/disc";
import { cliftonQuestions, cliftonScale, scoreClifton } from "../data/clifton";
import { enneagramQuestions, enneagramScale, scoreEnneagram } from "../data/enneagram";

function getConfig(testId) {
  switch (testId) {
    case "mbti": return { questions: mbtiQuestions, type: "binary" };
    case "disc": return { questions: discQuestions, type: "multi" };
    case "clifton": return { questions: cliftonQuestions, type: "scale", scale: cliftonScale };
    case "enneagram": return { questions: enneagramQuestions, type: "scale", scale: enneagramScale };
    default: return null;
  }
}

function scoreTest(testId, answers) {
  switch (testId) {
    case "mbti": return scoreMBTI(answers);
    case "disc": return scoreDISC(answers);
    case "clifton": return scoreClifton(answers);
    case "enneagram": return scoreEnneagram(answers);
    default: return {};
  }
}

const TEST_LABELS = { mbti: "Myers-Briggs", disc: "DISC", clifton: "Clifton Strengths", enneagram: "Enneagram" };

export default function TestRunner({ name, testId, onComplete, onBack }) {
  const config = getConfig(testId);
  const [current, setCurrent] = useState(0);
  const [answers, setAnswers] = useState([]);
  const [selected, setSelected] = useState(null);

  const q = config.questions[current];
  const progress = ((current) / config.questions.length) * 100;

  function handleAnswer(value) {
    setSelected(value);
  }

  function handleNext() {
    if (selected === null) return;
    const newAnswers = [...answers, selected];
    if (current + 1 >= config.questions.length) {
      const result = scoreTest(testId, newAnswers);
      onComplete(result);
    } else {
      setAnswers(newAnswers);
      setCurrent(c => c + 1);
      setSelected(null);
    }
  }

  function handlePrev() {
    if (current === 0) { onBack(); return; }
    const prev = answers[current - 1];
    setAnswers(a => a.slice(0, -1));
    setCurrent(c => c - 1);
    setSelected(prev);
  }

  return (
    <div className="runner-container">
      <div className="runner-card">
        <div className="runner-header">
          <div className="runner-meta">
            <span className="runner-name">{name}</span>
            <span className="runner-test">{TEST_LABELS[testId]}</span>
          </div>
          <div className="question-counter">Question {current + 1} of {config.questions.length}</div>
        </div>

        <div className="progress-bar-track">
          <div className="progress-bar-fill" style={{ width: `${progress}%` }} />
        </div>

        <div className="question-area">
          <div className="question-number">Q{current + 1}</div>
          <div className="question-text">{q.text}</div>

          {config.type === "binary" && (
            <div className="options-list">
              {q.options.map((opt, i) => (
                <button
                  key={i}
                  className={`option-btn ${selected === opt.score ? "option-selected" : ""}`}
                  onClick={() => handleAnswer(opt.score)}
                >
                  <span className="option-letter">{String.fromCharCode(65 + i)}</span>
                  {opt.text}
                </button>
              ))}
            </div>
          )}

          {config.type === "multi" && (
            <div className="options-list">
              {q.options.map((opt, i) => (
                <button
                  key={i}
                  className={`option-btn ${selected === opt.score ? "option-selected" : ""}`}
                  onClick={() => handleAnswer(opt.score)}
                >
                  <span className="option-letter">{String.fromCharCode(65 + i)}</span>
                  {opt.text}
                </button>
              ))}
            </div>
          )}

          {config.type === "scale" && (
            <div className="scale-container">
              <div className="scale-labels">
                <span>{config.scale[0].label}</span>
                <span>{config.scale[config.scale.length - 1].label}</span>
              </div>
              <div className="scale-buttons">
                {config.scale.map(s => (
                  <button
                    key={s.value}
                    className={`scale-btn ${selected === s.value ? "scale-selected" : ""}`}
                    onClick={() => handleAnswer(s.value)}
                  >
                    {s.value}
                  </button>
                ))}
              </div>
            </div>
          )}
        </div>

        <div className="runner-footer">
          <button className="btn-secondary" onClick={handlePrev}>
            {current === 0 ? "Exit" : "Back"}
          </button>
          <button
            className="btn-primary"
            onClick={handleNext}
            disabled={selected === null}
          >
            {current + 1 === config.questions.length ? "See Results" : "Next"}
          </button>
        </div>
      </div>
    </div>
  );
}
