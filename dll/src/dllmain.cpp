#include "../include/game_data_capture.h"
#include "../include/logger.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <memory>
#include <string>

// Global instance of the capture system
static std::unique_ptr<CS16Capture::GameDataCapture> g_captureSystem;

// Configuration
static const std::string WS_HOST = "localhost";
static const int WS_PORT = 8080;
static const std::string LOG_FILE = "cs16_datacapture.log";

#ifdef _WIN32
/**
 * @brief DLL entry point
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            // Initialize logger
            CS16Capture::Logger::getInstance().initialize(LOG_FILE);
            CS16Capture::Logger::getInstance().setLogLevel(CS16Capture::LogLevel::DEBUG);
            
            LOG_INFO("=================================================");
            LOG_INFO("CS 1.6 Data Capture DLL - Version 1.0.0");
            LOG_INFO("=================================================");
            LOG_INFO("DLL attached to process");

            // Create and initialize capture system
            g_captureSystem = std::make_unique<CS16Capture::GameDataCapture>();
            
            if (g_captureSystem->initialize(WS_HOST, WS_PORT)) {
                LOG_INFO("Capture system initialized successfully");
                
                // Start capturing
                if (g_captureSystem->start()) {
                    LOG_INFO("Capture system started successfully");
                } else {
                    LOG_ERROR("Failed to start capture system");
                }
            } else {
                LOG_ERROR("Failed to initialize capture system");
            }
            
            break;
        }
        
        case DLL_THREAD_ATTACH:
            // Do nothing
            break;
            
        case DLL_THREAD_DETACH:
            // Do nothing
            break;
            
        case DLL_PROCESS_DETACH: {
            LOG_INFO("DLL detaching from process");
            
            // Stop and cleanup capture system
            if (g_captureSystem) {
                g_captureSystem->stop();
                g_captureSystem.reset();
            }
            
            LOG_INFO("Capture system stopped and cleaned up");
            LOG_INFO("=================================================");
            
            // Close logger
            CS16Capture::Logger::getInstance().close();
            
            break;
        }
    }
    
    return TRUE;
}
#endif

// Exported functions for manual control (optional)
extern "C" {

#ifdef _WIN32
    __declspec(dllexport)
#endif
    bool StartCapture() {
        if (g_captureSystem) {
            return g_captureSystem->start();
        }
        return false;
    }

#ifdef _WIN32
    __declspec(dllexport)
#endif
    void StopCapture() {
        if (g_captureSystem) {
            g_captureSystem->stop();
        }
    }

#ifdef _WIN32
    __declspec(dllexport)
#endif
    bool IsCapturing() {
        if (g_captureSystem) {
            return g_captureSystem->isRunning();
        }
        return false;
    }

#ifdef _WIN32
    __declspec(dllexport)
#endif
    void SetUpdateInterval(int intervalMs) {
        if (g_captureSystem) {
            g_captureSystem->setUpdateInterval(intervalMs);
        }
    }

} // extern "C"
