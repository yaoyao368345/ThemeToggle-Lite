cmake_minimum_required(VERSION 3.20)

project(ThemeToggle VERSION 0.1.0 LANGUAGES CXX)

add_executable(ThemeToggle WIN32
    src/main.cpp
    src/theme.cpp
)

target_compile_features(ThemeToggle PRIVATE cxx_std_20)
target_compile_definitions(ThemeToggle PRIVATE UNICODE _UNICODE)
target_link_libraries(ThemeToggle PRIVATE user32 advapi32)
