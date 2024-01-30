import React, { useState, useEffect, useCallback } from "react";
import { DiscoveryMap } from "../components/discoverymap";
import { MazeMap } from "../components/mazemap";
import { DataHub } from "../components/datahub";

class ResetButton extends React.Component {
  handleClick = async () => {
    try {
      const response = await fetch("/api/reset", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
      });
      if (response.ok) {
        console.log("Reset successful");
      } else {
        console.error("Reset failed");
      }
    } catch (error) {
      console.error("Network error", error);
    }
  };

  render() {
    return (
      <button
        onClick={this.handleClick}
        className="max-w-xl bg-blue-500 hover:bg-blue-400 text-white font-bold py-2 px-4 border-b-4 border-blue-700 hover:border-blue-500 rounded"
      >
        Reset
      </button>
    );
  }
}

export function HomePage() {
  const [mazemap, setMaze] = useState<any>(null);
  const [discoverymap, setDiscovery] = useState<any>(null);
  const [lastTime, setLastTime] = useState(new Date());
  const [currentDateTime, setCurrentDateTime] = useState(new Date());

  const fetchData = useCallback(() => {
    fetch("/api/displaymaze")
      .then((response) => response.json())
      .then((data) => {
        const { maze, discovery, time } = data;
        if (
          JSON.stringify(maze) !== JSON.stringify(mazemap) ||
          JSON.stringify(discovery) !== JSON.stringify(discoverymap)
        ) {
          setMaze(maze);
          setDiscovery(discovery);
          setLastTime(time);
        }
      })
      .catch((error) => {
        console.log("Error fetching data:", error);
      });
  }, []);

  useEffect(() => {
    const intervalId = setInterval(() => {
      setCurrentDateTime(new Date());
      fetchData();
    }, 1000);
    return () => clearInterval(intervalId);
  }, [fetchData]);

  return (
    <>
      <div className="flex flex-row w-full">
        <div className="flex flex-col w-full text-left m-10 gap-4">
          <div className="text-4xl font-bold">Balance Bot Homepage</div>
          <h1>Current Date and Time: {currentDateTime.toLocaleString()}</h1>
          <ResetButton />
        </div>
        <div className="flex flex-col w-full m-10">
          <DataHub />
        </div>
      </div>

      {lastTime && mazemap && discoverymap ? (
        <>
          <div>Last Updated Time: {new Date(lastTime).toLocaleString()}</div>
          <div className="flex flex-row w-full">
            <div className="flex flex-col w-full items-center">
              <div className="text-lg font-bold">Maze Mapping</div>
              <MazeMap arrays={mazemap} />
            </div>
            <div className="flex flex-col w-full items-center">
              <div className="text-lg font-bold">Discovery Progress</div>
              <DiscoveryMap arrays={discoverymap} />
            </div>
          </div>
        </>
      ) : (
        <h1>No Update</h1>
      )}
    </>
  );
}
