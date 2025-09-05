#pragma once
#include <random>
#include <vector>
#include <chrono>
#include <thread>

// Stealth utilities for educational anti-detection
class StealthUtils {
public:
    // Randomization utilities for timing variations
    static std::random_device rd;
    static std::mt19937 gen;
    
    // Human-like timing patterns
    struct TimingProfile {
        int baseDelay;
        int variance;
        double reactionSpread;
        double fatigueChance;
        
        TimingProfile(int base = 100, int var = 30, double spread = 1.2, double fatigue = 0.05)
            : baseDelay(base), variance(var), reactionSpread(spread), fatigueChance(fatigue) {}
    };
    
    // Generate realistic human delay
    static int GenerateHumanDelay(const TimingProfile& profile) {
        // Gamma distribution for reaction times
        std::gamma_distribution<double> reaction(2.0, profile.baseDelay / 2.0);
        double delay = reaction(gen) * profile.reactionSpread;
        
        // Add random variance
        std::uniform_int_distribution<> variance(-profile.variance, profile.variance);
        delay += variance(gen);
        
        // Simulate fatigue (occasional longer delays)
        if (std::uniform_real_distribution<>(0.0, 1.0)(gen) < profile.fatigueChance) {
            delay += std::uniform_int_distribution<>(200, 800)(gen);
        }
        
        // Micro-hesitations
        if (std::uniform_real_distribution<>(0.0, 1.0)(gen) < 0.15) {
            delay += std::uniform_int_distribution<>(10, 50)(gen);
        }
        
        return static_cast<int>(std::max(10.0, delay));
    }
    
    // Advanced pattern breaking
    static bool ShouldSkipShot(double skipChance = 0.02) {
        return std::uniform_real_distribution<>(0.0, 1.0)(gen) < skipChance;
    }
    
    // Simulate aim drift
    static void SimulateAimDrift(int& x, int& y, int maxDrift = 3) {
        std::normal_distribution<> drift(0.0, maxDrift / 3.0);
        x += static_cast<int>(drift(gen));
        y += static_cast<int>(drift(gen));
    }
    
    // Burst fire patterns
    static bool ShouldBurstFire(double burstChance = 0.1, int* burstCount = nullptr) {
        static int remainingBursts = 0;
        
        if (remainingBursts > 0) {
            remainingBursts--;
            if (burstCount) *burstCount = remainingBursts;
            return true;
        }
        
        if (std::uniform_real_distribution<>(0.0, 1.0)(gen) < burstChance) {
            remainingBursts = std::uniform_int_distribution<>(2, 5)(gen);
            if (burstCount) *burstCount = remainingBursts;
            return true;
        }
        
        if (burstCount) *burstCount = 0;
        return false;
    }
    
    // Natural break patterns
    static bool ShouldTakeBreak(int shotsFired, double breakBaseChance = 0.001) {
        double adjustedChance = breakBaseChance * (1.0 + shotsFired / 100.0);
        return std::uniform_real_distribution<>(0.0, 1.0)(gen) < adjustedChance;
    }
    
    // Anti-pattern detection (optimized for faster response)
    static void AntiPatternDelay() {
        // Reduced frequency and duration for better responsiveness
        if (std::uniform_real_distribution<>(0.0, 1.0)(gen) < 0.02) { // Reduced from 0.05 to 0.02
            int randomDelay = std::uniform_int_distribution<>(100, 500)(gen); // Reduced from 500-2000
            std::this_thread::sleep_for(std::chrono::milliseconds(randomDelay));
        }
    }
};

// Initialize static members
std::random_device StealthUtils::rd;
std::mt19937 StealthUtils::gen(StealthUtils::rd());
