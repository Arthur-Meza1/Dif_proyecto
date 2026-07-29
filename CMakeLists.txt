cmake_minimum_required(VERSION 3.14)
project(ClothSimulator LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencias del sistema -----------------------------------------------
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)

# --- GLAD -------------------------------------------------------------------
find_package(glad QUIET)

if(glad_FOUND)
    message(STATUS "Usando GLAD del sistema")
    set(GLAD_TARGET glad::glad)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glad.c")
    message(STATUS "Usando GLAD local (glad.c)")
    list(APPEND SOURCES_GLAD glad.c)
    set(GLAD_TARGET "")
else()
    include(FetchContent)
    FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG v2.0.8
    )
    set(GLAD_PROFILE "core" CACHE INTERNAL "")
    set(GLAD_API "gl=3.3" CACHE INTERNAL "")
    set(GLAD_GENERATOR "c" CACHE INTERNAL "")
    FetchContent_MakeAvailable(glad)
    set(GLAD_TARGET glad)
endif()

# --- Fuentes ----------------------------------------------------------------
set(SOURCES
    src/main.cpp
    src/cloth.cpp
    src/shader.cpp
    src/camera.cpp
    src/text.cpp
    ${SOURCES_GLAD}
)

add_executable(cloth_sim ${SOURCES})

target_include_directories(cloth_sim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(cloth_sim PRIVATE
    glfw
    glm::glm
    OpenGL::GL
    ${GLAD_TARGET}
)
