cmake_minimum_required(VERSION 2.8)

find_package(Boost REQUIRED COMPONENTS thread)

include(CheckIncludeFile)


set(CMAKE_CXX_FLAGS "-Wno-c++11-extensions")
check_include_file(GL/glfw.h HAVE_GLFW_H)
if(NOT HAVE_GLFW_H)
    message(FATAL_ERROR "glfw.h not found")
endif()
check_include_file(GL/glew.h HAVE_GLEW_H)
if(NOT HAVE_GLEW_H)
    message(FATAL_ERROR "glew.h not found")
endif()

find_package(OpenGL REQUIRED)
include_directories(
    "."
    ${OPENGL_INCLUDE_PATH}
    ${Boost_INCLUDE_DIRS}
)

add_executable(
    fonttest
    TTF.cpp
)
target_link_libraries(fonttest
    ${OPENGL_LIBRARIES}
    ${Boost_LIBRARIES}
    glfw
    glew
)
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        find_library(COCOA_LIBRARY Cocoa)
        find_library(APPLICATION_SERVICES_LIBRARY ApplicationServices)
        find_library(AGL_LIBRARY AGL)
        IF(COCOA_LIBRARY)
            target_link_libraries(fonttest ${COCOA_LIBRARY} ${APPLICATION_SERVICES_LIBRARY} ${AGL_LIBRARY})
        ELSE(COCOA_LIBRARY)
        ENDIF(COCOA_LIBRARY)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
add_custom_command(TARGET fonttest POST_BUILD COMMAND cp fonttest ..)
