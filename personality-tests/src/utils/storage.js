const KEY = "personality_results";

export function saveResult(result) {
  const all = getResults();
  all.push({ ...result, id: Date.now(), completedAt: new Date().toISOString() });
  localStorage.setItem(KEY, JSON.stringify(all));
}

export function getResults() {
  try {
    return JSON.parse(localStorage.getItem(KEY)) || [];
  } catch {
    return [];
  }
}

export function clearResults() {
  localStorage.removeItem(KEY);
}
