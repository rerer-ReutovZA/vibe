#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "game_types.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace CS16Capture {

/**
 * @brief Memory reader for safe reading from game memory
 */
class MemoryReader {
public:
    MemoryReader();
    ~MemoryReader();

    /**
     * @brief Initialize the memory reader
     * @return true if initialization was successful
     */
    bool initialize();

    /**
     * @brief Read a value from memory
     * @tparam T Type of value to read
     * @param address Memory address to read from
     * @param outValue Reference to store the read value
     * @return true if read was successful
     */
    template<typename T>
    bool readMemory(uintptr_t address, T& outValue);

    /**
     * @brief Read a string from memory
     * @param address Memory address to read from
     * @param maxLength Maximum length of the string
     * @return The read string (empty if failed)
     */
    std::string readString(uintptr_t address, size_t maxLength = 256);

    /**
     * @brief Check if a memory address is valid
     * @param address Memory address to check
     * @return true if the address is valid
     */
    bool isValidAddress(uintptr_t address);

    /**
     * @brief Get the base address of a module
     * @param moduleName Name of the module (e.g., "hl.exe", "hw.dll")
     * @return Base address of the module (0 if not found)
     */
    uintptr_t getModuleBase(const std::string& moduleName);

    /**
     * @brief Find a pattern in memory
     * @param pattern Byte pattern to search for
     * @param mask Mask for the pattern (x = match, ? = wildcard)
     * @param startAddress Start address for search
     * @param searchSize Size of memory region to search
     * @return Address of the pattern (0 if not found)
     */
    uintptr_t findPattern(const std::vector<uint8_t>& pattern, 
                          const std::string& mask,
                          uintptr_t startAddress, 
                          size_t searchSize);

private:
    bool isInitialized_;
#ifdef _WIN32
    HANDLE processHandle_;
#endif
};

// Template implementation
template<typename T>
bool MemoryReader::readMemory(uintptr_t address, T& outValue) {
    if (!isInitialized_ || !isValidAddress(address)) {
        return false;
    }

#ifdef _WIN32
    SIZE_T bytesRead;
    return ReadProcessMemory(processHandle_, 
                           reinterpret_cast<LPCVOID>(address),
                           &outValue, 
                           sizeof(T), 
                           &bytesRead) && bytesRead == sizeof(T);
#else
    // Linux implementation would go here
    return false;
#endif
}

} // namespace CS16Capture
