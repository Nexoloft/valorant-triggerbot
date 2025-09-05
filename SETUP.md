# Development Environment Setup Guide

This guide will help you set up a C++ development environment to compile and run the educational triggerbot.

## Option 1: MinGW-w64 (Recommended for Beginners)

### Step 1: Download and Install MSYS2
1. Go to [https://www.msys2.org/](https://www.msys2.org/)
2. Download the installer
3. Run the installer and follow the setup wizard
4. After installation, open the MSYS2 terminal

### Step 2: Install MinGW-w64 Toolchain
In the MSYS2 terminal, run:
```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-make
```

### Step 3: Add to System PATH
1. Open Windows Environment Variables:
   - Press `Win + R`, type `sysdm.cpl`, press Enter
   - Click "Environment Variables"
   - Under "System Variables", find and select "Path"
   - Click "Edit" then "New"
   - Add: `C:\msys64\mingw64\bin`
2. Click OK to save changes
3. Restart Command Prompt/PowerShell

### Step 4: Verify Installation
Open a new Command Prompt and run:
```cmd
g++ --version
```
You should see version information.

## Option 2: Visual Studio Community (Full IDE)

### Step 1: Download Visual Studio
1. Go to [https://visualstudio.microsoft.com/vs/community/](https://visualstudio.microsoft.com/vs/community/)
2. Download Visual Studio Community (free)

### Step 2: Install with C++ Workload
1. Run the installer
2. Select "Desktop development with C++" workload
3. Make sure these components are checked:
   - MSVC v143 compiler toolset
   - Windows 10/11 SDK (latest version)
   - CMake tools for Visual Studio
4. Click Install

### Step 3: Setup Developer Command Prompt
After installation, you can use:
- "Developer Command Prompt for VS 2022" from Start Menu
- Or open regular Command Prompt and run: `"%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`

## Option 3: Build Tools Only (Lightweight)

### Step 1: Download Build Tools
1. Go to [https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
2. Download "Build Tools for Visual Studio 2022"

### Step 2: Install C++ Build Tools
1. Run the installer
2. Select "C++ build tools"
3. Make sure these are selected:
   - MSVC v143 - VS 2022 C++ x64/x86 build tools
   - Windows 10/11 SDK
4. Install

### Step 3: Use Developer Command Prompt
Use "Developer Command Prompt" from Start Menu or initialize the environment manually.

## Option 4: Using WSL (Windows Subsystem for Linux)

### Step 1: Install WSL
1. Open PowerShell as Administrator
2. Run: `wsl --install`
3. Restart computer when prompted

### Step 2: Install Ubuntu and Development Tools
```bash
sudo apt update
sudo apt install build-essential
sudo apt install g++ make
```

**Note:** WSL version won't have access to Windows APIs needed for the triggerbot, so this is mainly for learning C++ compilation.

## Testing Your Setup

After installing any of the above options:

### Test 1: Compiler Check
```cmd
g++ --version
```
or
```cmd
cl
```

### Test 2: Compile the Triggerbot
Navigate to the project directory and run:
```cmd
build-alt.bat
```

### Test 3: Manual Compilation
Try manual compilation:

**With MinGW:**
```cmd
g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o bin/triggerbot.exe -lgdi32 -luser32
```

**With Visual Studio:**
```cmd
cl main.cpp /std:c++17 /O2 /Fe:bin/triggerbot.exe user32.lib gdi32.lib
```

## Using VS Code

### Step 1: Install VS Code
1. Download from [https://code.visualstudio.com/](https://code.visualstudio.com/)
2. Install the C/C++ extension by Microsoft

### Step 2: Open the Project
1. Open VS Code
2. File → Open Folder → Select the triggerbot project folder
3. VS Code should automatically detect the C++ configuration

### Step 3: Build and Run
- Use `Ctrl+Shift+P` and type "Tasks: Run Task"
- Select "Build Triggerbot"
- Or use the terminal: `Ctrl+`` (backtick) and run build commands

## Troubleshooting

### Issue: "g++ is not recognized"
**Solution:** Add compiler to PATH or use Developer Command Prompt

### Issue: "windows.h not found"
**Solutions:**
1. Install Windows SDK
2. Use Developer Command Prompt
3. Check compiler installation

### Issue: "Cannot find lgdi32 or luser32"
**Solution:** Use correct linking flags for your compiler:
- MinGW: `-lgdi32 -luser32`
- Visual Studio: `user32.lib gdi32.lib`

### Issue: VS Code IntelliSense errors
**Solutions:**
1. Install C/C++ extension
2. Update `.vscode/c_cpp_properties.json` with correct paths
3. Use `Ctrl+Shift+P` → "C/C++: Edit Configurations (UI)"

### Issue: Build works but executable won't run
**Solutions:**
1. Check if all DLLs are available
2. Run from Developer Command Prompt
3. Install Visual C++ Redistributable

## Alternative Online Compilers

For quick testing without local setup:
1. [Compiler Explorer (Godbolt)](https://godbolt.org/)
2. [OnlineGDB](https://www.onlinegdb.com/)
3. [Replit](https://replit.com/)

**Note:** Online compilers won't support Windows-specific APIs like `windows.h`, so they're only good for syntax checking.

## Recommended Setup for This Project

For the triggerbot project specifically:

1. **Beginners:** MinGW-w64 via MSYS2
2. **Existing VS users:** Visual Studio Community
3. **Lightweight:** Build Tools for Visual Studio
4. **Advanced users:** Any of the above + VS Code

## Next Steps

After setting up your environment:
1. Compile the triggerbot successfully
2. Read the `README.md` for usage instructions
3. Follow the `TESTING.md` guide for proper testing
4. Experiment with the configuration options

## Getting Help

If you encounter issues:
1. Check the troubleshooting section above
2. Verify your compiler installation
3. Try the alternative build script (`build-alt.bat`)
4. Check VS Code's integrated terminal for detailed error messages
