cmake_minimum_required( VERSION 3.26.0 )

project(dccli)

# Set base directories
set(OCT_BASE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(OCT_CMAKE_DIR ${OCT_BASE_DIR}/tools/cmake)
set(OCT_EXT_LIBS_DIR ${OCT_BASE_DIR}/code/libs)
set(OCT_SDKS_DIR ${OCT_BASE_DIR}/code/sdks)

message(STATUS "OCT_BASE_DIR = ${OCT_BASE_DIR}")
message(STATUS "OCT_CMAKE_DIR = ${OCT_CMAKE_DIR}")
message(STATUS "OCT_EXT_LIBS_DIR = ${OCT_EXT_LIBS_DIR}")
message(STATUS "OCT_SDKS_DIR = ${OCT_SDKS_DIR}")

# Set C++ feature level
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set project configurations
if(NOT SETUP_CONFIGURATIONS_DONE)
    set(SETUP_CONFIGURATIONS_DONE TRUE)

    get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(isMultiConfig)
        set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebug;Release" CACHE STRING "" FORCE) 
    else()
        if(NOT CMAKE_BUILD_TYPE)
            message("Defaulting to release build.")
            set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
        endif()
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
        # set the valid options for cmake-gui drop-down list
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;RelWithDebug;Release")
    endif()
endif()

# Run toolchain file every time
if( DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_TOOLCHAIN_FILE}" )
	set(CMAKE_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" CACHE STRING "Toolchain File." FORCE)
elseif(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
	message(FATAL_ERROR "Cannot find cached toolchain file (${CMAKE_TOOLCHAIN_FILE}).")
else()
	include(${CMAKE_TOOLCHAIN_FILE})
endif()

message(STATUS "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
message(STATUS "OCT_TARGET_PLATFORM = ${OCT_TARGET_PLATFORM}")
message(STATUS "CMAKE_SYSTEM_VERSION = ${CMAKE_SYSTEM_VERSION}")

# CMake utilities
add_subdirectory(${OCT_CMAKE_DIR})
oct_setup_output_dirs()

# Project specific build chain
include(project.cmake)