#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include "stealth.h"

class Triggerbot {
private:
    bool isActive;
    int delay;
    int screenWidth;
    int screenHeight;
    int crosshairX;
    int crosshairY;
    int detectionRadius;
    int shotsFired;
    bool stealthMode;
    bool fastMode; // New fast mode option
    bool debugMode; // Debug mode for color detection
    bool testMode; // Test mode - instant shooting
    int sensitivityLevel; // 1=strict, 2=normal, 3=lenient, 4=very lenient
    
    // Aimbot features
    bool aimbotEnabled;
    float aimSpeed; // Mouse movement speed multiplier
    int aimSmoothness; // Smoothness factor (1-10)
    bool aimPrediction; // Lead target prediction
    int aimFOV; // Field of view for aimbot activation
    std::chrono::steady_clock::time_point lastAimMove;
    
    // Anti-detection features
    StealthUtils::TimingProfile humanTiming;
    std::chrono::steady_clock::time_point lastShot;
    std::chrono::steady_clock::time_point sessionStart;
    
    // Color thresholds for enemy detection
    struct ColorThreshold {
        int redMin, redMax;
        int greenMin, greenMax;
        int blueMin, blueMax;
    };
    
    ColorThreshold enemyColors;
    
public:
    Triggerbot() : isActive(false), delay(50), detectionRadius(12), shotsFired(0), stealthMode(true), fastMode(false), debugMode(false), testMode(false), sensitivityLevel(3), aimbotEnabled(false), aimSpeed(1.0f), aimSmoothness(5), aimPrediction(false), aimFOV(50) {
        // Get screen dimensions
        screenWidth = GetSystemMetrics(SM_CXSCREEN);
        screenHeight = GetSystemMetrics(SM_CYSCREEN);
        
        // Set crosshair to center of screen
        crosshairX = screenWidth / 2;
        crosshairY = screenHeight / 2;
        
        // Set enemy color thresholds (Valorant purple enemy colors) - EXPANDED RANGES
        // More lenient detection for faster response
        enemyColors = {170, 255, 30, 160, 190, 255}; // Purple: R:170-255, G:30-160, B:190-255 (broader)
        
        // Initialize stealth timing profile - Optimized for faster response
        humanTiming = StealthUtils::TimingProfile(25, 15, 1.1, 0.02); // Much faster: 25ms base, less variance
        sessionStart = std::chrono::steady_clock::now();
        lastShot = sessionStart;
        lastAimMove = sessionStart;
        
        // Try to load existing configuration
        loadConfig();
    }
    
    // Advanced stealth pixel reading
    COLORREF getPixelColor(int x, int y) {
        if (stealthMode) {
            // Add slight aim drift to simulate human imprecision
            StealthUtils::SimulateAimDrift(x, y, 1);
            
            // Use multiple methods to avoid detection patterns
            static int methodCounter = 0;
            methodCounter = (methodCounter + 1) % 3;
            
            switch (methodCounter) {
            case 0: {
                HDC hdc = GetDC(NULL);
                COLORREF color = GetPixel(hdc, x, y);
                ReleaseDC(NULL, hdc);
                return color;
            }
            case 1: {
                // BitBlt method for variety
                HDC hdcScreen = GetDC(NULL);
                HDC hdcMem = CreateCompatibleDC(hdcScreen);
                HBITMAP hbm = CreateCompatibleBitmap(hdcScreen, 1, 1);
                HGDIOBJ oldBitmap = SelectObject(hdcMem, hbm);
                
                BitBlt(hdcMem, 0, 0, 1, 1, hdcScreen, x, y, SRCCOPY);
                COLORREF color = GetPixel(hdcMem, 0, 0);
                
                SelectObject(hdcMem, oldBitmap);
                DeleteObject(hbm);
                DeleteDC(hdcMem);
                ReleaseDC(NULL, hdcScreen);
                return color;
            }
            default: {
                // Standard method
                HDC hdc = GetDC(NULL);
                COLORREF color = GetPixel(hdc, x, y);
                ReleaseDC(NULL, hdc);
                return color;
            }
            }
        } else {
            // Standard method for non-stealth mode
            HDC hdc = GetDC(NULL);
            COLORREF color = GetPixel(hdc, x, y);
            ReleaseDC(NULL, hdc);
            return color;
        }
    }
    
    // Extract RGB components from COLORREF
    void extractRGB(COLORREF color, int& r, int& g, int& b) {
        r = GetRValue(color);
        g = GetGValue(color);
        b = GetBValue(color);
    }
    
    // Check if color matches enemy color threshold
    bool isEnemyColor(COLORREF color) {
        int r, g, b;
        extractRGB(color, r, g, b);
        
        // Range-based detection (expanded ranges)
        bool rangeMatch = (r >= enemyColors.redMin && r <= enemyColors.redMax &&
                          g >= enemyColors.greenMin && g <= enemyColors.greenMax &&
                          b >= enemyColors.blueMin && b <= enemyColors.blueMax);
        
        // Adaptive tolerance based on sensitivity level
        int tolerance;
        switch (sensitivityLevel) {
            case 1: tolerance = 40; break;  // Strict
            case 2: tolerance = 60; break;  // Normal  
            case 3: tolerance = 80; break;  // Lenient (default)
            case 4: tolerance = 100; break; // Very lenient
            default: tolerance = 80; break;
        }
        
        // Multiple tolerance-based detections for different purple variations
        // Primary purple target
        bool purpleMatch1 = (abs(r - 240) <= tolerance &&
                            abs(g - 90) <= tolerance &&
                            abs(b - 255) <= tolerance);
        
        // Dark purple variant
        bool purpleMatch2 = (abs(r - 200) <= tolerance &&
                            abs(g - 60) <= tolerance &&
                            abs(b - 220) <= tolerance);
        
        // Bright purple variant
        bool purpleMatch3 = (abs(r - 255) <= tolerance &&
                            abs(g - 120) <= tolerance &&
                            abs(b - 255) <= tolerance);
        
        // Pink-purple variant (sometimes appears)
        bool purpleMatch4 = (abs(r - 220) <= tolerance &&
                            abs(g - 80) <= tolerance &&
                            abs(b - 240) <= tolerance);
        
        // For very lenient mode, add more color variations
        bool extraMatch = false;
        if (sensitivityLevel >= 3) {
            extraMatch = (abs(r - 180) <= tolerance &&
                         abs(g - 70) <= tolerance &&
                         abs(b - 200) <= tolerance);
        }
        
        return rangeMatch || purpleMatch1 || purpleMatch2 || purpleMatch3 || purpleMatch4 || extraMatch;
    }
    
    // Check if enemy is in crosshair area
    bool isEnemyInCrosshair() {
        static int colorCheckCount = 0;
        colorCheckCount++;
        
        // Check center pixel first for fastest response
        COLORREF centerColor = getPixelColor(crosshairX, crosshairY);
        if (isEnemyColor(centerColor)) {
            if (debugMode) {
                int r, g, b;
                extractRGB(centerColor, r, g, b);
                std::cout << "ENEMY DETECTED at CENTER! RGB: (" << r << "," << g << "," << b << ")" << std::endl;
            }
            return true;
        }
        
        // Check immediate surrounding pixels (3x3 area)
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue; // Already checked center
                int x = crosshairX + dx;
                int y = crosshairY + dy;
                
                COLORREF pixelColor = getPixelColor(x, y);
                if (isEnemyColor(pixelColor)) {
                    if (debugMode) {
                        int r, g, b;
                        extractRGB(pixelColor, r, g, b);
                        std::cout << "ENEMY DETECTED in 3x3! RGB: (" << r << "," << g << "," << b << ") at (" << x << "," << y << ")" << std::endl;
                    }
                    return true;
                }
            }
        }
        
        // Check full detection area (spiral pattern from center outward)
        for (int radius = 2; radius <= detectionRadius; radius++) {
            for (int x = crosshairX - radius; x <= crosshairX + radius; x++) {
                for (int y = crosshairY - radius; y <= crosshairY + radius; y++) {
                    // Skip if not on the current radius boundary
                    if (abs(x - crosshairX) != radius && abs(y - crosshairY) != radius) continue;
                    
                    COLORREF pixelColor = getPixelColor(x, y);
                    
                    if (debugMode && colorCheckCount % 1000 == 0) {
                        int r, g, b;
                        extractRGB(pixelColor, r, g, b);
                        std::cout << "Checking radius " << radius << " pixel (" << x << "," << y << ") RGB: (" << r << "," << g << "," << b << ")" << std::endl;
                    }
                    
                    if (isEnemyColor(pixelColor)) {
                        if (debugMode) {
                            int r, g, b;
                            extractRGB(pixelColor, r, g, b);
                            std::cout << "ENEMY DETECTED at radius " << radius << "! RGB: (" << r << "," << g << "," << b << ") at (" << x << "," << y << ")" << std::endl;
                        }
                        return true;
                    }
                }
            }
        }
        
        return false;
    }
    
    // Find closest enemy pixel to crosshair
    bool findClosestEnemy(int& targetX, int& targetY) {
        int closestDistance = INT_MAX;
        bool foundEnemy = false;
        int pixelsChecked = 0;
        int enemyPixelsFound = 0;
        
        // Search in detection area around crosshair (same as triggerbot detection)
        int searchRadius = detectionRadius + 5; // Slightly larger than detection radius
        
        if (debugMode) {
            std::cout << "Aimbot: Searching in radius " << searchRadius << " around (" << crosshairX << "," << crosshairY << ")" << std::endl;
        }
        
        for (int x = crosshairX - searchRadius; x <= crosshairX + searchRadius; x++) {
            for (int y = crosshairY - searchRadius; y <= crosshairY + searchRadius; y++) {
                // Check if within screen bounds
                if (x < 0 || x >= screenWidth || y < 0 || y >= screenHeight) continue;
                
                pixelsChecked++;
                COLORREF pixelColor = getPixelColor(x, y);
                if (isEnemyColor(pixelColor)) {
                    enemyPixelsFound++;
                    int distance = sqrt((x - crosshairX) * (x - crosshairX) + (y - crosshairY) * (y - crosshairY));
                    if (distance < closestDistance) {
                        closestDistance = distance;
                        targetX = x;
                        targetY = y;
                        foundEnemy = true;
                        
                        if (debugMode) {
                            int r, g, b;
                            extractRGB(pixelColor, r, g, b);
                            std::cout << "Aimbot: Found enemy color at (" << x << "," << y << ") RGB(" << r << "," << g << "," << b << ") distance: " << distance << std::endl;
                        }
                    }
                }
            }
        }
        
        if (debugMode) {
            std::cout << "Aimbot: Checked " << pixelsChecked << " pixels, found " << enemyPixelsFound << " enemy pixels";
            if (foundEnemy) {
                std::cout << ", closest at (" << targetX << "," << targetY << ") distance: " << closestDistance;
            }
            std::cout << std::endl;
        }
        
        return foundEnemy;
    }
    
    // Smooth mouse movement with human-like characteristics for FPS games
    void smoothMoveMouse(int targetX, int targetY) {
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastMove = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAimMove).count();
        
        // Reduce timing restriction for more responsive aimbot
        if (timeSinceLastMove < 5) {
            if (debugMode) std::cout << "Aimbot: Movement blocked by timing (last move " << timeSinceLastMove << "ms ago)" << std::endl;
            return;
        }
        
        // In FPS games, crosshair is always at center of screen
        // We need to calculate relative movement to aim at the target
        int centerX = crosshairX; // This should be screen center
        int centerY = crosshairY;
        
        // Calculate how far the target is from the crosshair center
        float deltaX = targetX - centerX;
        float deltaY = targetY - centerY;
        float distance = sqrt(deltaX * deltaX + deltaY * deltaY);
        
        if (debugMode) {
            std::cout << "Aimbot: Target (" << targetX << "," << targetY << ") is " 
                     << deltaX << "," << deltaY << " pixels from crosshair center (" 
                     << centerX << "," << centerY << ")" << std::endl;
            std::cout << "Aimbot: Distance from center: " << distance << " pixels" << std::endl;
        }
        
        // Don't move if target is very close to crosshair center
        if (distance < 3.0f) {
            if (debugMode) std::cout << "Aimbot: Target already at crosshair center" << std::endl;
            return;
        }
        
        // Calculate relative mouse movement needed
        // In FPS games, mouse movement translates to camera rotation
        float moveX, moveY;
        
        if (fastMode || distance < 20.0f) {
            // Fast mode or close target: move directly
            moveX = deltaX;
            moveY = deltaY;
            if (debugMode) std::cout << "Aimbot: Using direct movement to target" << std::endl;
        } else {
            // Smooth movement: move percentage of the way
            float moveRatio = 0.6f / std::max(1.0f, static_cast<float>(aimSmoothness) / 10.0f);
            moveX = deltaX * moveRatio * aimSpeed;
            moveY = deltaY * moveRatio * aimSpeed;
            if (debugMode) std::cout << "Aimbot: Using smooth movement (" << (moveRatio * 100) << "% of distance)" << std::endl;
        }
        
        // Add slight randomization in stealth mode
        if (stealthMode && distance > 5.0f) {
            moveX += (rand() % 3 - 1) * 0.5f;
            moveY += (rand() % 3 - 1) * 0.5f;
        }
        
        if (debugMode) {
            std::cout << "Aimbot: Calculated relative movement: (" << moveX << "," << moveY << ")" << std::endl;
        }
        
        // Try multiple methods for mouse movement
        if (debugMode) std::cout << "Aimbot: Attempting mouse movement..." << std::endl;
        
        // Method 1: Try SendInput with relative movement
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dx = static_cast<LONG>(moveX);
        input.mi.dy = static_cast<LONG>(moveY);
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        input.mi.mouseData = 0;
        input.mi.dwExtraInfo = 0;
        input.mi.time = 0;
        
        UINT result = SendInput(1, &input, sizeof(INPUT));
        
        if (debugMode) {
            std::cout << "Aimbot: SendInput relative movement (" << moveX << "," << moveY << ") Result: " << result << std::endl;
        }
        
        // Method 2: Also try mouse_event as backup
        mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(moveX), static_cast<DWORD>(moveY), 0, 0);
        
        if (debugMode) {
            std::cout << "Aimbot: mouse_event relative movement (" << moveX << "," << moveY << ") sent" << std::endl;
        }
        
        // Method 3: Try scaled movement if game uses different sensitivity
        if (abs(moveX) > 1 || abs(moveY) > 1) {
            // Try with scaled movement for high DPI or different game sensitivity
            float scale = 2.0f; // Try 2x scaling
            INPUT scaledInput = {};
            scaledInput.type = INPUT_MOUSE;
            scaledInput.mi.dx = static_cast<LONG>(moveX * scale);
            scaledInput.mi.dy = static_cast<LONG>(moveY * scale);
            scaledInput.mi.dwFlags = MOUSEEVENTF_MOVE;
            scaledInput.mi.mouseData = 0;
            scaledInput.mi.dwExtraInfo = 0;
            scaledInput.mi.time = 0;
            
            UINT scaledResult = SendInput(1, &scaledInput, sizeof(INPUT));
            
            if (debugMode) {
                std::cout << "Aimbot: SendInput scaled movement (" << (moveX * scale) << "," << (moveY * scale) << ") Result: " << scaledResult << std::endl;
            }
        }
        
        lastAimMove = now;
    }
    
    // Aimbot main function
    void aimbot() {
        if (!aimbotEnabled) {
            if (debugMode) {
                static int disabledCount = 0;
                disabledCount++;
                if (disabledCount % 1000 == 0) {
                    std::cout << "Aimbot called but disabled" << std::endl;
                }
            }
            return;
        }
        
        if (debugMode) {
            std::cout << "Aimbot: Searching for enemies..." << std::endl;
        }
        
        int targetX, targetY;
        if (findClosestEnemy(targetX, targetY)) {
            if (debugMode) {
                std::cout << "Aimbot: Enemy found at (" << targetX << "," << targetY << "), moving mouse..." << std::endl;
            }
            smoothMoveMouse(targetX, targetY);
        } else {
            if (debugMode) {
                static int noTargetCount = 0;
                noTargetCount++;
                if (noTargetCount % 1000 == 0) {
                    std::cout << "Aimbot: No enemies found in search area" << std::endl;
                }
            }
        }
        
        // Test mode: Force mouse movement to test if it's working at all
        if (testMode) {
            static int testCounter = 0;
            testCounter++;
            if (testCounter % 100 == 0) { // Every 100 frames
                if (debugMode) std::cout << "TEST MODE: Forcing mouse movement..." << std::endl;
                
                // Try small test movements
                INPUT testInput = {};
                testInput.type = INPUT_MOUSE;
                testInput.mi.dx = 10; // Move 10 pixels right
                testInput.mi.dy = 0;
                testInput.mi.dwFlags = MOUSEEVENTF_MOVE;
                
                UINT testResult = SendInput(1, &testInput, sizeof(INPUT));
                if (debugMode) std::cout << "TEST MODE: SendInput test movement result: " << testResult << std::endl;
                
                // Also try mouse_event
                mouse_event(MOUSEEVENTF_MOVE, 10, 0, 0, 0);
                if (debugMode) std::cout << "TEST MODE: mouse_event test movement sent" << std::endl;
            }
        }
    }
    
    // Advanced stealth shooting with human-like patterns
    void shoot() {
        if (debugMode) {
            std::cout << "SHOOT() called - stealthMode:" << stealthMode << " fastMode:" << fastMode << " testMode:" << testMode << std::endl;
        }
        
        if (testMode) {
            // Test mode - instant shooting with no delays
            if (debugMode) std::cout << "TEST MODE: Instant shot!" << std::endl;
            
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            
            shotsFired++;
            std::cout << "TEST SHOT FIRED! (Total: " << shotsFired << ")" << std::endl;
            return;
        }
        
        if (stealthMode && !fastMode) {
            // Full stealth mode with all delays
            // Check if we should skip this shot (reduced imperfection for faster response)
            if (StealthUtils::ShouldSkipShot(0.01)) { // Reduced from 0.03 to 0.01
                if (debugMode) std::cout << "Shot skipped due to human imperfection" << std::endl;
                return;
            }
            
            // Check for burst fire pattern (more aggressive)
            static int burstCount = 0;
            bool shouldBurst = StealthUtils::ShouldBurstFire(0.18, &burstCount); // Increased from 0.12
            
            if (!shouldBurst) {
                // Use human-like delay
                int humanDelay = StealthUtils::GenerateHumanDelay(humanTiming);
                auto now = std::chrono::steady_clock::now();
                auto timeSinceLastShot = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastShot).count();
                
                if (debugMode) {
                    std::cout << "Human delay: " << humanDelay << "ms, Time since last: " << timeSinceLastShot << "ms" << std::endl;
                }
                
                if (timeSinceLastShot < humanDelay) {
                    if (debugMode) std::cout << "Shot blocked by human delay timing" << std::endl;
                    return; // Too soon, human wouldn't react this fast
                }
                
                lastShot = now;
            } else {
                if (debugMode) std::cout << "Burst fire mode active" << std::endl;
            }
            
            if (debugMode) std::cout << "Executing stealth shot..." << std::endl;
            
            // Simulate realistic input timing (reduced for faster response)
            std::this_thread::sleep_for(std::chrono::microseconds(
                std::uniform_int_distribution<>(50, 200)(StealthUtils::gen) // Reduced from 100-800
            ));
            
            // Use mouse_event for maximum compatibility
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            if (debugMode) std::cout << "Mouse down sent" << std::endl;
            
            // Variable hold time (optimized for faster response)
            int holdTime = std::uniform_int_distribution<>(5, 15)(StealthUtils::gen); // Reduced from 8-25
            std::this_thread::sleep_for(std::chrono::milliseconds(holdTime));
            
            // Mouse up
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            if (debugMode) std::cout << "Mouse up sent - SHOT COMPLETE!" << std::endl;
            
            shotsFired++;
            
            // Occasional anti-pattern delay
            StealthUtils::AntiPatternDelay();
            
        } else if (stealthMode && fastMode) {
            if (debugMode) std::cout << "Executing fast mode shot..." << std::endl;
            
            // Fast stealth mode - minimal delays but use mouse_event for compatibility
            std::this_thread::sleep_for(std::chrono::microseconds(25));
            
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            
            if (debugMode) std::cout << "Fast shot complete!" << std::endl;
            
            shotsFired++;
            
        } else {
            if (debugMode) std::cout << "Executing standard shot..." << std::endl;
            
            // Standard shooting for non-stealth mode
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            
            if (debugMode) std::cout << "Standard shot complete!" << std::endl;
        }
    }
    
    // Main triggerbot loop with advanced stealth features
    void run() {
        std::cout << "Advanced Stealth Triggerbot started.\n";
        std::cout << "Stealth Mode: " << (stealthMode ? "ENABLED" : "DISABLED") << "\n";
        std::cout << "Controls: F1=Toggle, F2=Exit, F3=Toggle Stealth, F4=Break Simulation, F5=Fast Mode, F6=Debug, F7=Color Sample, F8=Test Mode, F9=Sensitivity, F10=Aimbot\n";
        std::cout << "Detection area: " << crosshairX << "x" << crosshairY << " (radius: " << detectionRadius << ")\n";
        
        bool breakMode = false;
        auto lastBreakCheck = std::chrono::steady_clock::now();
        
        while (true) {
            // Check for hotkeys
            if (GetAsyncKeyState(VK_F1) & 0x8000) {
                isActive = !isActive;
                std::cout << "Triggerbot " << (isActive ? "ENABLED" : "DISABLED") << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
            if (GetAsyncKeyState(VK_F2) & 0x8000) {
                std::cout << "Exiting triggerbot..." << std::endl;
                std::cout << "Session stats: " << shotsFired << " shots fired" << std::endl;
                break;
            }
            
            if (GetAsyncKeyState(VK_F3) & 0x8000) {
                stealthMode = !stealthMode;
                std::cout << "Stealth Mode " << (stealthMode ? "ENABLED" : "DISABLED") << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
            if (GetAsyncKeyState(VK_F4) & 0x8000) {
                breakMode = !breakMode;
                std::cout << "Break Simulation " << (breakMode ? "ENABLED" : "DISABLED") << std::endl;
                if (breakMode) {
                    std::cout << "Taking a human-like break..." << std::endl;
                    int breakTime = std::uniform_int_distribution<>(3000, 15000)(StealthUtils::gen);
                    std::this_thread::sleep_for(std::chrono::milliseconds(breakTime));
                    breakMode = false;
                    std::cout << "Break finished. Resuming..." << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
            if (GetAsyncKeyState(VK_F5) & 0x8000) {
                fastMode = !fastMode;
                std::cout << "Fast Mode " << (fastMode ? "ENABLED" : "DISABLED") << std::endl;
                if (fastMode) {
                    std::cout << "Warning: Fast mode reduces stealth effectiveness for maximum speed!" << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
            if (GetAsyncKeyState(VK_F6) & 0x8000) {
                debugMode = !debugMode;
                std::cout << "Debug Mode " << (debugMode ? "ENABLED" : "DISABLED") << std::endl;
                if (debugMode) {
                    std::cout << "Debug mode will show color detection information..." << std::endl;
                    std::cout << "Looking for RGB ranges: R:" << enemyColors.redMin << "-" << enemyColors.redMax 
                             << " G:" << enemyColors.greenMin << "-" << enemyColors.greenMax 
                             << " B:" << enemyColors.blueMin << "-" << enemyColors.blueMax << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
            if (GetAsyncKeyState(VK_F7) & 0x8000) {
                // Color sampling mode - show what color is at crosshair
                COLORREF centerColor = getPixelColor(crosshairX, crosshairY);
                int r, g, b;
                extractRGB(centerColor, r, g, b);
                std::cout << "CENTER PIXEL RGB: (" << r << "," << g << "," << b << ")";
                if (isEnemyColor(centerColor)) {
                    std::cout << " - ENEMY COLOR DETECTED!";
                }
                std::cout << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            
            if (GetAsyncKeyState(VK_F8) & 0x8000) {
                testMode = !testMode;
                std::cout << "Test Mode " << (testMode ? "ENABLED" : "DISABLED") << std::endl;
                if (testMode) {
                    std::cout << "WARNING: Test mode uses instant shooting with no delays!" << std::endl;
                    std::cout << "This will bypass ALL stealth features for debugging." << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
            if (GetAsyncKeyState(VK_F9) & 0x8000) {
                sensitivityLevel = (sensitivityLevel % 4) + 1; // Cycle 1->2->3->4->1
                std::string sensitivity;
                switch (sensitivityLevel) {
                    case 1: sensitivity = "STRICT"; break;
                    case 2: sensitivity = "NORMAL"; break;
                    case 3: sensitivity = "LENIENT"; break;
                    case 4: sensitivity = "VERY LENIENT"; break;
                }
                std::cout << "Detection Sensitivity: " << sensitivity << " (Level " << sensitivityLevel << ")" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
            if (GetAsyncKeyState(VK_F10) & 0x8000) {
                aimbotEnabled = !aimbotEnabled;
                std::cout << "Aimbot " << (aimbotEnabled ? "ENABLED" : "DISABLED") << std::endl;
                if (aimbotEnabled) {
                    std::cout << "Aimbot settings: Speed=" << aimSpeed << " Smoothness=" << aimSmoothness 
                             << " FOV=" << aimFOV << " Prediction=" << (aimPrediction ? "ON" : "OFF") << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
            // Automatic break simulation
            auto now = std::chrono::steady_clock::now();
            if (stealthMode && std::chrono::duration_cast<std::chrono::seconds>(now - lastBreakCheck).count() > 30) {
                if (StealthUtils::ShouldTakeBreak(shotsFired, 0.002)) {
                    std::cout << "Taking automatic human-like break..." << std::endl;
                    int breakTime = std::uniform_int_distribution<>(2000, 8000)(StealthUtils::gen);
                    std::this_thread::sleep_for(std::chrono::milliseconds(breakTime));
                    std::cout << "Break finished." << std::endl;
                    shotsFired = 0; // Reset counter after break
                }
                lastBreakCheck = now;
            }
            
            // Main triggerbot logic
            if (isActive && !breakMode) {
                // Run aimbot first
                if (aimbotEnabled) {
                    aimbot();
                    // Delay after aimbot movement to let mouse position settle
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                
                if (isEnemyInCrosshair()) {
                    if (debugMode) std::cout << "Enemy in crosshair - calling shoot()!" << std::endl;
                    shoot();
                    if (!stealthMode) {
                        std::cout << "Shot fired!" << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                    }
                } else if (debugMode) {
                    static int noEnemyCount = 0;
                    noEnemyCount++;
                    if (noEnemyCount % 2000 == 0) {
                        std::cout << "No enemy detected in crosshair area" << std::endl;
                    }
                }
            } else if (debugMode) {
                static int inactiveCount = 0;
                inactiveCount++;
                if (inactiveCount % 2000 == 0) {
                    std::cout << "Triggerbot inactive - isActive:" << isActive << " breakMode:" << breakMode << std::endl;
                }
            }
            
            // Adaptive sleep timing (optimized for responsiveness)
            if (fastMode) {
                // Ultra-fast mode - minimal sleep
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            } else if (stealthMode) {
                // Reduced loop timing for faster detection
                std::this_thread::sleep_for(std::chrono::microseconds(500)); // Much faster than 1-3ms
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds(200)); // Even faster for non-stealth
            }
        }
    }
    
    // Configuration methods
    void setDelay(int newDelay) { delay = newDelay; }
    void setDetectionRadius(int radius) { detectionRadius = radius; }
    void setCrosshairPosition(int x, int y) { crosshairX = x; crosshairY = y; }
    void setStealthMode(bool enabled) { stealthMode = enabled; }
    void setFastMode(bool enabled) { fastMode = enabled; }
    void setDebugMode(bool enabled) { debugMode = enabled; }
    void setTestMode(bool enabled) { testMode = enabled; }
    void setAimbotEnabled(bool enabled) { aimbotEnabled = enabled; }
    void setAimSpeed(float speed) { aimSpeed = speed; }
    void setAimSmoothness(int smoothness) { aimSmoothness = smoothness; }
    void setAimFOV(int fov) { aimFOV = fov; }
    void setAimPrediction(bool enabled) { aimPrediction = enabled; }
    void setHumanTiming(int base, int variance, double spread, double fatigue) {
        humanTiming = StealthUtils::TimingProfile(base, variance, spread, fatigue);
    }
    
    void setEnemyColorThreshold(int rMin, int rMax, int gMin, int gMax, int bMin, int bMax) {
        enemyColors = {rMin, rMax, gMin, gMax, bMin, bMax};
    }
    
    // Save configuration to file
    bool saveConfig(const std::string& filename = "triggerbot_config.txt") {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "Error: Cannot create config file: " << filename << std::endl;
            return false;
        }
        
        file << "# Advanced Stealth Triggerbot Configuration File\n";
        file << "# This file is automatically generated\n";
        file << "delay=" << delay << "\n";
        file << "detectionRadius=" << detectionRadius << "\n";
        file << "crosshairX=" << crosshairX << "\n";
        file << "crosshairY=" << crosshairY << "\n";
        file << "stealthMode=" << (stealthMode ? 1 : 0) << "\n";
        file << "humanTimingBase=" << humanTiming.baseDelay << "\n";
        file << "humanTimingVariance=" << humanTiming.variance << "\n";
        file << "humanTimingSpread=" << humanTiming.reactionSpread << "\n";
        file << "humanTimingFatigue=" << humanTiming.fatigueChance << "\n";
        file << "enemyColorRedMin=" << enemyColors.redMin << "\n";
        file << "enemyColorRedMax=" << enemyColors.redMax << "\n";
        file << "enemyColorGreenMin=" << enemyColors.greenMin << "\n";
        file << "enemyColorGreenMax=" << enemyColors.greenMax << "\n";
        file << "enemyColorBlueMin=" << enemyColors.blueMin << "\n";
        file << "enemyColorBlueMax=" << enemyColors.blueMax << "\n";
        
        file.close();
        std::cout << "Advanced configuration saved to: " << filename << std::endl;
        return true;
    }
    
    // Load configuration from file
    bool loadConfig(const std::string& filename = "triggerbot_config.txt") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "No existing config file found. Using defaults." << std::endl;
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            // Skip comments and empty lines
            if (line.empty() || line[0] == '#') continue;
            
            // Find the equals sign
            size_t equalPos = line.find('=');
            if (equalPos == std::string::npos) continue;
            
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);
            
            // Parse different value types
            if (key == "delay") delay = std::stoi(value);
            else if (key == "detectionRadius") detectionRadius = std::stoi(value);
            else if (key == "crosshairX") crosshairX = std::stoi(value);
            else if (key == "crosshairY") crosshairY = std::stoi(value);
            else if (key == "stealthMode") stealthMode = (std::stoi(value) == 1);
            else if (key == "humanTimingBase") humanTiming.baseDelay = std::stoi(value);
            else if (key == "humanTimingVariance") humanTiming.variance = std::stoi(value);
            else if (key == "humanTimingSpread") humanTiming.reactionSpread = std::stod(value);
            else if (key == "humanTimingFatigue") humanTiming.fatigueChance = std::stod(value);
            else if (key == "enemyColorRedMin") enemyColors.redMin = std::stoi(value);
            else if (key == "enemyColorRedMax") enemyColors.redMax = std::stoi(value);
            else if (key == "enemyColorGreenMin") enemyColors.greenMin = std::stoi(value);
            else if (key == "enemyColorGreenMax") enemyColors.greenMax = std::stoi(value);
            else if (key == "enemyColorBlueMin") enemyColors.blueMin = std::stoi(value);
            else if (key == "enemyColorBlueMax") enemyColors.blueMax = std::stoi(value);
        }
        
        file.close();
        std::cout << "Advanced configuration loaded from: " << filename << std::endl;
        return true;
    }
    
    // Display current settings
    void showSettings() {
        std::cout << "\n=== Advanced Stealth Triggerbot Settings ===" << std::endl;
        std::cout << "Status: " << (isActive ? "ACTIVE" : "INACTIVE") << std::endl;
        std::cout << "Stealth Mode: " << (stealthMode ? "ENABLED" : "DISABLED") << std::endl;
        std::cout << "Fast Mode: " << (fastMode ? "ENABLED" : "DISABLED") << std::endl;
        std::cout << "Debug Mode: " << (debugMode ? "ENABLED" : "DISABLED") << std::endl;
        std::cout << "Test Mode: " << (testMode ? "ENABLED" : "DISABLED") << std::endl;
        
        std::string sensitivity;
        switch (sensitivityLevel) {
            case 1: sensitivity = "STRICT"; break;
            case 2: sensitivity = "NORMAL"; break;
            case 3: sensitivity = "LENIENT"; break;
            case 4: sensitivity = "VERY LENIENT"; break;
        }
        std::cout << "Detection Sensitivity: " << sensitivity << " (Level " << sensitivityLevel << ")" << std::endl;
        std::cout << "Aimbot: " << (aimbotEnabled ? "ENABLED" : "DISABLED") << std::endl;
        if (aimbotEnabled) {
            std::cout << "  Speed: " << aimSpeed << "x" << std::endl;
            std::cout << "  Smoothness: " << aimSmoothness << "/10" << std::endl;
            std::cout << "  FOV: " << aimFOV << " pixels" << std::endl;
            std::cout << "  Prediction: " << (aimPrediction ? "ON" : "OFF") << std::endl;
        }
        std::cout << "Base Delay: " << delay << "ms (when stealth disabled)" << std::endl;
        std::cout << "Detection radius: " << detectionRadius << " pixels" << std::endl;
        std::cout << "Crosshair position: (" << crosshairX << ", " << crosshairY << ")" << std::endl;
        std::cout << "Shots fired this session: " << shotsFired << std::endl;
        std::cout << "\nHuman Timing Profile:" << std::endl;
        std::cout << "  Base delay: " << humanTiming.baseDelay << "ms" << std::endl;
        std::cout << "  Variance: ±" << humanTiming.variance << "ms" << std::endl;
        std::cout << "  Reaction spread: " << humanTiming.reactionSpread << "x" << std::endl;
        std::cout << "  Fatigue chance: " << (humanTiming.fatigueChance * 100) << "%" << std::endl;
        std::cout << "\nEnemy color RGB thresholds:" << std::endl;
        std::cout << "  Red: " << enemyColors.redMin << "-" << enemyColors.redMax << std::endl;
        std::cout << "  Green: " << enemyColors.greenMin << "-" << enemyColors.greenMax << std::endl;
        std::cout << "  Blue: " << enemyColors.blueMin << "-" << enemyColors.blueMax << std::endl;
        std::cout << "=========================================" << std::endl;
    }
};

// Color picker utility
void colorPicker() {
    std::cout << "\n=== Color Picker Mode ===" << std::endl;
    std::cout << "Move your mouse to a pixel and press SPACE to get its color." << std::endl;
    std::cout << "Press ESC to exit color picker." << std::endl;
    
    while (true) {
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            POINT cursorPos;
            GetCursorPos(&cursorPos);
            
            HDC hdc = GetDC(NULL);
            COLORREF color = GetPixel(hdc, cursorPos.x, cursorPos.y);
            ReleaseDC(NULL, hdc);
            
            int r = GetRValue(color);
            int g = GetGValue(color);
            int b = GetBValue(color);
            
            std::cout << "Position: (" << cursorPos.x << ", " << cursorPos.y << ") ";
            std::cout << "Color: RGB(" << r << ", " << g << ", " << b << ")" << std::endl;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    std::cout << "=== Advanced Stealth Triggerbot for Educational Purposes ===" << std::endl;
    std::cout << "This version includes sophisticated anti-detection techniques!" << std::endl;
    std::cout << "\nSelect an option:" << std::endl;
    std::cout << "1. Run Triggerbot (with stealth features)" << std::endl;
    std::cout << "2. Color Picker (to find enemy colors)" << std::endl;
    std::cout << "3. Reset Configuration to Defaults" << std::endl;
    std::cout << "4. Advanced Stealth Configuration" << std::endl;
    std::cout << "5. Aimbot Configuration" << std::endl;
    std::cout << "6. Exit" << std::endl;
    
    int choice;
    std::cin >> choice;
    
    switch (choice) {
        case 1: {
            Triggerbot bot;
            
            // Show initial settings (loaded from config if available)
            std::cout << "\n=== Current Configuration ===" << std::endl;
            bot.showSettings();
            
            // Optional: Configure basic settings
            std::cout << "\nWould you like to modify basic settings? (y/n): ";
            char configure;
            std::cin >> configure;
            
            if (configure == 'y' || configure == 'Y') {
                int delay, radius, rMin, rMax, gMin, gMax, bMin, bMax;
                
                std::cout << "Enter delay between shots (ms, for non-stealth mode): ";
                std::cin >> delay;
                bot.setDelay(delay);
                
                std::cout << "Enter detection radius (pixels): ";
                std::cin >> radius;
                bot.setDetectionRadius(radius);
                
                std::cout << "Enter enemy color thresholds (RGB):" << std::endl;
                std::cout << "Red min: "; std::cin >> rMin;
                std::cout << "Red max: "; std::cin >> rMax;
                std::cout << "Green min: "; std::cin >> gMin;
                std::cout << "Green max: "; std::cin >> gMax;
                std::cout << "Blue min: "; std::cin >> bMin;
                std::cout << "Blue max: "; std::cin >> bMax;
                
                bot.setEnemyColorThreshold(rMin, rMax, gMin, gMax, bMin, bMax);
                
                // Save the new configuration
                bot.saveConfig();
                
                std::cout << "\nUpdated settings:" << std::endl;
                bot.showSettings();
            }
            
            // Start the triggerbot
            bot.run();
            break;
        }
        case 2:
            colorPicker();
            break;
        case 3: {
            // Reset configuration to defaults
            std::cout << "\nResetting configuration to advanced defaults..." << std::endl;
            
            std::ofstream file("triggerbot_config.txt");
            file << "# Advanced Stealth Triggerbot Configuration - Reset to Defaults\n";
            file << "# Using sophisticated anti-detection settings\n";
            file << "delay=50\n";
            file << "detectionRadius=5\n";
            file << "crosshairX=" << GetSystemMetrics(SM_CXSCREEN) / 2 << "\n";
            file << "crosshairY=" << GetSystemMetrics(SM_CYSCREEN) / 2 << "\n";
            file << "stealthMode=1\n";
            file << "humanTimingBase=80\n";
            file << "humanTimingVariance=25\n";
            file << "humanTimingSpread=1.3\n";
            file << "humanTimingFatigue=0.08\n";
            file << "enemyColorRedMin=220\n";
            file << "enemyColorRedMax=255\n";
            file << "enemyColorGreenMin=70\n";
            file << "enemyColorGreenMax=110\n";
            file << "enemyColorBlueMin=235\n";
            file << "enemyColorBlueMax=255\n";
            file.close();
            
            std::cout << "Advanced configuration reset to defaults and saved!" << std::endl;
            break;
        }
        case 4: {
            // Advanced stealth configuration
            std::cout << "\n=== Advanced Stealth Configuration ===" << std::endl;
            
            Triggerbot bot;
            bot.showSettings();
            
            std::cout << "\nConfigure human-like timing:" << std::endl;
            int baseDelay, variance;
            double spread, fatigue;
            
            std::cout << "Base reaction time (ms) [50-150]: ";
            std::cin >> baseDelay;
            std::cout << "Timing variance (±ms) [10-50]: ";
            std::cin >> variance;
            std::cout << "Reaction spread multiplier [1.0-2.0]: ";
            std::cin >> spread;
            std::cout << "Fatigue chance [0.01-0.2]: ";
            std::cin >> fatigue;
            
            bot.setHumanTiming(baseDelay, variance, spread, fatigue);
            
            char enableStealth;
            std::cout << "Enable stealth mode? (y/n): ";
            std::cin >> enableStealth;
            bot.setStealthMode(enableStealth == 'y' || enableStealth == 'Y');
            
            bot.saveConfig();
            
            std::cout << "\nAdvanced stealth configuration saved!" << std::endl;
            bot.showSettings();
            break;
        }
        case 5: {
            // Aimbot configuration
            Triggerbot bot;
            
            std::cout << "\n=== Aimbot Configuration ===" << std::endl;
            bot.showSettings();
            
            char enableAimbot;
            std::cout << "\nEnable aimbot? (y/n): ";
            std::cin >> enableAimbot;
            bot.setAimbotEnabled(enableAimbot == 'y' || enableAimbot == 'Y');
            
            if (enableAimbot == 'y' || enableAimbot == 'Y') {
                float speed;
                int smoothness, fov;
                char prediction;
                
                std::cout << "Aim speed multiplier [0.5-3.0]: ";
                std::cin >> speed;
                bot.setAimSpeed(speed);
                
                std::cout << "Aim smoothness [1-10] (higher = smoother): ";
                std::cin >> smoothness;
                bot.setAimSmoothness(smoothness);
                
                std::cout << "Aimbot FOV radius [20-200] (pixels): ";
                std::cin >> fov;
                bot.setAimFOV(fov);
                
                std::cout << "Enable target prediction? (y/n): ";
                std::cin >> prediction;
                bot.setAimPrediction(prediction == 'y' || prediction == 'Y');
            }
            
            bot.saveConfig();
            
            std::cout << "\nAimbot configuration saved!" << std::endl;
            bot.showSettings();
            break;
        }
        case 6:
            std::cout << "Goodbye!" << std::endl;
            break;
        default:
            std::cout << "Invalid choice!" << std::endl;
            break;
    }
    
    return 0;
}
