const fetch = require("node-fetch");

function randomgen() {
  let maze = [];
  for (let i = 0; i < 240; i++) {
    const row = [];
    for (let j = 0; j < 360; j++) {
      const randomValue = Math.floor(Math.random() * 100);
      row.push(randomValue);
    }
    maze.push(row);
  }

  let discovery = [];
  for (let i = 0; i < 240; i++) {
    const row = [];
    for (let j = 0; j < 360; j++) {
      const randomValue = Math.floor(Math.random() * 100);
      row.push(randomValue);
    }
    discovery.push(row);
  }

  const payload = {
    maze: maze,
    discovery: discovery,
    timestamp: new Date().toISOString(),
  };

  fetch("http://localhost:3000/api/updatemaze", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(payload),
  }).catch((error) => {
    console.error("Error:", error);
  });
}

function reset() {
  let maze = [];
  for (let i = 0; i < 240; i++) {
    const row = [];
    for (let j = 0; j < 360; j++) {
      row.push(-1);
    }
    maze.push(row);
  }

  let discovery = [];
  for (let i = 0; i < 240; i++) {
    const row = [];
    for (let j = 0; j < 360; j++) {
      row.push(0);
    }
    discovery.push(row);
  }

  const payload = {
    maze: maze,
    discovery: discovery,
    timestamp: " ",
  };

  fetch("http://localhost:3000/api/updatemaze", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(payload),
  }).catch((error) => {
    console.error("Error:", error);
  });
}

module.exports = {
  randomgen,
  reset,
};
