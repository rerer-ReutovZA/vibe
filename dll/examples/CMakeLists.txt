cmake_minimum_required(VERSION 3.15)
project(CS16DataCaptureExamples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Test DLL Loader
if(WIN32)
    add_executable(test_dll_loader test_dll_loader.cpp)
    
    # Make sure it can find the DLL
    set_target_properties(test_dll_loader PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    # Copy DLL to the same directory as the test executable
    add_custom_command(TARGET test_dll_loader POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/../bin/cs16_datacapture.dll
        $<TARGET_FILE_DIR:test_dll_loader>
        COMMENT "Copying DLL to test directory"
    )
endif()
