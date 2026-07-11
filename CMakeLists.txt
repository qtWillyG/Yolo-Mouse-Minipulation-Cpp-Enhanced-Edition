cmake_minimum_required(VERSION 3.20)

project(yolo_mouse VERSION 2.0.0 LANGUAGES CXX)

add_executable(yolo-mouse WIN32 src/main.cpp)
target_compile_features(yolo-mouse PRIVATE cxx_std_20)
target_link_libraries(yolo-mouse PRIVATE user32 comctl32 gdi32)

if(MSVC)
  target_compile_options(yolo-mouse PRIVATE /W4 /permissive-)
else()
  target_compile_options(yolo-mouse PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS yolo-mouse RUNTIME DESTINATION bin)
