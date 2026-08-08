cmake_minimum_required(VERSION 3.24)

project(secp256k1-wrapper VERSION 2.1.0 LANGUAGES C)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Build options
option(SECP256K1_WRAPPER_BUILD_TESTS "Build test suite" OFF)
option(SECP256K1_WRAPPER_BUILD_EXAMPLES "Build example programs" OFF)

# Library type follows the standard CMake variable; static by default,
# matching the target the `secp256k1-wrapper` alias resolved to before.
if(NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS OFF)
endif()

include(GNUInstallDirs)
include(FetchContent)
include(CMakePackageConfigHelpers)
include(CheckSymbolExists)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)
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
set(SECP256K1_ENABLE_MODULE_SILENTPAYMENTS OFF CACHE BOOL "Enable Silent Payments module")

# Fetch secp256k1 dependency
message(STATUS "Fetching secp256k1 dependency")

FetchContent_Declare(secp256k1
  GIT_REPOSITORY https://github.com/bitcoin-core/secp256k1.git
  GIT_TAG        6e2c8bc4ecdc6e71dbe7a368f360d8d453ce435d  # v0.8.0
)

set(_sw_saved_bsl ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)   # we embed secp256k1's objects; no separate .so wanted
FetchContent_MakeAvailable(secp256k1)
set(BUILD_SHARED_LIBS ${_sw_saved_bsl})
unset(_sw_saved_bsl)

# Define wrapper library sources
set(WRAPPER_SOURCES src/secp256k1_wrapper.c)

# Platform-specific libraries
if(APPLE)
    find_library(SECURITY_FRAMEWORK Security)
    if(NOT SECURITY_FRAMEWORK)
        message(FATAL_ERROR "Security framework not found")
    endif()
    set(PLATFORM_LIBS ${SECURITY_FRAMEWORK}) # CommonCrypto is headers-only in libSystem
else()
    # Linux - no additional libraries needed
    set(PLATFORM_LIBS)
endif()

# explicit_bzero is a libc-guaranteed secure-wipe primitive (not reliant on
# compiler-barrier semantics); prefer it when available.
check_symbol_exists(explicit_bzero "string.h;strings.h" HAVE_EXPLICIT_BZERO)

# Create wrapper library target (type follows BUILD_SHARED_LIBS)
add_library(secp256k1-wrapper-lib)

target_sources(secp256k1-wrapper-lib
    PRIVATE
    ${WRAPPER_SOURCES}
    $<TARGET_PROPERTY:secp256k1_objs,INTERFACE_SOURCES> # Embed secp256k1 sources
)

target_sources(secp256k1-wrapper-lib
    PUBLIC
    FILE_SET HEADERS
        BASE_DIRS include
        FILES include/secp256k1_wrapper.h
)

target_link_libraries(secp256k1-wrapper-lib PRIVATE ${PLATFORM_LIBS})

target_include_directories(secp256k1-wrapper-lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        $<TARGET_PROPERTY:secp256k1,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_features(secp256k1-wrapper-lib PRIVATE c_std_99)
target_compile_options(secp256k1-wrapper-lib PRIVATE -Wall -Wextra -Wpedantic)
if(HAVE_EXPLICIT_BZERO)
    target_compile_definitions(secp256k1-wrapper-lib PRIVATE HAVE_EXPLICIT_BZERO)
endif()

set_target_properties(secp256k1-wrapper-lib PROPERTIES
    OUTPUT_NAME secp256k1-wrapper
    EXPORT_NAME secp256k1-wrapper
    POSITION_INDEPENDENT_CODE ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Generic alias for downstream consumers, plus one type-specific alias
# matching whichever type BUILD_SHARED_LIBS actually produced.
add_library(secp256k1-wrapper ALIAS secp256k1-wrapper-lib)
if(BUILD_SHARED_LIBS)
    add_library(secp256k1-wrapper::shared ALIAS secp256k1-wrapper-lib)
else()
    add_library(secp256k1-wrapper::static ALIAS secp256k1-wrapper-lib)
endif()

# Build tests
if(SECP256K1_WRAPPER_BUILD_TESTS)

    FetchContent_Declare(Unity
        GIT_REPOSITORY      https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG             v2.6.1
    )
    FetchContent_MakeAvailable(Unity)


    # Build test executable
    add_executable(test_wrapper tests/test_wrapper.c)

    target_include_directories(test_wrapper PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        $<TARGET_PROPERTY:secp256k1,INTERFACE_INCLUDE_DIRECTORIES>
    )

    target_link_libraries(test_wrapper PRIVATE secp256k1-wrapper unity ${PLATFORM_LIBS})

    target_compile_features(test_wrapper PRIVATE c_std_99)
    target_compile_options(test_wrapper PRIVATE -Wall -Wextra -Wpedantic)

    # Enable testing
    enable_testing()
    add_test(NAME wrapper_tests COMMAND test_wrapper)
    
    message(STATUS "Test suite enabled - run 'make test' or 'ctest' to run tests")
endif()

# Build examples
if(SECP256K1_WRAPPER_BUILD_EXAMPLES)
    add_executable(demo examples/demo.c)
    target_include_directories(demo PRIVATE include)
    target_link_libraries(demo PRIVATE secp256k1-wrapper ${PLATFORM_LIBS})
    
    # Set C standard and compiler options for demo target
    target_compile_features(demo PRIVATE c_std_99)
    target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic)

    message(STATUS "Example programs enabled")
endif()

# Installation rules

# Install library + public headers (from FILE_SET) in standard dirs
install(TARGETS secp256k1-wrapper-lib
    EXPORT secp256k1-wrapperTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT secp256k1-wrapperTargets
    NAMESPACE secp256k1-wrapper::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/secp256k1-wrapper
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/secp256k1-wrapperConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/secp256k1-wrapperConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/secp256k1-wrapperConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/secp256k1-wrapper
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/secp256k1-wrapperConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/secp256k1-wrapperConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/secp256k1-wrapper
)

# Summary
message(STATUS "")
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
if(BUILD_SHARED_LIBS)
    message(STATUS "Library type: shared")
else()
    message(STATUS "Library type: static")
endif()
message(STATUS "Build tests: ${SECP256K1_WRAPPER_BUILD_TESTS}")
message(STATUS "Build examples: ${SECP256K1_WRAPPER_BUILD_EXAMPLES}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "====================================")
message(STATUS "")
