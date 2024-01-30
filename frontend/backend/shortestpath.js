function shortestPath(maze, startX, startY, endX, endY) {
  const numRows = maze.length;
  const numCols = maze[0].length;

  // Define the possible directions: up, right, down, left
  const directions = [
    [-1, 0],
    [0, 1],
    [1, 0],
    [0, -1],
  ];

  // Create a queue to perform breadth-first search
  const queue = [[startX, startY, []]];

  // Create a visited array to keep track of visited cells
  const visited = new Array(numRows)
    .fill(0)
    .map(() => new Array(numCols).fill(false));
  visited[startX][startY] = true;

  while (queue.length > 0) {
    const [currX, currY, path] = queue.shift();

    // Check if we have reached the end point
    if (currX === endX && currY === endY) {
      return path;
    }

    // Explore all possible directions
    for (const [dx, dy] of directions) {
      const nextX = currX + dx;
      const nextY = currY + dy;

      // Check if the next position is within the maze boundaries
      if (nextX >= 0 && nextX < numRows && nextY >= 0 && nextY < numCols) {
        // Check if the next cell is a free space and not visited
        if (
          (maze[nextX][nextY] < 50 || maze[nextX][nextY] === 200) &&
          !visited[nextX][nextY]
        ) {
          // Mark the next cell as visited
          visited[nextX][nextY] = true;

          // Add the next cell's position and path to the queue
          queue.push([nextX, nextY, path.concat([[nextX, nextY]])]);
        }
      }
    }
  }

  return [];
}

module.exports = shortestPath;
