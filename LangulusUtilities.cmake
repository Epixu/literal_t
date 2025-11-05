include(FetchContent)

# Utility for fetching external libraries using FetchContent                    
function(fetch_external_module NAME GIT_REPOSITORY REPO GIT_TAG TAG)
    if(NOT DEFINED LANGULUS_EXTERNAL_DIRECTORY)
        set(LANGULUS_EXTERNAL_DIRECTORY "${CMAKE_SOURCE_DIR}/external" CACHE PATH
            "Place where external dependencies will be downloaded")
        message(STATUS "LANGULUS_EXTERNAL_DIRECTORY not defined, \
        using default: ${LANGULUS_EXTERNAL_DIRECTORY}")
    endif()

    # Completely avoid downloading or updating anything, once the appropriate   
    # folder exists                                                             
    string(TOUPPER ${NAME} UPPERCASE_NAME)
    if (EXISTS "${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src")
        set(FETCHCONTENT_SOURCE_DIR_${UPPERCASE_NAME} "${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src" CACHE INTERNAL "" FORCE)
        message(STATUS "Reusing external library ${NAME}... \
        (delete ${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src manually in order to redownload)")
    else()
        unset(FETCHCONTENT_SOURCE_DIR_${UPPERCASE_NAME} CACHE)
        message(STATUS "Downloading external library ${NAME}...")
    endif()

    FetchContent_Declare(
        ${NAME}
        GIT_REPOSITORY  ${REPO}
        GIT_TAG         ${TAG}
        SOURCE_DIR      "${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src"
        SUBBUILD_DIR    "${CMAKE_BINARY_DIR}/external/${NAME}-subbuild"
        ${ARGN}
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(${NAME})
    
    string(TOLOWER ${NAME} LOWERCASE_NAME)
    set(${NAME}_SOURCE_DIR "${${LOWERCASE_NAME}_SOURCE_DIR}" CACHE INTERNAL "${NAME} source directory")
    set(${NAME}_BINARY_DIR "${${LOWERCASE_NAME}_BINARY_DIR}" CACHE INTERNAL "${NAME} binary directory")
endfunction()

# Create an executable                                                          
function(add_langulus_app NAME)
    set(multiValueArgs SOURCES LIBRARIES DEPENDENCIES EMSCRIPTEN_COMPILE_FLAGS EMSCRIPTEN_LINK_FLAGS)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "" "${multiValueArgs}")
    add_executable(${NAME} ${arg_SOURCES})

    if (EMSCRIPTEN)
        # Pack all dependencies into a *.data file                              
        foreach(ITEM ${arg_DEPENDENCIES})
            string(APPEND arg_EMSCRIPTEN_LINK_FLAGS
                " --preload-file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ITEM}.so@${ITEM}.so")
        endforeach()

        # attention: -fwasm-exception are currently supported in several
        # major web browsers, but may not be supported in all           
        # WebAssembly engines yet. Also -sASYNCIFY isn't compatible!    
        set_target_properties(${NAME} PROPERTIES
            COMPILE_FLAGS  "-sMAIN_MODULE -fwasm-exceptions ${arg_EMSCRIPTEN_COMPILE_FLAGS}"
            LINK_FLAGS     "-sMAIN_MODULE -sWASM=1 --emrun -sALLOW_MEMORY_GROWTH -fwasm-exceptions ${arg_EMSCRIPTEN_LINK_FLAGS}"
            SUFFIX         ".html"
        )

        # When building for emscripten, our shared libraries are "fake" 
        # and have to be linked in a specific way from wasm MAIN_MODULEs
        # - once by using the *.wasm file, and once by using the shared 
        # library target.                                               
        # Any *.so files on the other hand must be packed in a *.data   
        # file by using --preload-file with all the required mods, like 
        # so: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/LangulusMod*.so         
        foreach(ITEM ${arg_LIBRARIES})
            target_link_libraries(${NAME}
                PRIVATE $<IF:$<TARGET_EXISTS:${ITEM}>,$<TARGET_FILE:${ITEM}>, >	
                        ${ITEM}
            )
        endforeach()
    else()
        target_link_libraries(${NAME} PRIVATE ${arg_LIBRARIES})
    endif()

    foreach(ITEM ${arg_DEPENDENCIES})
        add_dependencies(${NAME} ${ITEM})
    endforeach()
endfunction()

# Create a test executable if tests are enabled                                 
function(add_langulus_test NAME)
    add_langulus_app(${NAME} ${ARGN})
    add_test(
        NAME				${NAME}
        COMMAND				${NAME}
        WORKING_DIRECTORY	${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endfunction()
