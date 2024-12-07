let raylibScriptLoaded = false; // Flag to ensure it is loaded only once

export const loadRaylib = async (): Promise<void> => {
  if (raylibScriptLoaded) {
    console.log("Raylib script already loaded");
    return;
  }

  return new Promise((resolve) => {
    const script = document.createElement("script");
    script.src = "/game/client.js";
    script.async = true;

    script.onload = () => {
      console.log("Raylib JS loaded");
      raylibScriptLoaded = true; // Mark as loaded
      resolve();
    };

    script.onerror = () => {
      console.error("Error loading Raylib JS");
    };

    document.body.appendChild(script);
  });
};
