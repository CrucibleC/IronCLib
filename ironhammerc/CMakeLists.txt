cmake_minimum_required(VERSION 3.20)
project(PortabilityTest C)

# -----------------------------
# Build matrix inputs (set by run_all.py, defaults for plain builds)
# -----------------------------
set(C_STD "c11" CACHE STRING "C standard: c99 or c11")
set(OPT_LEVEL "-O0" CACHE STRING "Optimization level: -O0 or -O2")
set(COMPILER_NAME "" CACHE STRING "Expected compiler: gcc, clang or msvc (optional)")

# Fail if the compiler is not the one the run is labelled with
set(_expected_ids_gcc "GNU")
set(_expected_ids_clang "Clang")
set(_expected_ids_msvc "MSVC")
if (COMPILER_NAME AND NOT CMAKE_C_COMPILER_ID STREQUAL "${_expected_ids_${COMPILER_NAME}}")
    message(FATAL_ERROR "COMPILER_NAME=${COMPILER_NAME} but the compiler is ${CMAKE_C_COMPILER_ID}")
endif()

# C standard: strict ISO mode (-std=c99, not -std=gnu99)
if (C_STD STREQUAL "c99")
    set(_std_version 199901L)
    if (MSVC)
        message(FATAL_ERROR "MSVC has no C99 mode; use C_STD=c11")
    endif()
elseif (C_STD STREQUAL "c11")
    set(_std_version 201112L)
else()
    message(FATAL_ERROR "Unsupported C_STD '${C_STD}' (expected c99 or c11)")
endif()

string(REGEX REPLACE "^c" "" CMAKE_C_STANDARD "${C_STD}")
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Optimization: OPT_LEVEL is the only source, so drop the per-config defaults
# (e.g. Release's /O2 /DNDEBUG) and keep asserts on in every build
foreach (_config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_C_FLAGS_${_config} "")
endforeach()

if (OPT_LEVEL STREQUAL "-O0")
    set(_optimized 0)
    set(_msvc_opt /Od)
elseif (OPT_LEVEL STREQUAL "-O2")
    set(_optimized 1)
    set(_msvc_opt /O2)
else()
    message(FATAL_ERROR "Unsupported OPT_LEVEL '${OPT_LEVEL}' (expected -O0 or -O2)")
endif()

add_executable(hammer_ironclib 
    main.c
    tests/inline_func_wrapper_a.c
    tests/inline_func_wrapper_b.c
    tests/opaque_structs.c
    tests/co_job_test_funcs.c
)

target_include_directories(hammer_ironclib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../ironclib
)

# main.c checks these against __STDC_VERSION__ / __OPTIMIZE__ and fails the build on mismatch
target_compile_definitions(hammer_ironclib PRIVATE
    IHC_EXPECTED_STDC_VERSION=${_std_version}
    IHC_EXPECTED_OPTIMIZED=${_optimized}
)

# Strict ISO mode hides POSIX declarations (e.g. nanosleep) used by the pthread backend
if (NOT WIN32)
    target_compile_definitions(hammer_ironclib PRIVATE _POSIX_C_SOURCE=200809L)
endif()

if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(hammer_ironclib PRIVATE
        ${OPT_LEVEL}
        -Wall
        -Wextra
        -Wpedantic
        -Werror

        # macro-generated static_assert typedef noise ----
        -Wno-unused-local-typedefs
    )
endif()

if (MSVC)
    target_compile_options(hammer_ironclib PRIVATE
        ${_msvc_opt}
        /W4
        /WX
        /wd4127  # macro pattern (do { } while(0))
    )
endif()