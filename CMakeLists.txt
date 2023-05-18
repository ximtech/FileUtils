cmake_minimum_required(VERSION 3.20)
project(FileUtils VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include(cmake/CPM.cmake)

CPMAddPackage(
        NAME CRC
        GITHUB_REPOSITORY ximtech/CRC
        GIT_TAG origin/main)

CPMAddPackage(
        NAME BufferString
        GITHUB_REPOSITORY ximtech/BufferString
        GIT_TAG origin/main
        OPTIONS
        "ENABLE_FLOAT_FORMATTING ON"
)

CPMAddPackage(
        NAME Collections
        GITHUB_REPOSITORY ximtech/Collections
        GIT_TAG origin/main)

set(SOURCE_FILES
        FileUtils.c
        include/FileUtils.h)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(${PROJECT_NAME} CRC)
target_link_libraries(${PROJECT_NAME} BufferString)
target_link_libraries(${PROJECT_NAME} Collections)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Export
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME})

install(EXPORT ${PROJECT_NAME}Export
        FILE ${PROJECT_NAME}-config.cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME})