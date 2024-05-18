cmake_minimum_required(VERSION 3.20)

project(Telecaster LANGUAGES CXX VERSION 0.0.1 DESCRIPTION "A highly optimized SIMD CIDR utilities library for c++")

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(PUBLIC_HEADER_DIR "${PROJECT_SOURCE_DIR}/include")

file(GLOB_RECURSE SRC_FILES
    "${SRC_DIR}/**.cpp"
    "${SRC_DIR}/**.hpp"
)
file(GLOB_RECURSE PUBLIC_HEADER_FILES
    "${PUBLIC_HEADER_DIR}/**.hpp"
)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES} ${PUBLIC_HEADER_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

target_include_directories(${PROJECT_NAME} PRIVATE "${PUBLIC_HEADER_DIR}/telecaster/")
target_include_directories(${PROJECT_NAME} PUBLIC "${SRC_DIR}")

# public stuff
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADER_FILES}")
target_include_directories(${PROJECT_NAME} PUBLIC "${PUBLIC_HEADER_DIR}")

# Warnings as errors
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)

add_compile_definitions(${PROJECT_NAME} 
    "_CRT_SECURE_NO_WARNINGS"
    "NOMINMAX"
    "WIN32_LEAN_AND_MEAN"
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PUBLIC -mavx2)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
# we have to use avx here bc msvc "does not know" what sse2 is    
    target_compile_options(${PROJECT_NAME} PUBLIC /arch:AVX2)
endif()


# Optimizations
if(MSVC)    
    set(CMAKE_MODULE_LINKER_FLAGS
        "${CMAKE_MODULE_LINKER_FLAGS} /LTCG /OPT:REF,ICF /GUARD:NO")

    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Oi /Ot /Oy /Ob3 /sdl- /GL /GF /GS- /Gw /Qpar")
    string(REPLACE "/Ob1" "/Ob3" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif()