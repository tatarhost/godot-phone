cmake_minimum_required(VERSION 3.22.1)
project(neurogenesis LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET3 OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(INSTALL_LIBS OFF CACHE BOOL "" FORCE)
include(FetchContent)
FetchContent_Declare(bullet
  GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
  GIT_TAG 3.25
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(bullet)

add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)

add_library(neurogenesis SHARED main.cpp)
target_include_directories(neurogenesis PRIVATE ${bullet_SOURCE_DIR}/src)
target_link_libraries(neurogenesis PRIVATE native_app_glue BulletDynamics BulletCollision LinearMath android log EGL GLESv3)
