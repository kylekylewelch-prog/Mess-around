export const cliftonQuestions = [
  // Strategic Thinking
  { id: 1, theme: "Analytical", domain: "Strategic Thinking", text: "I enjoy dissecting data and finding patterns others miss." },
  { id: 2, theme: "Context", domain: "Strategic Thinking", text: "I look to history and past experiences to understand the present." },
  { id: 3, theme: "Futuristic", domain: "Strategic Thinking", text: "I am energized by visions of the future and what could be." },
  { id: 4, theme: "Ideation", domain: "Strategic Thinking", text: "I love connecting seemingly unrelated concepts into one original idea." },
  { id: 5, theme: "Input", domain: "Strategic Thinking", text: "I am naturally curious and collect interesting information and objects." },
  { id: 6, theme: "Intellection", domain: "Strategic Thinking", text: "I enjoy deep thinking and intellectual discussion." },
  { id: 7, theme: "Learner", domain: "Strategic Thinking", text: "I am energized by the journey of learning something new." },
  { id: 8, theme: "Strategic", domain: "Strategic Thinking", text: "I quickly spot relevant patterns and find the best route forward." },

  // Executing
  { id: 9, theme: "Achiever", domain: "Executing", text: "I have a constant drive to accomplish tasks and get things done." },
  { id: 10, theme: "Arranger", domain: "Executing", text: "I enjoy juggling many variables to find the best configuration." },
  { id: 11, theme: "Belief", domain: "Executing", text: "I have core values that give my life purpose and direction." },
  { id: 12, theme: "Consistency", domain: "Executing", text: "I am keenly aware of the importance of treating people the same." },
  { id: 13, theme: "Deliberative", domain: "Executing", text: "I identify obstacles and risks and plan around them carefully." },
  { id: 14, theme: "Discipline", domain: "Executing", text: "I enjoy creating routines and structure to manage my life." },
  { id: 15, theme: "Focus", domain: "Executing", text: "I set goals and pursue them with determination, filtering distractions." },
  { id: 16, theme: "Responsibility", domain: "Executing", text: "I take psychological ownership of my commitments and deliver on them." },
  { id: 17, theme: "Restorative", domain: "Executing", text: "I am energized by finding solutions to difficult problems." },

  // Influencing
  { id: 18, theme: "Activator", domain: "Influencing", text: "I am impatient for action and love to get things moving." },
  { id: 19, theme: "Command", domain: "Influencing", text: "I have presence and take control of situations." },
  { id: 20, theme: "Communication", domain: "Influencing", text: "I find it easy to put thoughts into words and engage an audience." },
  { id: 21, theme: "Competition", domain: "Influencing", text: "I measure my progress against others and am driven to win." },
  { id: 22, theme: "Maximizer", domain: "Influencing", text: "I focus on strengths to make average things excellent." },
  { id: 23, theme: "Self-Assurance", domain: "Influencing", text: "I have confidence in my abilities and my decisions." },
  { id: 24, theme: "Significance", domain: "Influencing", text: "I want to make a big impact and be recognized for it." },
  { id: 25, theme: "Woo", domain: "Influencing", text: "I love meeting new people and winning them over." },

  // Relationship Building
  { id: 26, theme: "Adaptability", domain: "Relationship Building", text: "I live in the moment and easily adjust to change." },
  { id: 27, theme: "Connectedness", domain: "Relationship Building", text: "I believe everything happens for a reason and all people are connected." },
  { id: 28, theme: "Developer", domain: "Relationship Building", text: "I see the potential in others and invest in their growth." },
  { id: 29, theme: "Empathy", domain: "Relationship Building", text: "I can sense the emotions of others and feel what they feel." },
  { id: 30, theme: "Harmony", domain: "Relationship Building", text: "I look for consensus and avoid conflict." },
  { id: 31, theme: "Includer", domain: "Relationship Building", text: "I make sure people feel welcomed and part of the group." },
  { id: 32, theme: "Individualization", domain: "Relationship Building", text: "I am intrigued by the unique qualities of each person." },
  { id: 33, theme: "Positivity", domain: "Relationship Building", text: "I have contagious enthusiasm and see the best in situations." },
  { id: 34, theme: "Relator", domain: "Relationship Building", text: "I enjoy close relationships and find deep satisfaction in working with friends." },
];

export const cliftonScale = [
  { value: 1, label: "Not at all like me" },
  { value: 2, label: "Slightly like me" },
  { value: 3, label: "Somewhat like me" },
  { value: 4, label: "Mostly like me" },
  { value: 5, label: "Completely like me" },
];

export const domainColors = {
  "Strategic Thinking": "#9b59b6",
  "Executing": "#e74c3c",
  "Influencing": "#f39c12",
  "Relationship Building": "#27ae60",
};

export function scoreClifton(answers) {
  const scored = cliftonQuestions.map((q, i) => ({ ...q, score: answers[i] }));
  const top5 = [...scored].sort((a, b) => b.score - a.score).slice(0, 5);
  const domainScores = {};
  scored.forEach(q => {
    domainScores[q.domain] = (domainScores[q.domain] || 0) + q.score;
  });
  const primaryDomain = Object.entries(domainScores).sort((a, b) => b[1] - a[1])[0][0];
  return { top5, domainScores, primaryDomain };
}
