cmake_minimum_required(VERSION 2.8)
project(ModelViewer)
add_compile_options(-std=c++11)

find_package(OpenCV REQUIRED)
find_package(assimp REQUIRED)
find_package(glm REQUIRED)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
)

aux_source_directory(${PROJECT_SOURCE_DIR}/src LIB_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/example EXAMPLE_SRC)

add_executable(${PROJECT_NAME}
    ${LIB_SRC}
    ${EXAMPLE_SRC}
)

set(GL_LIBS glut GL GLU GLEW)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/examples)
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${ASSIMP_LIBRARIES}
    ${GL_LIBS}
)