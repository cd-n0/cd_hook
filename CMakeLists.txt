cmake_minimum_required(VERSION 3.5)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("cd_hook")

include(CTest)

# Set C standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define directories for main sources, tests, and includes
set(SRCDIR "${CMAKE_SOURCE_DIR}/src")
set(TESTDIR "${CMAKE_SOURCE_DIR}/tests")
set(INCLUDEDIR "${CMAKE_SOURCE_DIR}/include")

# Collect the main project source files (assumed to be C sources)
file(GLOB_RECURSE MAIN_SOURCES "${SRCDIR}/*.c")

# Build a static library from the main sources (similar to ar rcs)
add_library(${PROJECT_NAME}_lib STATIC ${MAIN_SOURCES})
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${INCLUDEDIR})

set(TARGET ${PROJECT_NAME}_lib)

# Collect all test source files
file(GLOB TEST_SOURCES "${TESTDIR}/*.cpp")

# Enable testing and include CTest module
enable_testing()

# Build each test as its own executable and link against the main target.
foreach(TEST_SOURCE ${TEST_SOURCES})
  # Extract a name for the test executable based on the source file name.
  get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

  add_executable(${TEST_NAME} ${TEST_SOURCE} "tests/test_common.h")
  
  # Link the primary target (the library or executable built above)
  target_link_libraries(${TEST_NAME} PRIVATE ${TARGET})

  # Set include directories for the tests if needed.
  target_include_directories(${TEST_NAME} PRIVATE ${INCLUDEDIR})

  # Register the test with CTest
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
