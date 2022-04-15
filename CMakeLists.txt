cmake_minimum_required(VERSION 3.5)

project(D3D)

# executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
# module libraries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# static libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(D3D)
add_subdirectory(Shapes)

target_compile_features(D3D
    PUBLIC cxx_std_17
)
