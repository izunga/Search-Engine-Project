cmake_minimum_required(VERSION 3.10)
project(Project4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add RapidJSON
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/rapidjson)

set(SOURCES
        main.cpp
        searchEngine.cpp
)

set(HEADERS
        avl_tree.h
        document_info.h
        searchEngine.h
        text_processor.h
)

add_executable(supersearch ${SOURCES} ${HEADERS})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(supersearch PRIVATE -Wall -Wextra)
endif()