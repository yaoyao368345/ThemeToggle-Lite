cmake_minimum_required(VERSION 3.20)

project(ThemeToggle VERSION 0.3.0 LANGUAGES CXX RC)

configure_file(resources/version.h.in generated/version.h @ONLY)

add_executable(ThemeToggle WIN32
    src/main.cpp
    src/theme.cpp
    src/tray.cpp
    src/startup.cpp
    resources/resource.rc
)

target_compile_features(ThemeToggle PRIVATE cxx_std_20)
target_compile_definitions(ThemeToggle PRIVATE UNICODE _UNICODE)
target_compile_options(ThemeToggle PRIVATE
    "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8>"
)
target_include_directories(ThemeToggle PRIVATE resources "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(ThemeToggle PRIVATE user32 shell32 advapi32)
