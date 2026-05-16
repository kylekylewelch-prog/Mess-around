import { useState } from "react";
import Home from "./components/Home";
import TestRunner from "./components/TestRunner";
import Results from "./components/Results";
import Leaderboard from "./components/Leaderboard";
import { saveResult } from "./utils/storage";
import "./App.css";

export default function App() {
  const [screen, setScreen] = useState("home");
  const [userName, setUserName] = useState("");
  const [testId, setTestId] = useState("");
  const [result, setResult] = useState(null);

  function handleStart(name, test) {
    setUserName(name);
    setTestId(test);
    setScreen("test");
  }

  function handleComplete(res) {
    setResult(res);
    saveResult({ name: userName, testId, result: res });
    setScreen("results");
  }

  function handleRetake() {
    setResult(null);
    setScreen("test");
  }

  function handleHome() {
    setResult(null);
    setTestId("");
    setScreen("home");
  }

  return (
    <div className="app">
      <nav className="top-nav">
        <div className="nav-brand" onClick={handleHome}>&#9670; Personality Hub</div>
        <div className="nav-links">
          <button className={`nav-link ${screen === "home" ? "active" : ""}`} onClick={handleHome}>Home</button>
          <button className={`nav-link ${screen === "leaderboard" ? "active" : ""}`} onClick={() => setScreen("leaderboard")}>Leaderboard</button>
        </div>
      </nav>

      <main className="main-content">
        {screen === "home" && <Home onStart={handleStart} />}
        {screen === "test" && (
          <TestRunner
            name={userName}
            testId={testId}
            onComplete={handleComplete}
            onBack={handleHome}
          />
        )}
        {screen === "results" && (
          <Results
            name={userName}
            testId={testId}
            result={result}
            onRetake={handleRetake}
            onLeaderboard={() => setScreen("leaderboard")}
            onHome={handleHome}
          />
        )}
        {screen === "leaderboard" && <Leaderboard onHome={handleHome} />}
      </main>
    </div>
  );
}
