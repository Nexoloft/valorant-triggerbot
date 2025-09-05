# Educational Triggerbot for Custom Games

This is an educational triggerbot implementation designed for learning purposes and use in custom games only.

## ⚠️ IMPORTANT DISCLAIMER
This code is for **EDUCATIONAL PURPOSES ONLY**. It is designed to help understand game automation concepts and should only be used in custom games that you own or have explicit permission to modify.

## Features

- **Color-based enemy detection**: Detects enemies based on RGB color thresholds
- **Configurable detection area**: Adjustable crosshair detection radius
- **Customizable firing delay**: Prevents spam clicking
- **Persistent configuration**: Settings are saved and loaded automatically
- **Real-time toggle**: F1 to enable/disable, F2 to exit
- **Color picker utility**: Help identify enemy colors in your game
- **Low CPU usage**: Optimized scanning loop
- **Configuration management**: Save, load, and reset settings

## How It Works

1. **Screen Capture**: Uses Windows API to capture pixel colors around the crosshair
2. **Color Analysis**: Compares pixel colors against configured enemy color thresholds
3. **Automatic Shooting**: Simulates mouse clicks when enemy colors are detected
4. **Smart Detection**: Scans multiple pixels in a radius around the crosshair for better accuracy

## Compilation

### Prerequisites
- Windows operating system
- C++ compiler with Windows SDK (MinGW, Visual Studio, etc.)
- C++17 support

### Build Commands

Using g++ (MinGW):
```bash
g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o triggerbot.exe
```

Using Visual Studio:
```bash
cl main.cpp /std:c++17 /O2 /Fe:triggerbot.exe
```

## Usage

1. **Compile the program** using one of the methods above
2. **Run the executable**: `./triggerbot.exe`
3. **Choose an option**:
   - Option 1: Run the triggerbot (loads saved config automatically)
   - Option 2: Use color picker to find enemy colors
   - Option 3: Reset configuration to defaults
   - Option 4: Exit

### Controls
- **F1**: Toggle triggerbot on/off
- **F2**: Exit the program
- **Space** (in color picker): Sample color at mouse position
- **ESC** (in color picker): Exit color picker

### Configuration

The triggerbot now automatically saves and loads configuration:

- **Automatic Loading**: Settings are loaded from `triggerbot_config.txt` on startup
- **Automatic Saving**: Configuration is saved when you modify settings
- **Manual Editing**: You can edit the config file directly with any text editor
- **Reset Option**: Use menu option 3 to reset to default settings

Configuration includes:
- **Delay**: Time between shots (milliseconds)
- **Detection Radius**: Size of detection area around crosshair
- **Crosshair Position**: Screen coordinates (auto-detected but can be manually set)
- **Enemy Colors**: RGB thresholds for enemy detection

#### Configuration File Format
```
delay=50
detectionRadius=5
crosshairX=960
crosshairY=540
enemyColorRedMin=220
enemyColorRedMax=255
enemyColorGreenMin=70
enemyColorGreenMax=110
enemyColorBlueMin=235
enemyColorBlueMax=255
```

**Default Colors**: The triggerbot now uses Valorant-style purple enemy colors by default (RGB: 240, 90, 255 with tolerance), which work well for games with similar purple enemy highlighting.

## Technical Details

### Detection Algorithm
```cpp
// Scans pixels in a radius around the crosshair
for (int x = crosshairX - radius; x <= crosshairX + radius; x++) {
    for (int y = crosshairY - radius; y <= crosshairY + radius; y++) {
        // Check if pixel matches enemy color
        if (isEnemyColor(getPixelColor(x, y))) {
            return true; // Enemy detected
        }
    }
}
```

### Color Matching
Uses RGB thresholds to identify enemy pixels:
```cpp
bool isEnemyColor(COLORREF color) {
    int r, g, b;
    extractRGB(color, r, g, b);
    
    return (r >= redMin && r <= redMax &&
            g >= greenMin && g <= greenMax &&
            b >= blueMin && b <= blueMax);
}
```

## Customization Tips

### Finding Enemy Colors
1. Use the built-in color picker (Option 2)
2. Position mouse over enemy models in your game
3. Press SPACE to sample colors
4. Note the RGB values
5. Set appropriate thresholds (usually ±10-20 from sampled values)

### Optimal Settings
- **Delay**: 50-100ms for most games
- **Radius**: 3-10 pixels depending on crosshair size
- **Color Thresholds**: Tight enough to avoid false positives, loose enough to catch variations

## Advanced Features

### Multi-Color Detection
Extend the code to detect multiple enemy color schemes:
```cpp
std::vector<ColorThreshold> enemyColorSchemes = {
    {200, 255, 0, 50, 0, 50},    // Red team
    {0, 50, 0, 50, 200, 255},    // Blue team
};
```

### Confidence Scoring
Implement confidence-based detection:
```cpp
float calculateConfidence(int matchingPixels, int totalPixels) {
    return (float)matchingPixels / totalPixels;
}
```

### Anti-Detection Measures
- Random delays between shots
- Natural mouse movement simulation
- Periodic breaks in operation

## Security Considerations

This implementation uses basic Windows API calls. For use in custom games, consider:
- Memory reading instead of screen capture for better performance
- Hardware-level input simulation
- Process isolation and sandboxing

## Legal and Ethical Use

**Remember**: This tool should only be used:
- In your own custom games
- In games where automation is explicitly allowed
- For educational and learning purposes
- With proper authorization from game developers

**Do NOT use** in:
- Competitive online games
- Games where automation violates terms of service
- Any context where it provides unfair advantage over other players

## Contributing

This is an educational project. Feel free to:
- Improve the detection algorithms
- Add new features for learning
- Optimize performance
- Add better documentation

## License

This project is for educational purposes. Use responsibly and in accordance with applicable laws and terms of service.
