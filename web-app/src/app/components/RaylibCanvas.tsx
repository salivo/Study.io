import React, { useEffect, useRef } from "react";
import { loadRaylib } from "../../utils/loadRaylib";

const RaylibCanvas: React.FC = () => {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  // Resize canvas to take up all space
  const resizeCanvas = () => {
    if (canvasRef?.current) {
      canvasRef.current.width = window.innerWidth;
      canvasRef.current.height = window.innerHeight;
    }
  };

  useEffect(() => {
    const initializeRaylib = async () => {
      if (!canvasRef?.current) {
        console.error("Canvas not yet initialized");
        return;
      }

      // Resize to fit the initial window size
      resizeCanvas();

      // Handle window resize events
      window.addEventListener("resize", resizeCanvas);

      // Dynamically set the Module's canvas reference
      (window as any).Module = {
        canvas: canvasRef.current,
      };

      await loadRaylib();

      return () => {
        // Cleanup on unmount
        window.removeEventListener("resize", resizeCanvas);
      };
    };

    initializeRaylib();
  }, []);

  return (
    <div>
      <canvas
        ref={canvasRef}
        id="canvas"
        style={{
          display: "block",
          border: "none",
          position: "absolute",
          top: 0,
          left: 0,
        }}
      />
    </div>
  );
};

export default RaylibCanvas;
