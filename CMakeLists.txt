cmake_minimum_required(VERSION 3.16)
project(KatCompiler VERSION 1.0)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Compiler executable
add_executable(kat_compiler 
    src/main.cpp
)

# Specify include directories (if any)
target_include_directories(kat_compiler PRIVATE include)

# Add .kat file as argument when running the compiler
add_custom_target(run
    COMMAND kat_compiler ${KAT_FILE}
    DEPENDS kat_compiler
    COMMENT "Running Kat Compiler"
)
