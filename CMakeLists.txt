cmake_minimum_required(VERSION 3.16)

project(CompilerCpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)
find_package(Java REQUIRED)

# ANTLR runtime FIRST
add_subdirectory(antlr4/runtime/Cpp)

set(ANTLR_JAR
        ${CMAKE_SOURCE_DIR}/tools/antlr-4.13.1-complete.jar
)

set(ANTLR_OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${ANTLR_OUTPUT_DIR})

set(ANTLR_GENERATED_SRC
        ${ANTLR_OUTPUT_DIR}/CppLexer.cpp
        ${ANTLR_OUTPUT_DIR}/CppParser.cpp
        ${ANTLR_OUTPUT_DIR}/CppBaseVisitor.cpp
        ${ANTLR_OUTPUT_DIR}/CppVisitor.cpp
        ${ANTLR_OUTPUT_DIR}/CppBaseListener.cpp
        ${ANTLR_OUTPUT_DIR}/CppListener.cpp
)

add_custom_command(
        OUTPUT ${ANTLR_GENERATED_SRC}

        COMMAND
        ${Java_JAVA_EXECUTABLE}
        -jar ${ANTLR_JAR}
        -Dlanguage=Cpp
        -visitor
        -listener
        -o ${ANTLR_OUTPUT_DIR}
        ${CMAKE_SOURCE_DIR}/src/C++00/grammar/Cpp.g4

        DEPENDS
        ${CMAKE_SOURCE_DIR}/src/C++00/grammar/Cpp.g4
)

add_custom_target(
        GenerateGrammar
        DEPENDS ${ANTLR_GENERATED_SRC}
)

# ---------------------------
# EXECUTABLE FIRST
# ---------------------------
add_executable(AntlrQtEditor
        src/C++00/main.cpp
        src/Qt/mainwindow.cpp
        src/Qt/mainwindow.h
        src/Qt/mainwindow.ui
        ${ANTLR_GENERATED_SRC}
)

add_dependencies(AntlrQtEditor GenerateGrammar)

target_include_directories(AntlrQtEditor PRIVATE
        ${ANTLR_OUTPUT_DIR}
        antlr4/runtime/Cpp/runtime/src
)

target_link_libraries(AntlrQtEditor PRIVATE
        Qt6::Widgets
        Qt6::Concurrent
        antlr4_shared
)

# ---------------------------
# ONLY AFTER TARGET EXISTS
# ---------------------------
target_compile_definitions(AntlrQtEditor PRIVATE
        ANTLR4CPP_USING_ABSEIL=0
)

set_target_properties(AntlrQtEditor PROPERTIES
        CXX_STANDARD 17
)