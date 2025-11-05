cmake_minimum_required(VERSION 3.30)
project(LangulusLiteral
    VERSION         1.0.0
    DESCRIPTION     "One template argument to rule them all"
    HOMEPAGE_URL    https://langulus.com
)

include(LangulusUtilities.cmake)

# Options                                                                       
option(LANGULUS_OPTION_SAFE_MODE 
    "Overrides additional error checking and sanity checks, \
    incurs a serious runtime overhead, disabled by default" OFF)

option(LANGULUS_OPTION_TESTING 
    "Builds tests, disabled by default" OFF)

# Check if this project is built as standalone, or as a part of something else  
if (PROJECT_IS_TOP_LEVEL OR NOT LANGULUS)
    # It is very important these are set before any targets are introduced      
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

# Define the library                                                            
add_library(LangulusLiteral INTERFACE)
target_compile_features(LangulusLiteral INTERFACE cxx_std_23)
target_include_directories(LangulusLiteral INTERFACE include)

# Reflect all the options, so that code is aware of them                        
macro(reflect_option OPTION MESSAGE)
    if (${OPTION})
        message(STATUS "[FEATURE] ${MESSAGE} (${OPTION})")
        target_compile_definitions(LangulusLiteral INTERFACE "$<$<BOOL:${OPTION}>:${OPTION}>")
    endif()
endmacro()

reflect_option(LANGULUS_OPTION_SAFE_MODE    "Safe mode enabled")
reflect_option(LANGULUS_OPTION_TESTING      "Tests enabled")

# Include tests                                                                 
if (LANGULUS_OPTION_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
