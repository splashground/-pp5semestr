
cmake_minimum_required(VERSION 3.20)

project(Calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Папка с исходниками основного приложения и плагинов
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Основное консольное приложение
add_executable(Calculator
    "${SOURCE_DIR}/main.cpp"
    "${SOURCE_DIR}/parser.cpp"
    "${SOURCE_DIR}/plugin_loader.cpp"
)

target_include_directories(Calculator PRIVATE
    "${SOURCE_DIR}"
)

# Папка для готовых файлов
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")

set_target_properties(Calculator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)

# Функция создания DLL-плагина
function(add_calc_plugin target_name source_file)
    add_library(${target_name} SHARED
        "${SOURCE_DIR}/${source_file}"
    )

    target_include_directories(${target_name} PRIVATE
        "${SOURCE_DIR}"
    )

    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}/plugins"
        LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}/plugins"
    )
endfunction()

# Математические плагины
add_calc_plugin(funcsin plugin_sin.cpp)
add_calc_plugin(funcln plugin_ln.cpp)
add_calc_plugin(funcsquare plugin_square.cpp)

# Гарантируем, что DLL будут собраны вместе с приложением
add_dependencies(Calculator
    funcsin
    funcln
    funcsquare
)

message(STATUS "Calculator CMake configuration loaded")
