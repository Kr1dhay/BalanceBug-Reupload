import React from "react";
import ImageRenderer from "./imagerenderer";

interface MazeMapProps {
  arrays: number[][];
}

export function MazeMap({ arrays }: MazeMapProps) {
  function calculateColor(num: number) {
    if (num === -1) {
      return "#69D2E7";
    } else if (num === 200) {
      return "#F38630";
    } else if (num === 300) {
      return "#C02942";
    } else {
      const min = 0;
      const max = 100;

      const normalizedNum = (num - min) / (max - min);
      const shade = Math.round(255 - normalizedNum * 255);
      const color = shade.toString(16).padStart(2, "0");

      return `#${color}${color}${color}`;
    }
  }

  const pixels = arrays.map((arr) => arr.map((num) => calculateColor(num)));

  return (
    <div className="flex flex-col mt-5">
      <ImageRenderer pixels={pixels} />
    </div>
  );
}
