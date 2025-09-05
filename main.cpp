#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

class Triggerbot {
private:
    bool isActive;
    int delay;
    int screenWidth;
    int screenHeight;
    int crosshairX;
    int crosshairY;
    int detectionRadius;
    
    // Color thresholds for enemy detection
    struct ColorThreshold {
        int redMin, redMax;
        int greenMin, greenMax;
        int blueMin, blueMax;
    };
    
    ColorThreshold enemyColors;
    
public:
    Triggerbot() : isActive(false), delay(50), detectionRadius(5) {
        // Get screen dimensions
        screenWidth = GetSystemMetrics(SM_CXSCREEN);
        screenHeight = GetSystemMetrics(SM_CYSCREEN);
        
        // Set crosshair to center of screen
        crosshairX = screenWidth / 2;
        crosshairY = screenHeight / 2;
        
        // Set enemy color thresholds (Valorant purple enemy colors)
        // These values are based on Valorant's default purple enemy highlighting
        enemyColors = {220, 255, 70, 110, 235, 255}; // Purple enemies (RGB: 240, 90, 255 ± tolerance)
        
        // Try to load existing configuration
        loadConfig();
    }
    
    // Get pixel color at specific coordinates
    COLORREF getPixelColor(int x, int y) {
        HDC hdc = GetDC(NULL);
        COLORREF color = GetPixel(hdc, x, y);
        ReleaseDC(NULL, hdc);
        return color;
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
    
    // Simulate mouse click
    void shoot() {
        // Left mouse button down
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // Left mouse button up
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }
    
    // Main triggerbot loop
    void run() {
        std::cout << "Triggerbot started. Press F1 to toggle, F2 to exit.\n";
        std::cout << "Detection area: " << crosshairX << "x" << crosshairY << " (radius: " << detectionRadius << ")\n";
        
        while (true) {
            // Check for hotkeys
            if (GetAsyncKeyState(VK_F1) & 0x8000) {
                isActive = !isActive;
                std::cout << "Triggerbot " << (isActive ? "ENABLED" : "DISABLED") << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Prevent key spam
            }
            
            if (GetAsyncKeyState(VK_F2) & 0x8000) {
                std::cout << "Exiting triggerbot..." << std::endl;
                break;
            }
            
            // Main triggerbot logic
            if (isActive) {
                if (isEnemyInCrosshair()) {
                    shoot();
                    std::cout << "Shot fired!" << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Small delay to prevent excessive CPU usage
        }
    }
    
    // Configuration methods
    void setDelay(int newDelay) { delay = newDelay; }
    void setDetectionRadius(int radius) { detectionRadius = radius; }
    void setCrosshairPosition(int x, int y) { crosshairX = x; crosshairY = y; }
    
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
        
        file << "# Triggerbot Configuration File\n";
        file << "# This file is automatically generated\n";
        file << "delay=" << delay << "\n";
        file << "detectionRadius=" << detectionRadius << "\n";
        file << "crosshairX=" << crosshairX << "\n";
        file << "crosshairY=" << crosshairY << "\n";
        file << "enemyColorRedMin=" << enemyColors.redMin << "\n";
        file << "enemyColorRedMax=" << enemyColors.redMax << "\n";
        file << "enemyColorGreenMin=" << enemyColors.greenMin << "\n";
        file << "enemyColorGreenMax=" << enemyColors.greenMax << "\n";
        file << "enemyColorBlueMin=" << enemyColors.blueMin << "\n";
        file << "enemyColorBlueMax=" << enemyColors.blueMax << "\n";
        
        file.close();
        std::cout << "Configuration saved to: " << filename << std::endl;
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
            
            // Parse the value
            int intValue = std::stoi(value);
            
            // Set the appropriate configuration
            if (key == "delay") delay = intValue;
            else if (key == "detectionRadius") detectionRadius = intValue;
            else if (key == "crosshairX") crosshairX = intValue;
            else if (key == "crosshairY") crosshairY = intValue;
            else if (key == "enemyColorRedMin") enemyColors.redMin = intValue;
            else if (key == "enemyColorRedMax") enemyColors.redMax = intValue;
            else if (key == "enemyColorGreenMin") enemyColors.greenMin = intValue;
            else if (key == "enemyColorGreenMax") enemyColors.greenMax = intValue;
            else if (key == "enemyColorBlueMin") enemyColors.blueMin = intValue;
            else if (key == "enemyColorBlueMax") enemyColors.blueMax = intValue;
        }
        
        file.close();
        std::cout << "Configuration loaded from: " << filename << std::endl;
        return true;
    }
    
    // Display current settings
    void showSettings() {
        std::cout << "\n=== Triggerbot Settings ===" << std::endl;
        std::cout << "Status: " << (isActive ? "ACTIVE" : "INACTIVE") << std::endl;
        std::cout << "Delay: " << delay << "ms" << std::endl;
        std::cout << "Detection radius: " << detectionRadius << " pixels" << std::endl;
        std::cout << "Crosshair position: (" << crosshairX << ", " << crosshairY << ")" << std::endl;
        std::cout << "Enemy color RGB thresholds:" << std::endl;
        std::cout << "  Red: " << enemyColors.redMin << "-" << enemyColors.redMax << std::endl;
        std::cout << "  Green: " << enemyColors.greenMin << "-" << enemyColors.greenMax << std::endl;
        std::cout << "  Blue: " << enemyColors.blueMin << "-" << enemyColors.blueMax << std::endl;
        std::cout << "=========================" << std::endl;
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
    std::cout << "=== Educational Triggerbot for Custom Games ===" << std::endl;
    std::cout << "This is for educational purposes only!" << std::endl;
    std::cout << "\nSelect an option:" << std::endl;
    std::cout << "1. Run Triggerbot" << std::endl;
    std::cout << "2. Color Picker (to find enemy colors)" << std::endl;
    std::cout << "3. Reset Configuration to Defaults" << std::endl;
    std::cout << "4. Exit" << std::endl;
    
    int choice;
    std::cin >> choice;
    
    switch (choice) {
        case 1: {
            Triggerbot bot;
            
            // Show initial settings (loaded from config if available)
            std::cout << "\n=== Current Configuration ===" << std::endl;
            bot.showSettings();
            
            // Optional: Configure settings
            std::cout << "\nWould you like to modify settings? (y/n): ";
            char configure;
            std::cin >> configure;
            
            if (configure == 'y' || configure == 'Y') {
                int delay, radius, rMin, rMax, gMin, gMax, bMin, bMax;
                
                std::cout << "Enter delay between shots (ms): ";
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
            Triggerbot defaultBot;
            // Don't load config for this instance, keep defaults
            std::cout << "\nResetting configuration to defaults..." << std::endl;
            
            // Create a new config file with defaults
            std::ofstream file("triggerbot_config.txt");
            file << "# Triggerbot Configuration File - Reset to Defaults\n";
            file << "# Using Valorant-style purple enemy colors\n";
            file << "delay=50\n";
            file << "detectionRadius=5\n";
            file << "crosshairX=" << GetSystemMetrics(SM_CXSCREEN) / 2 << "\n";
            file << "crosshairY=" << GetSystemMetrics(SM_CYSCREEN) / 2 << "\n";
            file << "enemyColorRedMin=220\n";
            file << "enemyColorRedMax=255\n";
            file << "enemyColorGreenMin=70\n";
            file << "enemyColorGreenMax=110\n";
            file << "enemyColorBlueMin=235\n";
            file << "enemyColorBlueMax=255\n";
            file.close();
            
            std::cout << "Configuration reset to defaults and saved!" << std::endl;
            break;
        }
        case 4:
            std::cout << "Goodbye!" << std::endl;
            break;
        default:
            std::cout << "Invalid choice!" << std::endl;
            break;
    }
    
    return 0;
}
