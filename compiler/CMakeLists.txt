cmake_minimum_required(VERSION 3.14)
project(TinyCompiler VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compilation database for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include LLVM headers
include_directories(${LLVM_INCLUDE_DIRS})

# Include project headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Collect source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.cc"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Map LLVM components to library names - use LLVM_AVAILABLE_LIBS for MSYS2 compatibility
if(LLVM_AVAILABLE_LIBS)
    # Use available libraries directly
    set(llvm_libs "")
    foreach(lib ${LLVM_AVAILABLE_LIBS})
        list(APPEND llvm_libs "${lib}")
    endforeach()
else()
    # Fallback to component mapping
    llvm_map_components_to_libnames(llvm_libs
        core
        irreader
        support
        nativecodegen
        mcjit
        x86info
    )
endif()

# Link LLVM libraries
target_link_libraries(${PROJECT_NAME} ${llvm_libs})

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
