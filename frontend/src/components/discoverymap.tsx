import React from "react";
import ImageRenderer from "./imagerenderer";

interface DiscoveryMapProps {
  arrays: number[][];
}

export function DiscoveryMap({ arrays }: DiscoveryMapProps) {
  function calculateColor(num: number) {
    const startColor = [249, 205, 173];
    const endColor = [252, 67, 101];

    const r = Math.round(
      startColor[0] + (endColor[0] - startColor[0]) * (num / 100)
    );
    const g = Math.round(
      startColor[1] + (endColor[1] - startColor[1]) * (num / 100)
    );
    const b = Math.round(
      startColor[2] + (endColor[2] - startColor[2]) * (num / 100)
    );

    return `rgb(${r}, ${g}, ${b})`;
  }

  const pixels = arrays.map((arr) => arr.map((num) => calculateColor(num)));

  return (
    <div className="flex flex-col mt-5">
      <ImageRenderer pixels={pixels} />
    </div>
  );
}
