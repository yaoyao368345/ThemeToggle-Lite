cmake_minimum_required(VERSION 3.20)

project(ThemeToggle VERSION 0.1.0 LANGUAGES CXX RC)

add_executable(ThemeToggle WIN32
    src/main.cpp
    src/theme.cpp
    src/tray.cpp
    resources/resource.rc
)

target_compile_features(ThemeToggle PRIVATE cxx_std_20)
target_compile_definitions(ThemeToggle PRIVATE UNICODE _UNICODE)
target_include_directories(ThemeToggle PRIVATE resources)
target_link_libraries(ThemeToggle PRIVATE user32 shell32 advapi32)
