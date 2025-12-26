#include "../include/memory_reader.h"
#include "../include/logger.h"

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#endif

namespace CS16Capture {

MemoryReader::MemoryReader() 
    : isInitialized_(false)
#ifdef _WIN32
    , processHandle_(nullptr)
#endif
{
}

MemoryReader::~MemoryReader() {
#ifdef _WIN32
    if (processHandle_ != nullptr && processHandle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(processHandle_);
    }
#endif
}

bool MemoryReader::initialize() {
    if (isInitialized_) {
        return true;
    }

#ifdef _WIN32
    // Get current process handle
    processHandle_ = GetCurrentProcess();
    
    if (processHandle_ == nullptr || processHandle_ == INVALID_HANDLE_VALUE) {
        LOG_ERROR("Failed to get current process handle");
        return false;
    }

    isInitialized_ = true;
    LOG_INFO("MemoryReader initialized successfully");
    return true;
#else
    LOG_ERROR("MemoryReader not implemented for this platform");
    return false;
#endif
}

std::string MemoryReader::readString(uintptr_t address, size_t maxLength) {
    if (!isInitialized_ || !isValidAddress(address)) {
        return "";
    }

#ifdef _WIN32
    std::vector<char> buffer(maxLength + 1, 0);
    SIZE_T bytesRead;
    
    if (ReadProcessMemory(processHandle_, 
                         reinterpret_cast<LPCVOID>(address),
                         buffer.data(), 
                         maxLength, 
                         &bytesRead)) {
        buffer[bytesRead] = '\0';
        return std::string(buffer.data());
    }
#endif

    return "";
}

bool MemoryReader::isValidAddress(uintptr_t address) {
    if (!isInitialized_ || address == 0) {
        return false;
    }

#ifdef _WIN32
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQueryEx(processHandle_, 
                      reinterpret_cast<LPCVOID>(address), 
                      &mbi, 
                      sizeof(mbi)) == 0) {
        return false;
    }

    return (mbi.State == MEM_COMMIT) && 
           (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE));
#else
    return false;
#endif
}

uintptr_t MemoryReader::getModuleBase(const std::string& moduleName) {
#ifdef _WIN32
    HMODULE hModule = GetModuleHandleA(moduleName.c_str());
    if (hModule == nullptr) {
        LOG_WARNING("Failed to get module handle for: " + moduleName);
        return 0;
    }

    LOG_DEBUG("Module base for " + moduleName + ": " + 
              std::to_string(reinterpret_cast<uintptr_t>(hModule)));
    return reinterpret_cast<uintptr_t>(hModule);
#else
    return 0;
#endif
}

uintptr_t MemoryReader::findPattern(const std::vector<uint8_t>& pattern, 
                                    const std::string& mask,
                                    uintptr_t startAddress, 
                                    size_t searchSize) {
    if (!isInitialized_ || pattern.empty() || pattern.size() != mask.size()) {
        return 0;
    }

#ifdef _WIN32
    std::vector<uint8_t> buffer(searchSize);
    SIZE_T bytesRead;

    if (!ReadProcessMemory(processHandle_,
                          reinterpret_cast<LPCVOID>(startAddress),
                          buffer.data(),
                          searchSize,
                          &bytesRead)) {
        LOG_ERROR("Failed to read memory for pattern search");
        return 0;
    }

    for (size_t i = 0; i <= bytesRead - pattern.size(); ++i) {
        bool found = true;
        for (size_t j = 0; j < pattern.size(); ++j) {
            if (mask[j] == 'x' && buffer[i + j] != pattern[j]) {
                found = false;
                break;
            }
        }
        
        if (found) {
            LOG_DEBUG("Pattern found at offset: " + std::to_string(i));
            return startAddress + i;
        }
    }
#endif

    return 0;
}

} // namespace CS16Capture
