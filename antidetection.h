#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <winternl.h>
#include <vector>
#include <string>
#include <random>
#include <thread>
#include <chrono>
#include <memory>

// Anti-detection utilities for educational purposes
class AntiDetection {
private:
    static std::random_device rd;
    static std::mt19937 gen;
    static bool isInitialized;
    static HMODULE ntdll;
    static HMODULE kernel32;
    
    // Function pointers for direct syscalls
    typedef NTSTATUS(NTAPI* NtQueryInformationProcess_t)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
    typedef NTSTATUS(NTAPI* NtSetInformationThread_t)(HANDLE, THREADINFOCLASS, PVOID, ULONG);
    typedef BOOL(WINAPI* IsDebuggerPresent_t)();
    
    static NtQueryInformationProcess_t NtQueryInformationProcess;
    static NtSetInformationThread_t NtSetInformationThread;
    static IsDebuggerPresent_t pIsDebuggerPresent;

public:
    // Initialize anti-detection systems
    static bool Initialize() {
        if (isInitialized) return true;
        
        // Load system DLLs
        ntdll = GetModuleHandleA("ntdll.dll");
        kernel32 = GetModuleHandleA("kernel32.dll");
        
        if (!ntdll || !kernel32) return false;
        
        // Get function addresses
        NtQueryInformationProcess = (NtQueryInformationProcess_t)GetProcAddress(ntdll, "NtQueryInformationProcess");
        NtSetInformationThread = (NtSetInformationThread_t)GetProcAddress(ntdll, "NtSetInformationThread");
        pIsDebuggerPresent = (IsDebuggerPresent_t)GetProcAddress(kernel32, "IsDebuggerPresent");
        
        if (!NtQueryInformationProcess || !NtSetInformationThread || !pIsDebuggerPresent) {
            return false;
        }
        
        isInitialized = true;
        return true;
    }
    
    // Advanced debugger detection
    static bool IsBeingDebugged() {
        // Method 1: PEB BeingDebugged flag
        __try {
            PPEB peb = (PPEB)__readgsqword(0x60);
            if (peb->BeingDebugged) return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return true; // Exception might indicate debugging
        }
        
        // Method 2: NtGlobalFlag
        __try {
            PPEB peb = (PPEB)__readgsqword(0x60);
            if (peb->NtGlobalFlag & 0x70) return true; // FLG_HEAP_ENABLE_TAIL_CHECK | FLG_HEAP_ENABLE_FREE_CHECK | FLG_HEAP_VALIDATE_PARAMETERS
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return true;
        }
        
        // Method 3: Process Debug Port
        if (NtQueryInformationProcess) {
            HANDLE debugPort = nullptr;
            NTSTATUS status = NtQueryInformationProcess(GetCurrentProcess(), ProcessDebugPort, &debugPort, sizeof(debugPort), nullptr);
            if (NT_SUCCESS(status) && debugPort) return true;
        }
        
        // Method 4: Hardware breakpoints
        CONTEXT ctx = { 0 };
        ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        if (GetThreadContext(GetCurrentThread(), &ctx)) {
            if (ctx.Dr0 || ctx.Dr1 || ctx.Dr2 || ctx.Dr3) return true;
        }
        
        return false;
    }
    
    // Detect virtual machines and sandboxes
    static bool IsRunningInVM() {
        // Check for common VM artifacts
        std::vector<std::string> vmProcesses = {
            "vmtoolsd.exe", "vmwaretray.exe", "vmwareuser.exe",
            "vboxservice.exe", "vboxtray.exe", "vboxhost.exe",
            "qemu-ga.exe", "qemu-system", "xenservice.exe"
        };
        
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;
        
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(pe32);
        
        if (Process32First(snapshot, &pe32)) {
            do {
                std::string processName = pe32.szExeFile;
                for (const auto& vmProc : vmProcesses) {
                    if (processName.find(vmProc) != std::string::npos) {
                        CloseHandle(snapshot);
                        return true;
                    }
                }
            } while (Process32Next(snapshot, &pe32));
        }
        
        CloseHandle(snapshot);
        
        // Check registry keys for VM indicators
        HKEY hkey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\VBoxService", 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
            RegCloseKey(hkey);
            return true;
        }
        
        return false;
    }
    
    // Detect analysis tools and monitoring
    static bool IsBeingAnalyzed() {
        std::vector<std::string> analysisTools = {
            "ollydbg.exe", "x64dbg.exe", "x32dbg.exe", "windbg.exe",
            "ida.exe", "ida64.exe", "idaq.exe", "idaq64.exe",
            "ghidra.exe", "radare2.exe", "procmon.exe", "procexp.exe",
            "wireshark.exe", "fiddler.exe", "httpdebugger.exe",
            "cheatengine.exe", "artmoney.exe", "tsearch.exe"
        };
        
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;
        
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(pe32);
        
        if (Process32First(snapshot, &pe32)) {
            do {
                std::string processName = pe32.szExeFile;
                std::transform(processName.begin(), processName.end(), processName.begin(), ::tolower);
                
                for (const auto& tool : analysisTools) {
                    if (processName.find(tool) != std::string::npos) {
                        CloseHandle(snapshot);
                        return true;
                    }
                }
            } while (Process32Next(snapshot, &pe32));
        }
        
        CloseHandle(snapshot);
        return false;
    }
    
    // Generate human-like delays with natural variation
    static int GenerateHumanDelay(int baseDelay, double humanFactor = 1.5) {
        // Human reaction times follow a gamma distribution
        std::gamma_distribution<double> gamma(2.0, baseDelay / 2.0);
        double delay = gamma(gen) * humanFactor;
        
        // Add micro-movements and hesitation
        if (std::uniform_real_distribution<>(0.0, 1.0)(gen) < 0.1) {
            delay += std::uniform_int_distribution<>(50, 200)(gen); // Occasional hesitation
        }
        
        return static_cast<int>(std::max(10.0, delay));
    }
    
    // Simulate natural mouse jitter
    static void AddMouseJitter(int& x, int& y, int maxJitter = 2) {
        std::uniform_int_distribution<> jitter(-maxJitter, maxJitter);
        x += jitter(gen);
        y += jitter(gen);
    }
    
    // Advanced input simulation using low-level methods
    static void SendMouseInput(DWORD flags, DWORD data = 0) {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = flags;
        input.mi.dwExtraInfo = 0;
        input.mi.mouseData = data;
        input.mi.time = 0;
        
        // Add slight delay to simulate hardware timing
        std::this_thread::sleep_for(std::chrono::microseconds(
            std::uniform_int_distribution<>(100, 500)(gen)
        ));
        
        SendInput(1, &input, sizeof(INPUT));
    }
    
    // Obfuscated pixel reading using multiple methods
    static COLORREF SafeGetPixel(int x, int y) {
        static int method = 0;
        method = (method + 1) % 3; // Rotate methods
        
        switch (method) {
        case 0: {
            // Method 1: Standard GetPixel
            HDC hdc = GetDC(NULL);
            COLORREF color = GetPixel(hdc, x, y);
            ReleaseDC(NULL, hdc);
            return color;
        }
        case 1: {
            // Method 2: BitBlt to memory DC
            HDC hdcScreen = GetDC(NULL);
            HDC hdcMem = CreateCompatibleDC(hdcScreen);
            HBITMAP hbm = CreateCompatibleBitmap(hdcScreen, 1, 1);
            SelectObject(hdcMem, hbm);
            
            BitBlt(hdcMem, 0, 0, 1, 1, hdcScreen, x, y, SRCCOPY);
            COLORREF color = GetPixel(hdcMem, 0, 0);
            
            DeleteObject(hbm);
            DeleteDC(hdcMem);
            ReleaseDC(NULL, hdcScreen);
            return color;
        }
        case 2: {
            // Method 3: GetDIBits
            HDC hdc = GetDC(NULL);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, 1, 1);
            HDC hdcMem = CreateCompatibleDC(hdc);
            SelectObject(hdcMem, hbm);
            
            BitBlt(hdcMem, 0, 0, 1, 1, hdc, x, y, SRCCOPY);
            
            BITMAPINFO bmi = { 0 };
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = 1;
            bmi.bmiHeader.biHeight = 1;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            
            DWORD pixel;
            GetDIBits(hdcMem, hbm, 0, 1, &pixel, &bmi, DIB_RGB_COLORS);
            
            DeleteObject(hbm);
            DeleteDC(hdcMem);
            ReleaseDC(NULL, hdc);
            
            return RGB((pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, pixel & 0xFF);
        }
        }
        return 0;
    }
    
    // Hide thread from debuggers
    static void HideThread() {
        if (NtSetInformationThread) {
            const DWORD ThreadHideFromDebugger = 0x11;
            NtSetInformationThread(GetCurrentThread(), (THREADINFOCLASS)ThreadHideFromDebugger, NULL, 0);
        }
    }
    
    // Memory protection and obfuscation
    static void ProtectMemory(void* address, size_t size) {
        DWORD oldProtect;
        VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
        
        // XOR obfuscation
        BYTE* bytes = static_cast<BYTE*>(address);
        BYTE key = static_cast<BYTE>(std::uniform_int_distribution<>(1, 255)(gen));
        
        for (size_t i = 0; i < size; i++) {
            bytes[i] ^= key;
        }
    }
    
    // Timing attack detection
    static bool DetectTimingAttack() {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Perform a simple operation
        volatile int dummy = 0;
        for (int i = 0; i < 1000; i++) {
            dummy += i;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // If operation takes significantly longer, might be under analysis
        return duration.count() > 10000; // 10ms threshold
    }
    
    // Comprehensive security check
    static bool PerformSecurityCheck() {
        if (!Initialize()) return false;
        
        if (IsBeingDebugged()) return false;
        if (IsRunningInVM()) return false;
        if (IsBeingAnalyzed()) return false;
        if (DetectTimingAttack()) return false;
        
        return true;
    }
    
    // Decoy operations to confuse static analysis
    static void PerformDecoyOperations() {
        // Fake network activity
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock != INVALID_SOCKET) closesocket(sock);
        WSACleanup();
        
        // Fake file operations
        HANDLE hFile = CreateFileA("C:\\Windows\\System32\\kernel32.dll", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            GetFileSize(hFile, NULL);
            CloseHandle(hFile);
        }
        
        // Fake registry operations
        HKEY hkey;
        RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &hkey);
        RegCloseKey(hkey);
    }
};

// Initialize static members
std::random_device AntiDetection::rd;
std::mt19937 AntiDetection::gen(AntiDetection::rd());
bool AntiDetection::isInitialized = false;
HMODULE AntiDetection::ntdll = nullptr;
HMODULE AntiDetection::kernel32 = nullptr;
AntiDetection::NtQueryInformationProcess_t AntiDetection::NtQueryInformationProcess = nullptr;
AntiDetection::NtSetInformationThread_t AntiDetection::NtSetInformationThread = nullptr;
AntiDetection::IsDebuggerPresent_t AntiDetection::pIsDebuggerPresent = nullptr;
