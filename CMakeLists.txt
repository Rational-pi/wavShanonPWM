cmake_minimum_required(VERSION 2.8)
project(easpeakMouth)

set(CMAKE_BUILD_TYPE
    Debug
   # Release
)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
############################################################################
#resources copying
file(COPY ${CMAKE_SOURCE_DIR}/res DESTINATION ${CMAKE_BINARY_DIR})
############################################################################
#Qt creator specific (enable to brows src and include directoryes)
FILE(GLOB_RECURSE SrcFiles "src/*")
FILE(GLOB_RECURSE Heders "include/*")
INCLUDE_DIRECTORIES(include)
############################################################################
add_executable(${PROJECT_NAME} ${SrcFiles}  ${Heders})
target_include_directories(${PROJECT_NAME} PRIVATE ${TARGET_INCLUDE_DIRS} )
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
