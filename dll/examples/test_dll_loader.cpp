#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>

// Function pointer types for DLL exports
typedef bool (*StartCaptureFunc)();
typedef void (*StopCaptureFunc)();
typedef bool (*IsCapturingFunc)();
typedef void (*SetUpdateIntervalFunc)(int);

int main() {
    std::cout << "CS 1.6 Data Capture DLL Test Loader" << std::endl;
    std::cout << "=====================================" << std::endl;

    // Load the DLL
    std::cout << "Loading DLL..." << std::endl;
    HMODULE hDll = LoadLibraryA("cs16_datacapture.dll");
    
    if (hDll == nullptr) {
        std::cerr << "Failed to load DLL. Error: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "DLL loaded successfully!" << std::endl;

    // Get exported functions
    auto StartCapture = (StartCaptureFunc)GetProcAddress(hDll, "StartCapture");
    auto StopCapture = (StopCaptureFunc)GetProcAddress(hDll, "StopCapture");
    auto IsCapturing = (IsCapturingFunc)GetProcAddress(hDll, "IsCapturing");
    auto SetUpdateInterval = (SetUpdateIntervalFunc)GetProcAddress(hDll, "SetUpdateInterval");

    if (!StartCapture || !StopCapture || !IsCapturing || !SetUpdateInterval) {
        std::cerr << "Failed to get DLL functions" << std::endl;
        FreeLibrary(hDll);
        return 1;
    }

    std::cout << "Functions loaded successfully!" << std::endl;

    // Wait for DLL initialization
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Check if capturing
    if (IsCapturing()) {
        std::cout << "Capture is already running (auto-started)" << std::endl;
    } else {
        std::cout << "Starting capture..." << std::endl;
        if (StartCapture()) {
            std::cout << "Capture started successfully!" << std::endl;
        } else {
            std::cerr << "Failed to start capture" << std::endl;
        }
    }

    // Set update interval to 100ms
    std::cout << "Setting update interval to 100ms" << std::endl;
    SetUpdateInterval(100);

    // Run for 30 seconds
    std::cout << "\nCapturing data for 30 seconds..." << std::endl;
    std::cout << "Check cs16_datacapture.log for detailed logs" << std::endl;
    std::cout << "Make sure WebSocket server is running on localhost:8080" << std::endl;

    for (int i = 30; i > 0; --i) {
        std::cout << "\rTime remaining: " << i << " seconds   " << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "\n\nStopping capture..." << std::endl;
    StopCapture();

    std::cout << "Unloading DLL..." << std::endl;
    FreeLibrary(hDll);

    std::cout << "Test completed!" << std::endl;
    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}
