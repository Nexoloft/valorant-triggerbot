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
    Triggerbot() : isActive(false), delay(50), detectionRadius(5), shotsFired(0), stealthMode(true) {
        // Get screen dimensions
        screenWidth = GetSystemMetrics(SM_CXSCREEN);
        screenHeight = GetSystemMetrics(SM_CYSCREEN);
        
        // Set crosshair to center of screen
        crosshairX = screenWidth / 2;
        crosshairY = screenHeight / 2;
        
        // Set enemy color thresholds (Valorant purple enemy colors)
        // These values are based on Valorant's default purple enemy highlighting
        enemyColors = {220, 255, 70, 110, 235, 255}; // Purple enemies (RGB: 240, 90, 255 ± tolerance)
        
        // Initialize stealth timing profile
        humanTiming = StealthUtils::TimingProfile(80, 25, 1.3, 0.08);
        sessionStart = std::chrono::steady_clock::now();
        lastShot = sessionStart;
        
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
        
        return (r >= enemyColors.redMin && r <= enemyColors.redMax &&
                g >= enemyColors.greenMin && g <= enemyColors.greenMax &&
                b >= enemyColors.blueMin && b <= enemyColors.blueMax);
    }
    
    // Check if enemy is in crosshair area
    bool isEnemyInCrosshair() {
        // Check multiple pixels around crosshair for better detection
        for (int x = crosshairX - detectionRadius; x <= crosshairX + detectionRadius; x++) {
            for (int y = crosshairY - detectionRadius; y <= crosshairY + detectionRadius; y++) {
                COLORREF pixelColor = getPixelColor(x, y);
                if (isEnemyColor(pixelColor)) {
                    return true;
                }
            }
        }
        return false;
    }
    
    // Advanced stealth shooting with human-like patterns
    void shoot() {
        if (stealthMode) {
            // Check if we should skip this shot (human imperfection)
            if (StealthUtils::ShouldSkipShot(0.03)) {
                return;
            }
            
            // Check for burst fire pattern
            static int burstCount = 0;
            bool shouldBurst = StealthUtils::ShouldBurstFire(0.12, &burstCount);
            
            if (!shouldBurst) {
                // Use human-like delay
                int humanDelay = StealthUtils::GenerateHumanDelay(humanTiming);
                auto now = std::chrono::steady_clock::now();
                auto timeSinceLastShot = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastShot).count();
                
                if (timeSinceLastShot < humanDelay) {
                    return; // Too soon, human wouldn't react this fast
                }
                
                lastShot = now;
            }
            
            // Simulate realistic input timing
            std::this_thread::sleep_for(std::chrono::microseconds(
                std::uniform_int_distribution<>(100, 800)(StealthUtils::gen)
            ));
            
            // Use SendInput instead of mouse_event for better stealth
            INPUT input = {};
            input.type = INPUT_MOUSE;
            
            // Mouse down
            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            SendInput(1, &input, sizeof(INPUT));
            
            // Variable hold time (human-like)
            int holdTime = std::uniform_int_distribution<>(8, 25)(StealthUtils::gen);
            std::this_thread::sleep_for(std::chrono::milliseconds(holdTime));
            
            // Mouse up
            input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            SendInput(1, &input, sizeof(INPUT));
            
            shotsFired++;
            
            // Occasional anti-pattern delay
            StealthUtils::AntiPatternDelay();
            
        } else {
            // Standard shooting for non-stealth mode
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        }
    }
    
    // Main triggerbot loop with advanced stealth features
    void run() {
        std::cout << "Advanced Stealth Triggerbot started.\n";
        std::cout << "Stealth Mode: " << (stealthMode ? "ENABLED" : "DISABLED") << "\n";
        std::cout << "Controls: F1=Toggle, F2=Exit, F3=Toggle Stealth, F4=Break Simulation\n";
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
                if (isEnemyInCrosshair()) {
                    shoot();
                    if (!stealthMode) {
                        std::cout << "Shot fired!" << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                    }
                }
            }
            
            // Adaptive sleep timing
            if (stealthMode) {
                // Variable loop timing to avoid detection
                int sleepTime = std::uniform_int_distribution<>(1, 3)(StealthUtils::gen);
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
    
    // Configuration methods
    void setDelay(int newDelay) { delay = newDelay; }
    void setDetectionRadius(int radius) { detectionRadius = radius; }
    void setCrosshairPosition(int x, int y) { crosshairX = x; crosshairY = y; }
    void setStealthMode(bool enabled) { stealthMode = enabled; }
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
    std::cout << "5. Exit" << std::endl;
    
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
        case 5:
            std::cout << "Goodbye!" << std::endl;
            break;
        default:
            std::cout << "Invalid choice!" << std::endl;
            break;
    }
    
    return 0;
}
