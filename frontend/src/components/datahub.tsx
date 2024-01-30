import { useState, useEffect } from "react";

interface Data {
  coordinate?: [number, number];
  angles?: [number, number, number];
  orientation?: number;
  red?: number;
  blue?: number;
  yellow?: number;
  timestamp?: string;
}

export function DataHub() {
  const [data, setData] = useState<Data | null>(null);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await fetch("/api/displaydata");
        const jsonData = await response.json();
        setData(jsonData);
      } catch (error) {
        console.error("Error fetching data:", error);
      }
    };

    const intervalId = setInterval(fetchData, 1000);

    return () => {
      clearInterval(intervalId);
    };
  }, []);

  return (
    <>
      <div className="text-3xl font-bold text-right">Data Hub</div>
      <div className="flex flex-row mt-2 w-full justify-end">
        <div className="flex flex-col text-left w-1/4 font-semibold">
          <span>Angle:</span>
          <span>Coordinate:</span>
          <span>Orientation:</span>
          <span>Power (RBY mW):</span>
          <span>Timestamp:</span>
        </div>
        <div className="flex flex-col text-right w-1/4">
          <span>{data?.angles?.join(" / ")}</span>
          <span>{data?.coordinate?.join(" / ")}</span>
          <span>{data?.orientation}</span>
          <span>{`${data?.red} / ${data?.blue} / ${data?.yellow}`}</span>
          <span>{data?.timestamp}</span>
        </div>
      </div>
    </>
  );
}
