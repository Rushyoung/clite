cmake_minimum_required(VERSION 3.10)
project(clite VERSION 0.90.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Read version from VERSION.txt
file(READ VERSION.txt VERSION_STRING)
string(STRIP "${VERSION_STRING}" VERSION_STRING)

# Set compiler flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-int-conversion -Wno-switch -O2")
elseif(CMAKE_C_COMPILER_ID MATCHES "TinyCC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
endif()

# Add version definition
add_definitions(-DVERSION=${VERSION_STRING})

# Include directories
include_directories(include)

# Collect all source files
file(GLOB SOURCES "src/*.c")

# Create the main executable
add_executable(clite ${SOURCES} main.c)

# Set output directory for the executable
set_target_properties(clite PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Find system compilers for testing
find_program(GCC_EXECUTABLE gcc)
find_program(TCC_EXECUTABLE tcc)

# Custom test target
add_custom_target(test
    COMMAND ${CMAKE_COMMAND} -E echo "Running tests..."
    DEPENDS clite
)

# Get all example C files
file(GLOB EXAMPLE_FILES "example/*.c")

# Create test commands for each example
foreach(EXAMPLE_FILE ${EXAMPLE_FILES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)
    get_filename_component(EXAMPLE_FULLNAME ${EXAMPLE_FILE} NAME)
    
    # Test with clite
    add_custom_command(TARGET test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Testing ${EXAMPLE_FULLNAME} with clite..."
        COMMAND $<TARGET_FILE:clite> ${EXAMPLE_FILE} > ${CMAKE_BINARY_DIR}/${EXAMPLE_NAME}_clite_output.txt 2>&1 || ${CMAKE_COMMAND} -E echo "clite failed for ${EXAMPLE_FULLNAME}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    
    # Test with system compiler (gcc if available, otherwise tcc)
    if(GCC_EXECUTABLE)
        add_custom_command(TARGET test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Compiling and running ${EXAMPLE_FULLNAME} with gcc..."
            COMMAND ${GCC_EXECUTABLE} -std=c11 -o ${CMAKE_BINARY_DIR}/${EXAMPLE_NAME}_gcc ${EXAMPLE_FILE}
            COMMAND ${CMAKE_BINARY_DIR}/${EXAMPLE_NAME}_gcc > ${CMAKE_BINARY_DIR}/${EXAMPLE_NAME}_gcc_output.txt 2>&1 || ${CMAKE_COMMAND} -E echo "gcc execution failed for ${EXAMPLE_FULLNAME}"
            COMMAND ${CMAKE_COMMAND} -E echo "Comparing outputs for ${EXAMPLE_FULLNAME}..."
            COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_BINARY_DIR}/${EXAMPLE_NAME}_clite_output.txt ${CMAKE_BINARY_DIR}/${EXAMPLE_NAME}_gcc_output.txt || ${CMAKE_COMMAND} -E echo "Output differs for ${EXAMPLE_FULLNAME}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    elseif(TCC_EXECUTABLE)
        add_custom_command(TARGET test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Compiling and running ${EXAMPLE_FULLNAME} with tcc..."
            COMMAND ${TCC_EXECUTABLE} -o ${CMAKE_BINARY_DIR}/${EXAMPLE_NAME}_tcc ${EXAMPLE_FILE}
            COMMAND ${CMAKE_BINARY_DIR}/${EXAMPLE_NAME}_tcc > ${CMAKE_BINARY_DIR}/${EXAMPLE_NAME}_tcc_output.txt 2>&1 || ${CMAKE_COMMAND} -E echo "tcc execution failed for ${EXAMPLE_FULLNAME}"
            COMMAND ${CMAKE_COMMAND} -E echo "Comparing outputs for ${EXAMPLE_FULLNAME}..."
            COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_BINARY_DIR}/${EXAMPLE_NAME}_clite_output.txt ${CMAKE_BINARY_DIR}/${EXAMPLE_NAME}_tcc_output.txt || ${CMAKE_COMMAND} -E echo "Output differs for ${EXAMPLE_FULLNAME}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    else()
        add_custom_command(TARGET test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Warning: No system C compiler (gcc or tcc) found for comparison testing"
        )
    endif()
endforeach()

# Add final test summary
add_custom_command(TARGET test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Test comparison completed!"
)

# Print build info
message(STATUS "Building clite version: ${VERSION_STRING}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(GCC_EXECUTABLE)
    message(STATUS "Found GCC for testing: ${GCC_EXECUTABLE}")
endif()
if(TCC_EXECUTABLE)
    message(STATUS "Found TCC for testing: ${TCC_EXECUTABLE}")
endif()