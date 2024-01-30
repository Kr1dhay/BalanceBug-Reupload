import React, { useEffect, useRef } from "react";

interface ImageRendererProps {
  pixels: string[][];
}

const ImageRenderer: React.FC<ImageRendererProps> = ({ pixels }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext("2d");
    if (!ctx) return;

    const pixelSize = 2;

    for (let y = 0; y < pixels.length; y++) {
      for (let x = 0; x < pixels[y].length; x++) {
        const color = pixels[y][x];

        const rectX = x * pixelSize;
        const rectY = y * pixelSize;

        ctx.fillStyle = color;
        ctx.fillRect(rectX, rectY, pixelSize, pixelSize);
      }
    }
  }, [pixels]);

  return <canvas ref={canvasRef} width="720" height="480" />;
};

export default ImageRenderer;
