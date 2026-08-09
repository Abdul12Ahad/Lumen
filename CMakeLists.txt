cmake_minimum_required(VERSION 3.20)
project(lumen CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lumen
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/ast/ast_printer.cpp
)

target_include_directories(lumen PRIVATE src)