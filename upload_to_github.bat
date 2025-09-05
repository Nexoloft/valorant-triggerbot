@echo off
REM Script to upload triggerbot to GitHub
REM Make sure Git is installed before running this script

echo ==========================================
echo Uploading Triggerbot to GitHub
echo ==========================================

REM Check if git is available
where git >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Git not found!
    echo Please install Git from: https://git-scm.com/download/win
    pause
    exit /b 1
)

echo Step 1: Initializing Git repository...
git init

echo Step 2: Adding all files...
git add .

echo Step 3: Creating initial commit...
git commit -m "Initial commit: Educational Valorant-style Triggerbot with purple color detection"

echo Step 4: Adding remote repository...
git remote add origin https://github.com/Nexoloft/valorant-triggerbot.git

echo Step 5: Pushing to GitHub...
git branch -M main
git push -u origin main

echo ==========================================
echo Upload complete!
echo ==========================================
echo Your triggerbot has been uploaded to:
echo https://github.com/Nexoloft/valorant-triggerbot
echo.
pause
