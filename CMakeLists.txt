cmake_minimum_required(VERSION 3.16)
project(machikado-cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(ExternalProject)

set(SODIUM_SOURCE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/external/libsodium")
set(SODIUM_INSTALL_DIR "${CMAKE_BINARY_DIR}/sodium-install")

if(NOT EXISTS "${SODIUM_SOURCE_DIR}/configure")
    message(FATAL_ERROR
        "\nlibsodium submodule not found or not on stable branch.\n"
        "Run: git submodule update --init --recursive\n")
endif()

file(MAKE_DIRECTORY "${SODIUM_INSTALL_DIR}/include")
file(MAKE_DIRECTORY "${SODIUM_INSTALL_DIR}/lib")

ExternalProject_Add(sodium_build
    SOURCE_DIR        "${SODIUM_SOURCE_DIR}"
    BUILD_IN_SOURCE   1
    CONFIGURE_COMMAND <SOURCE_DIR>/configure
                      --prefix=<INSTALL_DIR>
                      --enable-static
                      --disable-shared
                      --with-pic
                      --disable-dependency-tracking
                      --silent
    BUILD_COMMAND     make -j4
    INSTALL_COMMAND   make install
    INSTALL_DIR       "${SODIUM_INSTALL_DIR}"
)

add_library(sodium_static STATIC IMPORTED GLOBAL)
set_target_properties(sodium_static PROPERTIES
    IMPORTED_LOCATION             "${SODIUM_INSTALL_DIR}/lib/libsodium.a"
    INTERFACE_INCLUDE_DIRECTORIES "${SODIUM_INSTALL_DIR}/include"
)
add_dependencies(sodium_static sodium_build)

message(STATUS "libsodium source:  ${SODIUM_SOURCE_DIR}")
message(STATUS "libsodium install: ${SODIUM_INSTALL_DIR}")

add_library(machikado STATIC
    src/key.cpp
    src/sign.cpp
    src/utils.cpp
)

target_include_directories(machikado
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes
)
target_link_libraries(machikado PUBLIC sodium_static)

add_library(machikado_check OBJECT
    src/key.cpp
    src/sign.cpp
    src/utils.cpp
)
target_include_directories(machikado_check
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes
)
target_link_libraries(machikado_check PRIVATE sodium_static)
target_compile_options(machikado_check PRIVATE
    -fsyntax-only -Wall -Wextra -Wpedantic -Wno-unused-parameter
)

add_custom_target(check
    DEPENDS machikado_check
    COMMENT "syntax check passed"
)

option(MACHIKADO_BUILD_EXAMPLES "Build examples" OFF)

if(MACHIKADO_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/main.cpp")
    add_executable(machikado_example examples/main.cpp)
    target_link_libraries(machikado_example PRIVATE machikado)
endif()
