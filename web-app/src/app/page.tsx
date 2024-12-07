"use client";
import Image from "next/image";
import styles from "./page.module.css";
import RaylibCanvas from "./components/RaylibCanvas";
import { useState } from "react";

export default function Home() {
  const [showGame, setShowGame] = useState(false);

  const handleButtonClick = () => {
    setShowGame(true); // Toggle the state
  };

  return (
    <div className={styles.page}>
      {!showGame && (
        <button className={styles.btnplay} onClick={handleButtonClick}>
          PlayNow
        </button>
      )}
      {showGame && <RaylibCanvas />}
    </div>
  );
}
