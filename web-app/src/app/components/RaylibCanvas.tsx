import React, { useEffect, useRef } from "react";
import { loadRaylib } from "../../utils/loadRaylib";
import styles from "../canvas.module.css";
const RaylibCanvas: React.FC = () => {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  // Resize canvas to take up all space and account for device pixel ratio
  const resizeCanvas = () => {
    if (canvasRef?.current) {
      const dpr = window.devicePixelRatio || 1; // Handle high-DPI displays
      const canvas = canvasRef.current;

      canvas.width = window.innerWidth * dpr;
      canvas.height = window.innerHeight * dpr;
      canvas.style.width = `${window.innerWidth}px`;
      canvas.style.height = `${window.innerHeight}px`;
    }
  };

  useEffect(() => {
    const initializeRaylib = async () => {
      if (!canvasRef?.current) {
        console.error("Canvas not yet initialized");
        return;
      }

      // Resize to fit the initial window size
      requestAnimationFrame(resizeCanvas);

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

  return <canvas ref={canvasRef} id="canvas" className={styles.canvas} />;
};

export default RaylibCanvas;
