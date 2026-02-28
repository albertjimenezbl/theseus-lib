cmake_minimum_required(VERSION 3.14)

project(theseus VERSION 1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(FetchContent)

#
# DEPENDENCIES
#

FetchContent_Declare(
    libhandlegraph
    GIT_REPOSITORY https://github.com/vgteam/libhandlegraph.git
    GIT_TAG b95f7637968b4c8c3c4d1bad13aaa1835c607fe0
)

FetchContent_GetProperties(libhandlegraph)
if(NOT libhandlegraph_POPULATED)
    FetchContent_Populate(libhandlegraph)
    add_subdirectory(${libhandlegraph_SOURCE_DIR} ${libhandlegraph_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

#
# SETTINGS
#

# List of valid build types
set(VALID_BUILD_TYPES Debug Release RelWithDebInfo MinSizeRel)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as no build type was specified")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Check if the provided CMAKE_BUILD_TYPE is in the valid list
if(NOT CMAKE_BUILD_TYPE IN_LIST VALID_BUILD_TYPES)
    message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}. Valid options are: ${VALID_BUILD_TYPES}")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Option for shared/static library. Static by default.
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Tests enabled by default
option(ENABLE_TESTS "Enable building of tests" ON)

#
# THESEUS LIBRARY
#

# Find all source files in the theseus directory
file(GLOB_RECURSE THESEUS_SOURCES CONFIGURE_DEPENDS
    "theseus/*.cpp"
)

# Add the library
add_library(${PROJECT_NAME} ${THESEUS_SOURCES})

# Per-target C++ standard
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

# Per-target include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Link against dependencies
target_link_libraries(${PROJECT_NAME} PUBLIC handlegraph_static)

# Per-target warnings for debug builds (PRIVATE to avoid leaking to consumers)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wuninitialized -Wunused -Wshadow)
endif()

# Specify the installation properties for the library
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # Default to bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}  # Default to lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}  # Default to lib
)

# Install the export set so consumers can find_package()
install(EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Generate and install the Config file that loads dependencies first
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Install the include directory
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}  # Default to include
)

#
# TESTS
#


add_library(doctest_main OBJECT tests/doctest_main.cpp)

file(GLOB_RECURSE UNIT_TESTS CONFIGURE_DEPENDS
    "tests/unit/*.cpp"
)

# file(GLOB_RECURSE CPP_EXAMPLES CONFIGURE_DEPENDS
#     "examples/cpp/*.cpp"
# )

# file(GLOB_RECURSE C_EXAMPLES CONFIGURE_DEPENDS
#     "examples/c/*.c"
# )


#
# TOOLS
#

# Find all the tools source files
file(GLOB_RECURSE THESEUS_TOOLS_SOURCES CONFIGURE_DEPENDS
    "tools/*.cpp"
)

# Add executable for each tool
foreach(tool_source ${THESEUS_TOOLS_SOURCES})
    get_filename_component(tool_name ${tool_source} NAME_WE)
    add_executable(${tool_name} ${tool_source})
    target_link_libraries(${tool_name} PRIVATE ${PROJECT_NAME})
    set_target_properties(${tool_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools")
endforeach()

# Compile tests and add them to CTest
function(add_tests test_files output_directory is_ctest is_doctest)
    foreach(test_file ${test_files})
        get_filename_component(test_name ${test_file} NAME_WE)

        # Conditionally add doctest_main based on is_doctest
        if(is_doctest)
            add_executable(${test_name} ${test_file} $<TARGET_OBJECTS:doctest_main>)
        else()
            add_executable(${test_name} ${test_file})
        endif()

        # To access test files from the test.
        target_compile_definitions(${test_name} PUBLIC TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/")
        # To access penalty files from test.
        target_compile_definitions(${test_name} PUBLIC SUB_MATRICES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/sub_matrices/")

        target_link_libraries(${test_name} PRIVATE ${PROJECT_NAME})
        set_target_properties(${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${output_directory}")

        # Register the test with CTest
        if(is_ctest)
            add_test(NAME ${test_name} COMMAND ${test_name})
        endif()
    endforeach()
endfunction()

if(ENABLE_TESTS)
    enable_testing()

    add_tests("${UNIT_TESTS}" "tests/unit" TRUE TRUE)

    # add_tests("${CPP_EXAMPLES}" "examples/cpp" TRUE FALSE)

    # add_tests("${C_EXAMPLES}" "examples/c" TRUE FALSE)
endif()
