export const discQuestions = [
  // D - Dominance
  { id: 1, text: "When faced with a challenge, I...", dimension: "D", options: [{ text: "Take charge and find a direct solution immediately", score: 4 }, { text: "Research before acting", score: 2 }, { text: "Consult the team first", score: 1 }, { text: "Follow an established process", score: 2 }] },
  { id: 2, text: "In meetings, I typically...", dimension: "D", options: [{ text: "Drive the agenda and push for decisions", score: 4 }, { text: "Energize the group and share ideas enthusiastically", score: 3 }, { text: "Support others and keep the peace", score: 1 }, { text: "Analyze the information carefully before speaking", score: 2 }] },
  { id: 3, text: "I am motivated by...", dimension: "D", options: [{ text: "Power, authority, and achieving results", score: 4 }, { text: "Recognition and social approval", score: 3 }, { text: "Security and team cooperation", score: 1 }, { text: "Accuracy and doing things correctly", score: 2 }] },
  { id: 4, text: "Under pressure, I tend to...", dimension: "D", options: [{ text: "Become demanding and aggressive", score: 4 }, { text: "Become overly emotional or impulsive", score: 3 }, { text: "Give in to avoid conflict", score: 1 }, { text: "Become perfectionistic and withdrawn", score: 2 }] },
  { id: 5, text: "My decision-making style is...", dimension: "D", options: [{ text: "Fast and decisive — I trust my gut", score: 4 }, { text: "Intuitive and based on enthusiasm", score: 3 }, { text: "Thoughtful and based on group consensus", score: 1 }, { text: "Careful and based on data and facts", score: 2 }] },

  // I - Influence
  { id: 6, text: "In social situations, I...", dimension: "I", options: [{ text: "Prefer small, focused groups", score: 1 }, { text: "Light up the room and love meeting new people", score: 4 }, { text: "Enjoy connecting with close friends", score: 3 }, { text: "Observe quietly and engage when appropriate", score: 2 }] },
  { id: 7, text: "I communicate best through...", dimension: "I", options: [{ text: "Direct, concise statements", score: 1 }, { text: "Storytelling and enthusiasm", score: 4 }, { text: "Warm, personal conversation", score: 3 }, { text: "Structured, detailed explanations", score: 2 }] },
  { id: 8, text: "I am at my best when...", dimension: "I", options: [{ text: "I have autonomy and control", score: 1 }, { text: "I can inspire and influence people", score: 4 }, { text: "I am part of a collaborative team", score: 3 }, { text: "I have clear guidelines and accurate data", score: 2 }] },
  { id: 9, text: "People describe me as...", dimension: "I", options: [{ text: "Driven, bold, and competitive", score: 1 }, { text: "Enthusiastic, optimistic, and persuasive", score: 4 }, { text: "Reliable, caring, and patient", score: 3 }, { text: "Precise, analytical, and thorough", score: 2 }] },
  { id: 10, text: "When sharing ideas, I...", dimension: "I", options: [{ text: "Get to the point quickly", score: 1 }, { text: "Paint a compelling picture and rally people", score: 4 }, { text: "Share carefully and consider everyone's input", score: 3 }, { text: "Back up ideas with evidence and logic", score: 2 }] },

  // S - Steadiness
  { id: 11, text: "I prefer a work environment that is...", dimension: "S", options: [{ text: "Fast-paced and competitive", score: 1 }, { text: "Lively and fun with lots of variety", score: 2 }, { text: "Stable, harmonious, and collaborative", score: 4 }, { text: "Structured and systematic", score: 3 }] },
  { id: 12, text: "When someone is upset, I...", dimension: "S", options: [{ text: "Address the issue directly and move on", score: 1 }, { text: "Try to cheer them up", score: 2 }, { text: "Listen patiently and offer consistent support", score: 4 }, { text: "Help them analyze the situation logically", score: 3 }] },
  { id: 13, text: "I value...", dimension: "S", options: [{ text: "Achievement and results", score: 1 }, { text: "Recognition and social connection", score: 2 }, { text: "Loyalty, teamwork, and stability", score: 4 }, { text: "Quality, accuracy, and standards", score: 3 }] },
  { id: 14, text: "Change at work makes me feel...", dimension: "S", options: [{ text: "Excited — change brings opportunity", score: 1 }, { text: "Energized — I love new possibilities", score: 2 }, { text: "Uncertain — I prefer steady consistency", score: 4 }, { text: "Cautious — I want to understand the implications", score: 3 }] },
  { id: 15, text: "My ideal pace of work is...", dimension: "S", options: [{ text: "Fast and decisive", score: 1 }, { text: "Energetic and varied", score: 2 }, { text: "Steady and methodical", score: 4 }, { text: "Careful and thorough", score: 3 }] },

  // C - Conscientiousness
  { id: 16, text: "Before starting a project, I...", dimension: "C", options: [{ text: "Dive in and figure it out", score: 1 }, { text: "Get excited and start immediately", score: 2 }, { text: "Make sure the team is aligned", score: 3 }, { text: "Gather all the facts and plan thoroughly", score: 4 }] },
  { id: 17, text: "I am bothered most by...", dimension: "C", options: [{ text: "Inefficiency and lack of control", score: 1 }, { text: "Negativity and being ignored", score: 2 }, { text: "Conflict and sudden change", score: 3 }, { text: "Inaccuracy, poor quality, and vagueness", score: 4 }] },
  { id: 18, text: "My approach to rules is...", dimension: "C", options: [{ text: "Break them if they slow results", score: 1 }, { text: "Bend them if it helps people connect", score: 2 }, { text: "Follow them to maintain stability", score: 3 }, { text: "Follow them — rules exist for good reasons", score: 4 }] },
  { id: 19, text: "I am most satisfied when...", dimension: "C", options: [{ text: "I have achieved a major goal", score: 1 }, { text: "Others are inspired by my ideas", score: 2 }, { text: "The team is working smoothly together", score: 3 }, { text: "My work is accurate and high quality", score: 4 }] },
  { id: 20, text: "My biggest fear is...", dimension: "C", options: [{ text: "Losing control or being taken advantage of", score: 1 }, { text: "Social rejection or disapproval", score: 2 }, { text: "Loss of stability or conflict", score: 3 }, { text: "Criticism of my work or being wrong", score: 4 }] },
];

export const discProfiles = {
  D: { name: "Dominant", color: "#e74c3c", desc: "Results-oriented, direct, and decisive. Natural leaders who thrive on challenges and are motivated by winning and achieving goals." },
  I: { name: "Influential", color: "#f39c12", desc: "Enthusiastic, optimistic, and collaborative. Natural communicators who inspire others and are motivated by recognition and social connections." },
  S: { name: "Steady", color: "#27ae60", desc: "Reliable, patient, and supportive. Natural team players who provide consistency and are motivated by stability and cooperation." },
  C: { name: "Conscientious", color: "#2980b9", desc: "Analytical, detail-oriented, and precise. Natural problem-solvers who value accuracy and are motivated by quality and correctness." },
};

export function scoreDISC(answers) {
  const totals = { D: 0, I: 0, S: 0, C: 0 };
  const dims = ["D", "I", "S", "C"];
  answers.forEach((score, i) => {
    const dim = discQuestions[i].dimension;
    totals[dim] += score;
  });
  const primary = Object.entries(totals).sort((a, b) => b[1] - a[1])[0][0];
  return { primary, totals, profile: discProfiles[primary] };
}
