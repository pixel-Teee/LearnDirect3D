cmake_minimum_required(VERSION 3.5)

project(LearnDirect3D)

# executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
# module libraries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# static libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# CMAKE_CURRENT_SOURCE_DIR : current being processed cmakelists.txt's directory
set(Src_Root_Path ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE Source_List
    LIST_DIRECTORIES FALSE
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${Src_Root_Path}/LearnDirect3D/*.cpp
    ${Src_Root_Path}/LearnDirect3D/*.h
)

#foreach(Item IN ITEMS ${Source_List})
#    message(${Item})
#endforeach()

add_executable(LearnDirect3D ${Source_List})

foreach(Item IN ITEMS ${Source_List})
    # get the Item's directory path, don't include the file name
    get_filename_component(Soure_Path ${Item} PATH)    
    # replace source path's / to \\, output into the Source_Path_MSVC
    string(REPLACE "/" "\\" Source_Path_MSVC ${Soure_Path})
    # first argument describle the group name
    # Item will place into the group
    source_group(${Source_Path_MSVC} FILES ${Item})

endforeach()

target_compile_features(LearnDirect3D
    PUBLIC cxx_std_17
)
