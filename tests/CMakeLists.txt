############################################################
# The ISIS project
# 
# Main CMake configuration file of the ISIS tests.
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Thu, 07 Apr 2011 14:39:17 +0200
# 
############################################################

include_directories(${CMAKE_SOURCE_DIR}/lib/Core)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)
include_directories(${Boost_INCLUDE_DIR})

string(REGEX MATCH "\\.[aA]$" DOT_A ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
string(REGEX MATCH "\\.[lL][iI][bB]$" DOT_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

message(STATUS ${DOT_LIB})

if(DOT_LIB OR DOT_A)
	message(STATUS "${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} seems to be a static library wont use \"-DBOOST_TEST_DYN_LINK\"")
else(DOT_LIB OR DOT_A)
	add_definitions("-DBOOST_TEST_DYN_LINK")
	message(STATUS "${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} seems to be a dynamic library adding \"-DBOOST_TEST_DYN_LINK\"")
endif(DOT_LIB OR DOT_A)


add_subdirectory(CoreUtils)
add_subdirectory(DataStorage)
add_subdirectory(ImageIO)
add_subdirectory(ExternalLibraryAdapter)

# addition option: build benchmarks
option(ISIS_BUILD_BENCHMARKS "Build the ISIS stress tests. For internal use only" OFF)

if(ISIS_BUILD_BENCHMARKS)
  message(STATUS "Building stress tests")
  add_subdirectory(benchmarks)
endif(ISIS_BUILD_BENCHMARKS)
