// server setup
const express = require("express");
const path = require("path");
const bodyParser = require("body-parser");
require("dotenv").config();

const app = express();
var options = {
  inflate: true,
  limit: "4000kb",
  type: "application/json",
};
app.use(bodyParser.raw(options));
app.use(express.static(path.join(__dirname, "build")));

const { Client } = require("pg");
const crypto = require("crypto");
const { randomgen, reset } = require("./backend/generation");
const parseInput = require("./backend/update");

var maze = [];
var discovery = [];
var timestamp;

var coordinate;
var angles;
var orientation;
var red;
var blue;
var yellow;
var datatime;

const shortestPath = require("./backend/shortestpath");

function findClosestSquare(arr, x, y) {
  let closestDistance = Number.MAX_SAFE_INTEGER;
  let closestCoordinate = null;

  for (let i = 0; i < arr.length; i++) {
    for (let j = 0; j < arr[i].length; j++) {
      if (arr[i][j] >= 0 && arr[i][j] <= 50) {
        const distance = Math.abs(i - x) + Math.abs(j - y);
        if (distance < closestDistance) {
          closestDistance = distance;
          closestCoordinate = [i, j];
        }
      }
    }
  }

  return closestCoordinate;
}

function pathing(discovery, orientation, xpos, ypos) {
  var closestSquare = findClosestSquare(discovery, xpos, ypos);
  if (closestSquare === []) {
    return;
  }
  var shortestpath = shortestPath(
    maze,
    xpos,
    ypos,
    closestSquare[0],
    closestSquare[1]
  );
  while (shortestpath === []) {
    discovery[closestSquare[0]][closestSquare[1]] = 100;
    pathing(discovery, orientation, xpos, ypos);
  }
  return shortestpath;
}

app.get("/api/pathing", function (req, res) {
  try {
    const data = JSON.parse(req.body);
    var xpos = data.xpos;
    var ypos = data.discovery;
    var orientation = data.orientation;

    const payload = pathing(xpos, ypos, orientation, discovery);

    res.status(200).json(payload);
  } catch (error) {
    const errorType = error.constructor.name;
    res.status(500).json({ error: errorType });
  }
});

app.post("/api/mockupdate", function (req, res) {
  try {
    randomgen();
    res.status(200).json({
      status: "successfully mock updated maze",
    });
  } catch (error) {
    const errorType = error.constructor.name;
    res.status(500).json({ error: errorType });
  }
});

app.post("/api/reset", function (req, res) {
  try {
    reset();
    coordinate = null;
    angles = null;
    orientation = null;
    red = null;
    blue = null;
    yellow = null;
    datatime = null;
    res.status(200).json({
      status: "successfully reset",
    });
  } catch (error) {
    const errorType = error.constructor.name;
    res.status(500).json({ error: errorType });
  }
});

app.post("/api/update", function (req, res) {
  try {
    const data = JSON.parse(req.body);
    var xpos = data.xpos;
    var ypos = data.ypos;
    var orientation = data.orientation;
    var lines = data.lines;

    parseInput(xpos, ypos, orientation, lines, maze, discovery);

    res.status(200).json({
      status: "successfully parsed data",
    });
  } catch (error) {
    const errorType = error.constructor.name;
    res.status(500).json({ error: errorType });
  }
});

app.post("/api/updatemaze", function (req, res) {
  try {
    const payload = JSON.parse(req.body);
    maze = payload.maze;
    discovery = payload.discovery;
    timestamp = payload.timestamp;

    var currentTimestamp = new Date().toISOString();
    var concatenatedString = JSON.stringify(maze) + currentTimestamp;

    var id = crypto
      .createHash("sha256")
      .update(concatenatedString)
      .digest("hex");

    // const client = new Client({
    //   connectionString: process.env.DATABASE_URL,
    //   ssl: { rejectUnauthorized: false },
    // });

    // client
    //   .connect()
    //   .then(() => {
    //     var query = `INSERT INTO mappings (id, maze, discovery, timestamp) VALUES ('${id}', '${JSON.stringify(
    //       maze
    //     )}', '${JSON.stringify(discovery)}', '${currentTimestamp}')`;
    //     return client.query(query);
    //   })
    //   .then(() => {
    //     res.status(200).json({
    //       status: "successfully updated maze and database",
    //     });
    //   })
    //   .catch((error) => {
    //     console.error("Error inserting row:", error);
    //     res.status(500).send(error.message);
    //   })
    //   .finally(() => {
    //     client.end();
    //   });

    // console.log(maze);
    // console.log(discovery);
    // console.log(timestamp);
    // console.log(id);
  } catch (error) {
    const errorType = error.constructor.name;
    res.status(500).json({ error: errorType });
  }
});

app.get("/api/displaymaze", function (req, res) {
  res.status(200).json({
    maze: maze,
    discovery: discovery,
    time: timestamp,
  });
});

app.post("/api/datahub", function (req, res) {
  try {
    const payload = JSON.parse(req.body);
    if (payload.hasOwnProperty("coordinate")) {
      coordinate = payload.coordinate;
    }
    if (payload.hasOwnProperty("angles")) {
      angles = payload.angles;
    }
    if (payload.hasOwnProperty("orientation")) {
      orientation = payload.orientation;
    }
    if (payload.hasOwnProperty("blue")) {
      blue = payload.blue;
    }
    if (payload.hasOwnProperty("yellow")) {
      yellow = payload.yellow;
    }
    if (payload.hasOwnProperty("red")) {
      red = payload.red;
    }
    datatime = new Date().toLocaleString();
    res.status(200).json({
      status: "successfully received data",
    });
  } catch (error) {
    const errorType = error.constructor.name;
    res.status(500).json({ error: errorType });
  }
});

app.get("/api/displaydata", function (req, res) {
  res.status(200).json({
    coordinate: coordinate,
    angles: angles,
    orientation: orientation,
    red: red,
    blue: blue,
    yellow: yellow,
    timestamp: datatime,
  });
});

let rawData = {
  left: 'Na',
  right: 'Na',
  front: 'Na'
};

app.get('/api/rawdata', (req, res) => {
  res.json(rawData);
});

app.put('/api/rawdata', (req, res) => {
  const updatedFields = JSON.parse(req.body);

  for (let key in updatedFields) {
    if (rawData.hasOwnProperty(key)) {
      rawData[key] = updatedFields[key];
    }
  }
  res.json({ message: 'Data updated successfully' });
});

// Server application
app.get("/*", function (req, res) {
  res.sendFile(path.join(__dirname, "build", "index.html"));
});

const port = process.env.PORT;
app.listen(port);
console.log("Server started on port " + port);
