cmake_minimum_required(VERSION 3.21)
project(LEMON_ASIO_USB_BRIDGE VERSION 0.1.3 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "LEMON ASIO currently targets Windows 10/11 only.")
endif()

add_executable(LEMON_ASIO WIN32
    src/main.cpp
    src/audio_devices.cpp
)

target_compile_features(LEMON_ASIO PRIVATE cxx_std_17)
target_compile_definitions(LEMON_ASIO PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(LEMON_ASIO PRIVATE ole32 uuid propsys comctl32)

if(MSVC)
    target_compile_options(LEMON_ASIO PRIVATE /W4 /permissive- /EHsc)
    set_property(TARGET LEMON_ASIO PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
