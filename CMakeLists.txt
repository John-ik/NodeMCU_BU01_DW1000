cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# COMPILE_ID
# increment on each build
set(COMPILE_ID_FILE "${CMAKE_CURRENT_SOURCE_DIR}/compile_id.txt")

if(NOT EXISTS ${COMPILE_ID_FILE})
    file(WRITE ${COMPILE_ID_FILE} "1\n")
endif()

# Function to read the current version from the file
function(read_id)
    file(READ ${COMPILE_ID_FILE} CURRENT_COMPILE_ID)
    string(STRIP ${CURRENT_COMPILE_ID} CURRENT_COMPILE_ID)  # Remove any whitespace
    set(COMPILE_ID ${CURRENT_COMPILE_ID} PARENT_SCOPE)
endfunction()

# Function to increment the version number
function(increment_version)
    read_id()
    math(EXPR NEW_COMPILE_ID "${COMPILE_ID} + 1")
    file(WRITE ${COMPILE_ID_FILE} "${NEW_COMPILE_ID}\n")  # Write the new version back to the file
    set(COMPILE_ID ${NEW_COMPILE_ID} PARENT_SCOPE)
endfunction()

# Increment the version number on each build
increment_version()

# Print the current version
message(STATUS "Current compile_id: ${COMPILE_ID}")
add_definitions(-DCOMPILE_ID=${COMPILE_ID})

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDWT_API_ERROR_CHECK) # assert-ы в dwt функциях
    # внутри либы я переопределил assert на assert_param
    add_definitions(-DUSE_FULL_ASSERT) # assert_param
endif()

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME NodeMCU_BU01_DW1000)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})


FILE(GLOB LIBS_INC "libs/*/include")
FILE(GLOB LIBS_SRC "libs/*/src/*.c")

include_directories(${LIBS_INC})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME} ${LIBS_SRC})


# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
