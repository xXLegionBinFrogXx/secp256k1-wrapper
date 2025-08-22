cmake_minimum_required(VERSION 3.24)

project(secp256k1-wrapper VERSION 1.3.0 LANGUAGES C)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Build options
option(BUILD_TESTS "Build test suite" OFF)
option(BUILD_EXAMPLES "Build example programs" OFF)
option(BUILD_SHARED "Build shared library" ON)
option(BUILD_STATIC "Build static library" ON)

include(GNUInstallDirs)
include(FetchContent)

# Configure secp256k1 options before fetching
set(SECP256K1_BUILD_BENCHMARK OFF CACHE BOOL "Build benchmark")
set(SECP256K1_BUILD_TESTS OFF CACHE BOOL "Build tests")
set(SECP256K1_BUILD_EXHAUSTIVE_TESTS OFF CACHE BOOL "Build exhaustive tests") 
set(SECP256K1_BUILD_CTIME_TESTS OFF CACHE BOOL "Build ctime tests")
set(SECP256K1_BUILD_EXAMPLES OFF CACHE BOOL "Build examples")
set(SECP256K1_ENABLE_MODULE_RECOVERY ON CACHE BOOL "Enable recovery module")
set(SECP256K1_ENABLE_MODULE_EXTRAKEYS OFF CACHE BOOL "Enable extrakeys module")
set(SECP256K1_ENABLE_MODULE_SCHNORRSIG OFF CACHE BOOL "Enable schnorrsig module")
set(SECP256K1_ENABLE_MODULE_ECDH OFF CACHE BOOL "Enable ECDH module")
set(SECP256K1_ENABLE_MODULE_ELLSWIFT OFF CACHE BOOL "Enable ellswift module")
set(SECP256K1_ENABLE_MODULE_MUSIG OFF CACHE BOOL "Enable musig module")

# Fetch secp256k1 dependency
message(STATUS "Fetching secp256k1 dependency")

FetchContent_Declare(secp256k1
  GIT_REPOSITORY https://github.com/bitcoin-core/secp256k1.git
  GIT_TAG        v0.7.0
)

FetchContent_MakeAvailable(secp256k1)

# Define wrapper library sources
set(WRAPPER_SOURCES src/secp256k1_wrapper.c)

# Platform-specific libraries
if(WIN32)
    set(PLATFORM_LIBS bcrypt)
elseif(APPLE)
    find_library(SECURITY_FRAMEWORK Security)
    if(NOT SECURITY_FRAMEWORK)
        message(FATAL_ERROR "Security framework not found")
    endif()
    set(PLATFORM_LIBS ${SECURITY_FRAMEWORK}) # CommonCrypto is headers-only in libSystem
else()
    # Linux/Unix - no additional libraries needed
    set(PLATFORM_LIBS)
endif()

# Create static library target
if(BUILD_STATIC)
    add_library(secp256k1-wrapper-static STATIC)
    
    target_sources(secp256k1-wrapper-static
        PRIVATE
        ${WRAPPER_SOURCES}
    )

    target_sources(secp256k1-wrapper-static
        PUBLIC
        FILE_SET HEADERS 
            BASE_DIRS include
            FILES include/secp256k1_wrapper.h
    )

    # Use secp256k1_objs to embed object files into our static library (v0.7.0+ feature)
    target_link_libraries(secp256k1-wrapper-static PRIVATE secp256k1_objs ${PLATFORM_LIBS})
    
    target_compile_features(secp256k1-wrapper-static PUBLIC c_std_99)
    if(MSVC)
        target_compile_options(secp256k1-wrapper-static PRIVATE /W4)
    else()
        target_compile_options(secp256k1-wrapper-static PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    
    set_target_properties(secp256k1-wrapper-static PROPERTIES
        OUTPUT_NAME secp256k1-wrapper
        POSITION_INDEPENDENT_CODE ON
    )
    
    # Create alias for consistent naming
    add_library(secp256k1-wrapper::static ALIAS secp256k1-wrapper-static)
endif()

# Create shared library target  
if(BUILD_SHARED)
    add_library(secp256k1-wrapper-shared SHARED)

    target_sources(secp256k1-wrapper-shared
        PRIVATE
        ${WRAPPER_SOURCES}
    )

    target_sources(secp256k1-wrapper-shared
        PUBLIC
        FILE_SET HEADERS 
            BASE_DIRS include
            FILES include/secp256k1_wrapper.h
    )

    # Shared library can link to secp256k1 normally
    target_link_libraries(secp256k1-wrapper-shared PRIVATE secp256k1 ${PLATFORM_LIBS})
    

    target_compile_features(secp256k1-wrapper-shared PUBLIC c_std_99)
    if(MSVC)
        target_compile_options(secp256k1-wrapper-shared PRIVATE /W4)
    else()
        target_compile_options(secp256k1-wrapper-shared PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    
    set_target_properties(secp256k1-wrapper-shared PROPERTIES
        OUTPUT_NAME secp256k1-wrapper
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
    
    # Create alias for consistent naming
    add_library(secp256k1-wrapper::shared ALIAS secp256k1-wrapper-shared)
endif()

# Set default library target
if(BUILD_STATIC)
    set(DEFAULT_LIBRARY_TARGET secp256k1-wrapper-static)
elseif(BUILD_SHARED)
    set(DEFAULT_LIBRARY_TARGET secp256k1-wrapper-shared)
endif()

# Create generic alias pointing to default target
if(DEFAULT_LIBRARY_TARGET)
    add_library(secp256k1-wrapper ALIAS ${DEFAULT_LIBRARY_TARGET})
endif()

# Build tests
if(BUILD_TESTS AND DEFAULT_LIBRARY_TARGET)

    FetchContent_Declare(Unity
        GIT_REPOSITORY      https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG             v2.6.1
    )
    FetchContent_MakeAvailable(Unity)
    
    
    # Build test executable
    add_executable(test_wrapper tests/test_wrapper.c)
    target_include_directories(test_wrapper PRIVATE 
        include
    )
    target_link_libraries(test_wrapper PRIVATE ${DEFAULT_LIBRARY_TARGET} unity ${PLATFORM_LIBS})
    
    target_compile_features(test_wrapper PRIVATE c_std_99)
    if(MSVC)
        target_compile_options(test_wrapper PRIVATE /W4)
    else()
        target_compile_options(test_wrapper PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    
    # Enable testing
    enable_testing()
    add_test(NAME wrapper_tests COMMAND test_wrapper)
    
    message(STATUS "Test suite enabled - run 'make test' or 'ctest' to run tests")
endif()

# Build examples
if(BUILD_EXAMPLES AND DEFAULT_LIBRARY_TARGET)
    add_executable(demo examples/demo.c)
    target_include_directories(demo PRIVATE include)
    target_link_libraries(demo PRIVATE ${DEFAULT_LIBRARY_TARGET} ${PLATFORM_LIBS})
    
    # Set C standard and compiler options for demo target
    target_compile_features(demo PRIVATE c_std_99)
    if(MSVC)
        target_compile_options(demo PRIVATE /W4)
    else()
        target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    
    message(STATUS "Example programs enabled")
endif()

# Installation rules
if(BUILD_STATIC OR BUILD_SHARED)
    # Collect targets to install
    set(INSTALL_TARGETS)
    if(BUILD_STATIC)
        list(APPEND INSTALL_TARGETS secp256k1-wrapper-static)
    endif()
    if(BUILD_SHARED)
        list(APPEND INSTALL_TARGETS secp256k1-wrapper-shared)
    endif()

    # Install libraries + public headers (from FILE_SET) in standard dirs
    install(TARGETS ${INSTALL_TARGETS}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

endif()

# Summary
message(STATUS "")
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Build static library: ${BUILD_STATIC}")
message(STATUS "Build shared library: ${BUILD_SHARED}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Build examples: ${BUILD_EXAMPLES}")
if(DEFAULT_LIBRARY_TARGET)
    message(STATUS "Default library target: ${DEFAULT_LIBRARY_TARGET}")
endif()
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "====================================")
message(STATUS "")