#include <windows.h>
#include "logger.h"
#include "cs16_capture.h"

static const std::string LOG_FILE = "dll_log.txt";

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            Logger::getInstance().initialize(LOG_FILE);
            Logger::getInstance().setLogLevel(LogLevel::DEBUG);
            Logger::getInstance().logInfo("DLL_PROCESS_ATTACH");
            CS16Capture::getInstance().initialize();
            break;
        }
        case DLL_PROCESS_DETACH: {
            Logger::getInstance().logInfo("DLL_PROCESS_DETACH");
            CS16Capture::getInstance().shutdown();
            Logger::getInstance().close();
            break;
        }
        case DLL_THREAD_ATTACH: {
            break;
        }
        case DLL_THREAD_DETACH: {
            break;
        }
    }
    return TRUE;
}
