# Advanced Aimbot Integration

## 🎯 **Aimbot Features Added**

### ⚡ **Core Aimbot Functionality:**

1. **Smart Target Detection** 🔍
   - Scans FOV area around crosshair for enemies
   - Finds closest enemy pixel automatically
   - Works with existing color detection system

2. **Smooth Mouse Movement** 🖱️
   - Human-like aim trajectories
   - Configurable speed and smoothness
   - Anti-detection mouse input methods

3. **Advanced Stealth** 🥷
   - Uses SendInput for undetectable mouse movement
   - Adds human-like aim drift and imprecision
   - Respects timing profiles for natural movement

### 🎮 **Controls & Configuration:**

#### **Runtime Controls:**
- **F10**: Toggle aimbot on/off
- **Menu Option 5**: Full aimbot configuration

#### **Aimbot Settings:**
- **Speed**: 0.5-3.0x (mouse movement multiplier)
- **Smoothness**: 1-10 (higher = smoother movement)
- **FOV**: 20-200 pixels (detection radius)
- **Prediction**: ON/OFF (future target prediction)

### ⚙️ **Default Settings:**
```
Aimbot: DISABLED
Speed: 1.0x (normal)
Smoothness: 5/10 (balanced)
FOV: 50 pixels
Prediction: OFF
```

## 🔧 **How It Works:**

### **Target Acquisition:**
1. Scans FOV area around crosshair
2. Detects enemy colors using existing system
3. Calculates closest enemy pixel
4. Returns target coordinates

### **Smooth Aiming:**
1. Gets current mouse position
2. Calculates distance to target
3. Applies speed and smoothness factors
4. Moves mouse incrementally toward target
5. Adds human-like imprecision if stealth enabled

### **Anti-Detection Features:**
- **Rate Limited**: Max 60 FPS aim updates
- **Human Drift**: Random micro-adjustments
- **SendInput Method**: Undetectable mouse injection
- **Natural Movement**: Curved trajectories, not straight lines

## 🎯 **Usage Scenarios:**

### **Stealth Aimbot (Recommended):**
- Speed: 0.8-1.2x
- Smoothness: 6-8
- FOV: 30-60 pixels
- Prediction: OFF

### **Aggressive Aimbot:**
- Speed: 1.5-2.5x
- Smoothness: 3-5
- FOV: 80-120 pixels
- Prediction: ON

### **Legit Aimbot:**
- Speed: 0.5-0.8x
- Smoothness: 8-10
- FOV: 20-40 pixels
- Prediction: OFF

## 🔍 **Integration with Triggerbot:**

The aimbot works seamlessly with the triggerbot:

1. **Aimbot runs first** - Aims at closest enemy
2. **Triggerbot follows** - Shoots when enemy in crosshair
3. **Combined effect** - Automatic target acquisition + shooting

## ⚡ **Performance Optimizations:**

- **Center-first scanning** for fastest target acquisition
- **Spiral search pattern** from crosshair outward
- **Early exit** when target found
- **Frame-rate limited** to prevent excessive CPU usage

## 🛡️ **Safety Features:**

- **FOV Limitation** - Only aims within specified radius
- **Human-like Movement** - No instant snapping
- **Stealth Integration** - Respects anti-detection settings
- **Manual Override** - F10 to disable instantly

## 📋 **Debug Information:**

With debug mode (F6) enabled, you'll see:
```
Aim: (960,540) -> (980,520) dist:28.3
```

This shows:
- Current mouse position
- Target position  
- Distance to target

## 🎮 **Quick Setup:**

1. **Start the program**
2. **Choose option 5** for aimbot config
3. **Enable aimbot** (y)
4. **Set speed**: 1.0
5. **Set smoothness**: 5  
6. **Set FOV**: 50
7. **Run with option 1**
8. **Press F1** to activate
9. **Press F10** to toggle aimbot

The aimbot will automatically aim at purple enemies within your FOV while the triggerbot handles shooting!
