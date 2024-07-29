set(ENV{CC}  cl)
set(ENV{CXX} cl)
set(ENV{CFLAGS} /WX)
set(ENV{CXXFLAGS} /WX)

set(dashboard_cache "
BUILD_TESTING:BOOL=ON
ADIOS2_BUILD_EXAMPLES:BOOL=ON

ADIOS2_USE_BZip2:BOOL=OFF
ADIOS2_USE_Fortran:BOOL=OFF
ADIOS2_USE_MPI:BOOL=ON
ADIOS2_USE_Python=OFF
")

set(CTEST_TEST_TIMEOUT 500)
set(CTEST_CMAKE_GENERATOR "Visual Studio 16 2019")
set(CTEST_CMAKE_GENERATOR_PLATFORM "x64")
list(APPEND CTEST_UPDATE_NOTES_FILES "${CMAKE_CURRENT_LIST_FILE}")
include(${CMAKE_CURRENT_LIST_DIR}/ci-common.cmake)
