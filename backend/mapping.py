def generate_maze(width, height):
    maze_map = [[' '] * width for _ in range(height)]
    projected_discovery = [[0] * width for _ in range(height)]
    return maze_map, projected_discovery

def update_maze(maze_map, projected_discovery, update_arr):
     for update in update_arr:
        x, y = update[0], update[1]
        maze_map[x][y] = update[2]

        projected_discovery[x][y] += 1
        for i in range(max(0, x - 4), min(x + 5, len(projected_discovery))):
            for j in range(max(0, y - 4), min(y + 5, len(projected_discovery[0]))):
                distance = max(abs(i - x), abs(j - y))
                if distance <= 4:
                    if 1 <= distance <= 2:
                        projected_discovery[i][j] += 0.5
                    elif distance != 0:
                        projected_discovery[i][j] += 0.25

def parse_input(maze_map, projected_discovery, input):
    update_maze(maze_map, projected_discovery, input.get("discovery"))
    path = mapping_instruction(input.get("position").get("xpos"), input.get("position").get("ypos"), projected_discovery)
    print(path)
    # Insert update AWS method below

# Function below needs to be fixed
def mapping_instruction(x, y, projected_discovery):
    maze = [row[:] for row in projected_discovery]
    maze[x][y] = -1 

    directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]

    def is_within_boundaries(row, col):
        return 0 <= row < len(maze) and 0 <= col < len(maze[0])


    curr_row, curr_col = x, y
    path = []

    while True:
        if maze[curr_row][curr_col] < 0.5:
            return path + [(curr_row, curr_col)]

        unvisited_neighbor = False

        for direction in directions:
            new_row = curr_row + direction[0]
            new_col = curr_col + direction[1]

            if is_within_boundaries(new_row, new_col) and maze[new_row][new_col] != -1:
                unvisited_neighbor = True
                maze[new_row][new_col] = -1

                curr_row, curr_col = new_row, new_col
                path.append((curr_row, curr_col))
                break

        if not unvisited_neighbor:
            if path:
                curr_row, curr_col = path.pop()
            else:
                break
    return []

from collections import deque

def mapping_instruction(x, y, dis):
    rows = len(dis)
    cols = len(dis[0])

    # Function to check if a cell is within the grid bounds
    def is_valid_cell(row, col):
        return 0 <= row < rows and 0 <= col < cols

    # Initialize a visited array to keep track of visited cells
    visited = [[False for _ in range(cols)] for _ in range(rows)]

    # Initialize a queue for BFS
    queue = deque([(x, y)])

    # Initialize a directions array for exploring neighbors (up, down, left, right)
    directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]

    while queue:
        curr_x, curr_y = queue.popleft()

        # Check if the current cell's value is less than 0.5
        if dis[curr_x][curr_y] < 0.5:
            # Found a cell with the desired value, backtrack to find the path
            path = []
            while (curr_x, curr_y) != (x, y):
                path.append((curr_x, curr_y))
                curr_x, curr_y = visited[curr_x][curr_y]

            # Reverse the path and return it
            return list(reversed(path))

        for dx, dy in directions:
            new_x, new_y = curr_x + dx, curr_y + dy

            if is_valid_cell(new_x, new_y) and not visited[new_x][new_y]:
                visited[new_x][new_y] = (curr_x, curr_y)
                queue.append((new_x, new_y))

    # No cell with a value less than 0.5 found
    return []


example_json_object = {
    "position": {
        "xpos": 1,
        "ypos": 0
    },
    "discovery": [
        [0, 0, "W"],
        [1, 0, "S"]
    ]
}


width = 10
height = 10

maze_map, projected_discovery = generate_maze(width, height)

parse_input(maze_map, projected_discovery, example_json_object)
print(maze_map)
print(projected_discovery)
