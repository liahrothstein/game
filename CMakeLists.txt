cmake_minimum_required(VERSION 3.24)
project(KostiIPepel CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(game
  src/main.cpp
  src/render/Shader.cpp
)
target_link_libraries(game PRIVATE SDL3::SDL3 glm::glm glad::glad opengl32)