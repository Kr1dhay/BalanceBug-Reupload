from flask import Flask, request, jsonify
import requests
import time
import random

app = Flask(__name__)


def find_shortest_path(maze):
    start_pos = None
    end_pos = None
    for row in range(len(maze)):
        for col in range(len(maze[row])):
            if maze[row][col] == 'S':
                start_pos = (row, col)
            elif maze[row][col] == 'E':
                end_pos = (row, col)

    movements = [(-1, 0), (1, 0), (0, -1), (0, 1)]

    queue = [(start_pos, [])]

    while queue:
        current_pos, path = queue.pop(0)
        row, col = current_pos

        if current_pos == end_pos:
            return path + [current_pos]

        for movement in movements:
            new_row = row + movement[0]
            new_col = col + movement[1]

            if (
                0 <= new_row < len(maze) and
                0 <= new_col < len(maze[new_row]) and
                maze[new_row][new_col] != '#'
            ):
                queue.append(((new_row, new_col), path + [current_pos]))
                maze[new_row][new_col] = '#'

    return None

@app.route('/shortest_path', methods=['POST'])
def shortest_path():
    maze_data = request.get_json()
    path = find_shortest_path(maze_data.get("maze"))
    if path:
        return jsonify({'path': path})
    else:
        return jsonify({'error': 'No path found'})

@app.route('/test_render', methods=['POST'])
def test_render():
    try:
        # requests.post("http://localhost:3000/updatemaze", json={"maze": [["#", "#", "#", "#", "#", "#", "#"], ["#", "S", "P", "P", "P", "P", "#"], ["#", " ", "#", "#", "#", "P", "#"], ["#", " ", " ", " ", "#", "P", "#"], ["#", "#", "#", "#", "#", "P", "#"], ["#", " ", " ", " ", " ", "E", "#"], ["#", "#", "#", "#", "#", "#", "#"]], "timestamp": "2011-08-12T20:17:46.384Z"})
        # time.sleep(1)
        # requests.post("http://localhost:3000/updatemaze", json={"maze": [["#", "#", "#", "#", "#", "#", "#"], ["#", " ", "S", "P", "P", "P", "#"], ["#", " ", "#", "#", "#", "P", "#"], ["#", " ", " ", " ", "#", "P", "#"], ["#", "#", "#", "#", "#", "P", "#"], ["#", " ", " ", " ", " ", "E", "#"], ["#", "#", "#", "#", "#", "#", "#"]], "timestamp": "2011-08-12T20:17:47.384Z"})
        # time.sleep(1)
        # requests.post("http://localhost:3000/updatemaze", json={"maze": [["#", "#", "#", "#", "#", "#", "#"], ["#", " ", " ", "S", "P", "P", "#"], ["#", " ", "#", "#", "#", "P", "#"], ["#", " ", " ", " ", "#", "P", "#"], ["#", "#", "#", "#", "#", "P", "#"], ["#", " ", " ", " ", " ", "E", "#"], ["#", "#", "#", "#", "#", "#", "#"]], "timestamp": "2011-08-12T20:17:48.384Z"})
        # time.sleep(1)
        # requests.post("http://localhost:3000/updatemaze", json={"maze": [["#", "#", "#", "#", "#", "#", "#"], ["#", " ", " ", " ", "S", "P", "#"], ["#", " ", "#", "#", "#", "P", "#"], ["#", " ", " ", " ", "#", "P", "#"], ["#", "#", "#", "#", "#", "P", "#"], ["#", " ", " ", " ", " ", "E", "#"], ["#", "#", "#", "#", "#", "#", "#"]], "timestamp": "2011-08-12T20:17:49.384Z"})
        # time.sleep(1)
        for int in range (0,5):
            maze = []
            for i in range(240):
                row = []
                for j in range(360):
                    random_value = random.randint(0, 99)
                    row.append(random_value)
                maze.append(row)
            discovery = []
            for i in range(240):
                row = []
                for j in range(360):
                    random_value = random.randint(0, 99)
                    row.append(random_value)
                discovery.append(row)
            payload = {"maze": maze, "discovery": discovery, "timestamp": "2011-08-12T20:17:49.384Z"}
            requests.post("https://balance-bug.5959pn4l16bde.eu-west-2.cs.amazonlightsail.com/updatemaze", json=payload)
            time.sleep(1)
        maze = []
        for i in range(240):
            row = []
            for j in range(360):
                row.append(0)
            maze.append(row)
        discovery = []
        for i in range(240):
            row = []
            for j in range(360):
                row.append(0)
            discovery.append(row)
        payload = {"maze": maze, "discovery": discovery, "timestamp": "2011-08-12T20:17:49.384Z"}
        requests.post("http://localhost:3000/updatemaze", json=payload)
        return jsonify({'status': 'successfully posted to endpoint'})
    except Exception as e:
        return jsonify({'error': str(e)})

if __name__ == '__main__':
    app.run(debug=True)
