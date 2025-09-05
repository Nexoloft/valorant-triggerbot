#pragma once
#include <windows.h>
#include <vector>
#include <random>

// Enhanced detection utilities
class DetectionUtils {
private:
    static std::random_device rd;
    static std::mt19937 gen;
    
public:
    // Color structure for easier manipulation
    struct Color {
        int r, g, b;
        Color(int red = 0, int green = 0, int blue = 0) : r(red), g(green), b(blue) {}
        Color(COLORREF colorref) {
            r = GetRValue(colorref);
            g = GetGValue(colorref);
            b = GetBValue(colorref);
        }
    };
    
    // Color threshold for enemy detection
    struct ColorThreshold {
        Color min, max;
        ColorThreshold(Color minimum, Color maximum) : min(minimum), max(maximum) {}
        ColorThreshold(int rMin, int rMax, int gMin, int gMax, int bMin, int bMax) 
            : min(rMin, gMin, bMin), max(rMax, gMax, bMax) {}
    };
    
    // Check if color falls within threshold
    static bool colorInRange(const Color& color, const ColorThreshold& threshold) {
        return (color.r >= threshold.min.r && color.r <= threshold.max.r &&
                color.g >= threshold.min.g && color.g <= threshold.max.g &&
                color.b >= threshold.min.b && color.b <= threshold.max.b);
    }
    
    // Calculate color distance (Euclidean distance in RGB space)
    static double colorDistance(const Color& c1, const Color& c2) {
        int dr = c1.r - c2.r;
        int dg = c1.g - c2.g;
        int db = c1.b - c2.b;
        return sqrt(dr*dr + dg*dg + db*db);
    }
    
    // Generate detection points in a pattern around crosshair
    static std::vector<POINT> generateDetectionPoints(int centerX, int centerY, int radius) {
        std::vector<POINT> points;
        
        // Center point
        points.push_back({centerX, centerY});
        
        // Cross pattern
        points.push_back({centerX - radius, centerY});
        points.push_back({centerX + radius, centerY});
        points.push_back({centerX, centerY - radius});
        points.push_back({centerX, centerY + radius});
        
        // Diagonal pattern
        points.push_back({centerX - radius/2, centerY - radius/2});
        points.push_back({centerX + radius/2, centerY - radius/2});
        points.push_back({centerX - radius/2, centerY + radius/2});
        points.push_back({centerX + radius/2, centerY + radius/2});
        
        return points;
    }
    
    // Advanced enemy detection with confidence scoring
    static float calculateDetectionConfidence(int centerX, int centerY, int radius, 
                                            const std::vector<ColorThreshold>& enemyColors) {
        auto points = generateDetectionPoints(centerX, centerY, radius);
        int totalPoints = points.size();
        int matchingPoints = 0;
        
        HDC hdc = GetDC(NULL);
        
        for (const auto& point : points) {
            COLORREF pixelColor = GetPixel(hdc, point.x, point.y);
            Color color(pixelColor);
            
            for (const auto& threshold : enemyColors) {
                if (colorInRange(color, threshold)) {
                    matchingPoints++;
                    break; // Found a match, no need to check other thresholds
                }
            }
        }
        
        ReleaseDC(NULL, hdc);
        
        return (float)matchingPoints / totalPoints;
    }
    
    // Generate random delay for anti-detection
    static int getRandomDelay(int baseDelay, int variance) {
        std::uniform_int_distribution<> dis(-variance, variance);
        return baseDelay + dis(gen);
    }
    
    // Smooth mouse movement (for future enhancements)
    static void smoothMouseMove(int fromX, int fromY, int toX, int toY, int steps = 10) {
        for (int i = 0; i <= steps; i++) {
            int x = fromX + (toX - fromX) * i / steps;
            int y = fromY + (toY - fromY) * i / steps;
            SetCursorPos(x, y);
            Sleep(1);
        }
    }
    
    // Get average color in a region
    static Color getAverageColor(int centerX, int centerY, int radius) {
        HDC hdc = GetDC(NULL);
        int totalR = 0, totalG = 0, totalB = 0;
        int count = 0;
        
        for (int x = centerX - radius; x <= centerX + radius; x++) {
            for (int y = centerY - radius; y <= centerY + radius; y++) {
                COLORREF color = GetPixel(hdc, x, y);
                totalR += GetRValue(color);
                totalG += GetGValue(color);
                totalB += GetBValue(color);
                count++;
            }
        }
        
        ReleaseDC(NULL, hdc);
        
        if (count > 0) {
            return Color(totalR / count, totalG / count, totalB / count);
        }
        return Color(0, 0, 0);
    }
};

// Initialize static members
std::random_device DetectionUtils::rd;
std::mt19937 DetectionUtils::gen(DetectionUtils::rd());
