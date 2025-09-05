#pragma once

// Configuration constants for the triggerbot
namespace Config {
    // Default timing settings
    const int DEFAULT_DELAY = 50;              // Milliseconds between shots
    const int DEFAULT_DETECTION_RADIUS = 5;    // Pixels around crosshair to check
    const int CPU_SLEEP_DELAY = 1;             // Main loop sleep to prevent CPU overuse
    const int HOTKEY_DEBOUNCE = 200;           // Prevent hotkey spam
    
    // Default enemy color thresholds (Valorant purple: RGB 240, 90, 255)
    struct DefaultColors {
        static const int RED_MIN = 220;
        static const int RED_MAX = 255;
        static const int GREEN_MIN = 70;
        static const int GREEN_MAX = 110;
        static const int BLUE_MIN = 235;
        static const int BLUE_MAX = 255;
    };
    
    // Hotkey definitions
    const int TOGGLE_KEY = VK_F1;              // F1 to toggle on/off
    const int EXIT_KEY = VK_F2;                // F2 to exit
    const int SAMPLE_KEY = VK_SPACE;           // Space to sample color
    const int ESCAPE_KEY = VK_ESCAPE;          // Escape to exit color picker
    
    // Advanced settings
    const int MAX_DETECTION_RADIUS = 20;       // Maximum allowed detection radius
    const int MIN_DELAY = 10;                  // Minimum delay between shots
    const int MAX_DELAY = 1000;                // Maximum delay between shots
    
    // Color tolerance settings
    const int DEFAULT_COLOR_TOLERANCE = 15;    // ± RGB tolerance for color matching
    
    // Performance settings
    const bool ENABLE_MULTIPOINT_DETECTION = true;  // Check multiple points in crosshair
    const int DETECTION_POINTS = 9;            // Number of points to check (3x3 grid)
    
    // Safety features
    const bool ENABLE_RANDOM_DELAY = true;     // Add random variation to delay
    const int RANDOM_DELAY_VARIANCE = 10;      // ± milliseconds of random variation
}
