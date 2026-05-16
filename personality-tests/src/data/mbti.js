export const mbtiQuestions = [
  // E vs I
  { id: 1, text: "At a party, you prefer to...", dimension: "EI", options: [{ text: "Interact with many people, including strangers", score: "E" }, { text: "Interact with a few close friends", score: "I" }] },
  { id: 2, text: "After a long week, you recharge by...", dimension: "EI", options: [{ text: "Going out and socializing with people", score: "E" }, { text: "Spending quiet time alone or with one close person", score: "I" }] },
  { id: 3, text: "In group settings, you tend to...", dimension: "EI", options: [{ text: "Speak up often and take the lead", score: "E" }, { text: "Listen more and speak when you have something important to say", score: "I" }] },
  { id: 4, text: "You find it easy to...", dimension: "EI", options: [{ text: "Approach strangers and start conversations", score: "E" }, { text: "Spend long periods alone without feeling lonely", score: "I" }] },
  { id: 5, text: "When working on a problem, you prefer...", dimension: "EI", options: [{ text: "Talking it through with others", score: "E" }, { text: "Thinking it through quietly on your own", score: "I" }] },

  // S vs N
  { id: 6, text: "When learning something new, you focus on...", dimension: "SN", options: [{ text: "The practical details and how it works in reality", score: "S" }, { text: "The big picture and what possibilities it opens up", score: "N" }] },
  { id: 7, text: "You trust more...", dimension: "SN", options: [{ text: "Concrete experience and proven methods", score: "S" }, { text: "Intuition and hunches about future possibilities", score: "N" }] },
  { id: 8, text: "You are more interested in...", dimension: "SN", options: [{ text: "What is actual and present", score: "S" }, { text: "What is possible and future", score: "N" }] },
  { id: 9, text: "You prefer to think about...", dimension: "SN", options: [{ text: "Facts and details that are concrete and observable", score: "S" }, { text: "Concepts, theories, and abstract ideas", score: "N" }] },
  { id: 10, text: "In your work, you value...", dimension: "SN", options: [{ text: "Precision, accuracy, and attention to detail", score: "S" }, { text: "Innovation, creativity, and new approaches", score: "N" }] },

  // T vs F
  { id: 11, text: "When making decisions, you primarily rely on...", dimension: "TF", options: [{ text: "Logic and objective analysis", score: "T" }, { text: "Personal values and how it affects people", score: "F" }] },
  { id: 12, text: "You find it more satisfying to...", dimension: "TF", options: [{ text: "Achieve a goal through rational strategy", score: "T" }, { text: "Help others and create harmony", score: "F" }] },
  { id: 13, text: "When a friend is upset, you first...", dimension: "TF", options: [{ text: "Help them analyze the situation objectively", score: "T" }, { text: "Listen and offer emotional support", score: "F" }] },
  { id: 14, text: "You believe the best decisions are made by...", dimension: "TF", options: [{ text: "Being objective and detached from emotions", score: "T" }, { text: "Considering feelings and the human impact", score: "F" }] },
  { id: 15, text: "In a disagreement, you tend to...", dimension: "TF", options: [{ text: "Focus on the facts and logical merits", score: "T" }, { text: "Consider the feelings of those involved", score: "F" }] },

  // J vs P
  { id: 16, text: "You prefer your lifestyle to be...", dimension: "JP", options: [{ text: "Planned and organized with clear goals", score: "J" }, { text: "Spontaneous and flexible", score: "P" }] },
  { id: 17, text: "When starting a project, you...", dimension: "JP", options: [{ text: "Plan carefully and follow the plan", score: "J" }, { text: "Jump in and figure it out as you go", score: "P" }] },
  { id: 18, text: "You feel better when...", dimension: "JP", options: [{ text: "Things are settled and decided", score: "J" }, { text: "Options are kept open", score: "P" }] },
  { id: 19, text: "Your workspace is typically...", dimension: "JP", options: [{ text: "Tidy and organized", score: "J" }, { text: "Flexible and adaptable to your current needs", score: "P" }] },
  { id: 20, text: "Deadlines feel...", dimension: "JP", options: [{ text: "Important — you finish well before them", score: "J" }, { text: "Like a guideline — you often work best right up to them", score: "P" }] },
];

export const mbtiTypes = {
  INTJ: { name: "The Architect", desc: "Strategic, independent, and determined. Natural planners who see the world as a place to be improved." },
  INTP: { name: "The Thinker", desc: "Analytical, objective, and reserved. Love theoretical concepts and logical reasoning." },
  ENTJ: { name: "The Commander", desc: "Bold, imaginative, and strong-willed. Natural leaders who find a way to achieve their goals." },
  ENTP: { name: "The Debater", desc: "Smart, curious, and mentally agile. Love to brainstorm and argue for the joy of it." },
  INFJ: { name: "The Advocate", desc: "Quiet, mystical, and inspiring. Idealists who see potential in people and want to help." },
  INFP: { name: "The Mediator", desc: "Poetic, kind, and altruistic. Always looking for the good in people and meaning in life." },
  ENFJ: { name: "The Protagonist", desc: "Charismatic, inspiring, and passionate. Natural leaders who captivate others." },
  ENFP: { name: "The Campaigner", desc: "Enthusiastic, creative, and sociable. See life as full of possibilities." },
  ISTJ: { name: "The Logistician", desc: "Practical, fact-minded, and reliable. Responsible people who value loyalty and hard work." },
  ISFJ: { name: "The Defender", desc: "Dedicated, warm, and protective. Take their responsibilities seriously and care deeply." },
  ESTJ: { name: "The Executive", desc: "Excellent administrators who are organized, straightforward, and committed to tradition." },
  ESFJ: { name: "The Consul", desc: "Caring, social, and popular. Attuned to other's feelings and work to keep harmony." },
  ISTP: { name: "The Virtuoso", desc: "Bold, practical, and observant. Love to explore with their hands and eyes." },
  ISFP: { name: "The Adventurer", desc: "Flexible, charming, and exploratory. Ready to push boundaries and experience new things." },
  ESTP: { name: "The Entrepreneur", desc: "Smart, energetic, and perceptive. Love to live on the edge and thrive on drama." },
  ESFP: { name: "The Entertainer", desc: "Spontaneous, energetic, and enthusiastic. Life is never boring around them." },
};

export function scoreMBTI(answers) {
  const counts = { E: 0, I: 0, S: 0, N: 0, T: 0, F: 0, J: 0, P: 0 };
  answers.forEach(a => counts[a]++);
  const type =
    (counts.E >= counts.I ? "E" : "I") +
    (counts.S >= counts.N ? "S" : "N") +
    (counts.T >= counts.F ? "T" : "F") +
    (counts.J >= counts.P ? "J" : "P");
  return { type, details: mbtiTypes[type], counts };
}
