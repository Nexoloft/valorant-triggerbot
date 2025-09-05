# Testing Guide for Educational Triggerbot

This guide will help you test and configure the triggerbot properly for your custom game.

## Prerequisites

1. **Compiled Triggerbot**: Ensure you have successfully compiled the program
2. **Test Environment**: Your custom game or a controlled testing environment
3. **Administrator Privileges**: May be required for some input simulation

## Testing Steps

### 1. Initial Setup Test

1. Run the triggerbot executable
2. Select option 1 (Run Triggerbot)
3. Choose not to configure settings initially (press 'n')
4. Verify the program displays default settings
5. Press F2 to exit

**Expected Output:**
```
=== Triggerbot Settings ===
Status: INACTIVE
Delay: 50ms
Detection radius: 5 pixels
Crosshair position: (960, 540)  // Will vary based on screen resolution
Enemy color RGB thresholds:
  Red: 200-255
  Green: 0-50
  Blue: 0-50
=========================
```

### 2. Color Picker Test

1. Run the triggerbot
2. Select option 2 (Color Picker)
3. Move mouse to different colored areas
4. Press SPACE to sample colors
5. Note the RGB values displayed
6. Press ESC to exit

**What to Test:**
- Sample colors from your game's UI elements
- Sample colors from enemy models/indicators
- Sample colors from backgrounds to avoid false positives

### 3. Hotkey Functionality Test

1. Start the triggerbot (option 1)
2. Test F1 key:
   - Press F1 → Should toggle "ENABLED"
   - Press F1 again → Should toggle "DISABLED"
3. Test F2 key:
   - Press F2 → Should exit with "Exiting triggerbot..."

### 4. Detection Accuracy Test

#### Test Environment Setup
1. Open your custom game
2. Create a scenario with enemy targets
3. Position crosshair over enemy
4. Enable triggerbot (F1)

#### Test Cases

**Test Case 1: Direct Hit**
- Position crosshair directly on enemy
- Enable triggerbot
- Expected: Should detect and fire

**Test Case 2: Near Miss**
- Position crosshair next to enemy (but not on)
- Enable triggerbot
- Expected: Should NOT fire

**Test Case 3: Background Colors**
- Position crosshair on background elements
- Enable triggerbot
- Expected: Should NOT fire

**Test Case 4: False Positives**
- Position crosshair on UI elements with similar colors
- Enable triggerbot
- Expected: Should NOT fire (if properly configured)

### 5. Performance Test

1. Monitor CPU usage while triggerbot runs
2. Test with different detection radius settings (3, 5, 10, 15)
3. Measure response time from detection to shot

**Acceptable Performance:**
- CPU usage: < 5% on modern systems
- Response time: < 100ms
- No noticeable impact on game performance

### 6. Configuration Test

#### Test Different Delays
1. Set delay to 25ms → Test rapid firing
2. Set delay to 100ms → Test slower, more controlled firing
3. Set delay to 500ms → Test very slow firing

#### Test Different Detection Radius
1. Radius 3 → Test precise detection
2. Radius 10 → Test wider detection area
3. Radius 20 → Test maximum detection area

#### Test Color Thresholds
1. Use color picker to find exact enemy colors
2. Set tight thresholds (±5 RGB values)
3. Test detection accuracy
4. Gradually widen thresholds if needed

### 7. Edge Case Testing

#### Screen Edge Test
1. Move game window to screen edges
2. Test if crosshair position adjusts correctly
3. Ensure no out-of-bounds errors

#### Multiple Enemies Test
1. Position multiple enemies in detection area
2. Verify triggerbot doesn't malfunction
3. Check firing rate doesn't exceed configured delay

#### Color Variation Test
1. Test under different lighting conditions in game
2. Test with different enemy skins/colors
3. Adjust color thresholds as needed

## Troubleshooting Common Issues

### Issue: No Detection
**Possible Causes:**
- Incorrect color thresholds
- Detection radius too small
- Crosshair position incorrect

**Solutions:**
1. Use color picker to verify enemy colors
2. Increase detection radius
3. Check screen resolution and crosshair position

### Issue: False Positives
**Possible Causes:**
- Color thresholds too wide
- UI elements have similar colors
- Background colors match enemy colors

**Solutions:**
1. Narrow color thresholds
2. Use more specific RGB ranges
3. Test in different game areas

### Issue: Slow Response
**Possible Causes:**
- High delay setting
- Large detection radius
- System performance

**Solutions:**
1. Reduce delay setting
2. Optimize detection radius
3. Close unnecessary programs

### Issue: Not Firing
**Possible Causes:**
- Game not recognizing mouse input
- Administrator privileges needed
- Input blocking by game

**Solutions:**
1. Run as administrator
2. Check game's input method
3. Verify mouse simulation works

## Validation Checklist

Before considering the triggerbot ready for use:

- [ ] Compiles without errors
- [ ] All hotkeys work correctly
- [ ] Color picker functions properly
- [ ] Detection works with test enemies
- [ ] No false positives in normal gameplay areas
- [ ] Performance is acceptable
- [ ] Configuration saves and loads correctly
- [ ] Exits cleanly without crashes

## Advanced Testing

### Stress Testing
1. Run triggerbot for extended periods (30+ minutes)
2. Monitor for memory leaks
3. Test stability under continuous use

### Accuracy Metrics
1. Count successful detections vs. total opportunities
2. Measure false positive rate
3. Calculate average response time

### Integration Testing
1. Test with different games
2. Test with various screen resolutions
3. Test with multiple monitors

## Safety Reminders

- Always test in a controlled environment first
- Never test in competitive online games
- Respect terms of service of any games you test with
- Use only in games where you have permission
- Remember this is for educational purposes only

## Performance Optimization Tips

1. **Reduce Detection Points**: Use fewer detection points for better performance
2. **Optimize Color Matching**: Use simpler color comparison algorithms
3. **Adjust Scan Frequency**: Increase main loop delay if CPU usage is high
4. **Use Hardware Acceleration**: Consider GPU-based color detection for advanced implementations

## Documentation

Keep a testing log with:
- Date and time of tests
- Configuration used
- Results achieved
- Issues encountered
- Performance metrics

This helps track improvements and identify optimal settings for different scenarios.
