export const enneagramQuestions = [
  // Type 1 - The Reformer
  { id: 1, type: 1, text: "I have a strong inner critic that notices when things are not as they should be." },
  { id: 2, type: 1, text: "I feel a persistent sense of responsibility to do things the 'right' way." },
  { id: 3, type: 1, text: "I often suppress my own desires in order to do what I believe is correct." },
  { id: 4, type: 1, text: "I get frustrated when others don't meet my standards." },

  // Type 2 - The Helper
  { id: 5, type: 2, text: "I anticipate what others need before they ask, and I feel good when I can provide it." },
  { id: 6, type: 2, text: "My sense of worth is tied to how much I help and support those around me." },
  { id: 7, type: 2, text: "I find it difficult to ask for help myself, but I give freely to others." },
  { id: 8, type: 2, text: "I naturally focus on other people's feelings and needs." },

  // Type 3 - The Achiever
  { id: 9, type: 3, text: "I am strongly motivated by success and being recognized for my accomplishments." },
  { id: 10, type: 3, text: "I adapt my image and behavior depending on who I'm with to achieve my goals." },
  { id: 11, type: 3, text: "I often measure my value by my productivity and achievements." },
  { id: 12, type: 3, text: "I hate failing and will work hard to avoid it at all costs." },

  // Type 4 - The Individualist
  { id: 13, type: 4, text: "I often feel different from others — like something is fundamentally missing in me." },
  { id: 14, type: 4, text: "I am drawn to beauty, deep emotion, and self-expression." },
  { id: 15, type: 4, text: "I can get lost in intense emotions, both positive and negative." },
  { id: 16, type: 4, text: "I long for deep connection but often feel misunderstood." },

  // Type 5 - The Investigator
  { id: 17, type: 5, text: "I prefer to observe and think things through before engaging." },
  { id: 18, type: 5, text: "I need a lot of private time to recharge and feel secure." },
  { id: 19, type: 5, text: "I tend to detach emotionally and rely heavily on my intellect." },
  { id: 20, type: 5, text: "I gather knowledge and information as a way to feel safe and prepared." },

  // Type 6 - The Loyalist
  { id: 21, type: 6, text: "I frequently anticipate worst-case scenarios and scan for potential problems." },
  { id: 22, type: 6, text: "I seek security through trusted relationships, authority, or systems." },
  { id: 23, type: 6, text: "I question my own decisions and seek reassurance from others." },
  { id: 24, type: 6, text: "Loyalty and trustworthiness are among my most important values." },

  // Type 7 - The Enthusiast
  { id: 25, type: 7, text: "I constantly seek new experiences and possibilities to keep life exciting." },
  { id: 26, type: 7, text: "I find it difficult to stay focused on difficult or painful experiences." },
  { id: 27, type: 7, text: "I am optimistic and reframe negative situations into positive ones." },
  { id: 28, type: 7, text: "I get bored easily and always want to plan the next adventure." },

  // Type 8 - The Challenger
  { id: 29, type: 8, text: "I take charge naturally and don't like feeling controlled by others." },
  { id: 30, type: 8, text: "I am comfortable with confrontation and stand my ground confidently." },
  { id: 31, type: 8, text: "I protect those I care about and fight injustice when I see it." },
  { id: 32, type: 8, text: "I can come across as intimidating, though that is rarely my intention." },

  // Type 9 - The Peacemaker
  { id: 33, type: 9, text: "I avoid conflict and try to keep the peace, even at my own expense." },
  { id: 34, type: 9, text: "I have difficulty knowing what I truly want — I tend to go along with others." },
  { id: 35, type: 9, text: "I find it easy to see all sides of an argument without taking a strong stance." },
  { id: 36, type: 9, text: "I sometimes feel invisible or overlooked, even in groups I belong to." },
];

export const enneagramTypes = {
  1: { name: "The Reformer", virtue: "Serenity", desc: "Principled, purposeful, and self-controlled. Reformers have a strong sense of right and wrong and are motivated by a desire to be good and do what is right.", wing: { a: "1w9", b: "1w2" } },
  2: { name: "The Helper", virtue: "Humility", desc: "Caring, generous, and people-pleasing. Helpers are driven by a need to be needed and find purpose in supporting and loving others.", wing: { a: "2w1", b: "2w3" } },
  3: { name: "The Achiever", virtue: "Honesty", desc: "Success-oriented, pragmatic, and adaptable. Achievers are motivated by a desire to be valuable and worthwhile and gain the admiration of others.", wing: { a: "3w2", b: "3w4" } },
  4: { name: "The Individualist", virtue: "Equanimity", desc: "Expressive, dramatic, and self-absorbed. Individualists are motivated by a need to be unique and to experience deep feelings and authentic connection.", wing: { a: "4w3", b: "4w5" } },
  5: { name: "The Investigator", virtue: "Non-Attachment", desc: "Intense, cerebral, and perceptive. Investigators are motivated by a need to possess knowledge and understand the world around them.", wing: { a: "5w4", b: "5w6" } },
  6: { name: "The Loyalist", virtue: "Courage", desc: "Committed, security-oriented, and anxious. Loyalists are motivated by a need for safety and support, and strongly value loyalty and trust.", wing: { a: "6w5", b: "6w7" } },
  7: { name: "The Enthusiast", virtue: "Sobriety", desc: "Spontaneous, versatile, and acquisitive. Enthusiasts are motivated by a need to be happy and to maintain their freedom and satisfaction.", wing: { a: "7w6", b: "7w8" } },
  8: { name: "The Challenger", virtue: "Innocence", desc: "Self-confident, decisive, and confrontational. Challengers are motivated by a need to be self-reliant and to prove their strength.", wing: { a: "8w7", b: "8w9" } },
  9: { name: "The Peacemaker", virtue: "Action", desc: "Receptive, reassuring, and complacent. Peacemakers are motivated by a need for internal and external peace, and to avoid conflict.", wing: { a: "9w8", b: "9w1" } },
};

export const enneagramScale = [
  { value: 1, label: "Not like me at all" },
  { value: 2, label: "Rarely like me" },
  { value: 3, label: "Sometimes like me" },
  { value: 4, label: "Often like me" },
  { value: 5, label: "Very much like me" },
];

export function scoreEnneagram(answers) {
  const totals = {};
  for (let t = 1; t <= 9; t++) totals[t] = 0;
  answers.forEach((score, i) => {
    totals[enneagramQuestions[i].type] += score;
  });
  const sorted = Object.entries(totals).sort((a, b) => b[1] - a[1]);
  const primaryType = parseInt(sorted[0][0]);
  const secondType = parseInt(sorted[1][0]);
  return { primaryType, totals, type: enneagramTypes[primaryType], secondType };
}
