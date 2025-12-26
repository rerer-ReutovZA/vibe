cmake_minimum_required(VERSION 3.15)
project(CS16DataCapture VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32 -D_WINDOWS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/hlsdk
    ${CMAKE_CURRENT_SOURCE_DIR}/include/websocketpp
)

# Source files
set(SOURCES
    src/dllmain.cpp
    src/data_capture.cpp
    src/websocket_client.cpp
    src/logger.cpp
    src/memory_reader.cpp
)

# Header files
set(HEADERS
    include/data_capture.h
    include/websocket_client.h
    include/logger.h
    include/memory_reader.h
    include/game_types.h
)

# Create shared library (DLL)
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# Link libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ws2_32
        wsock32
    )
endif()

# Set output name
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "cs16_datacapture"
    PREFIX ""
    SUFFIX ".dll"
)

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

# Install rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
