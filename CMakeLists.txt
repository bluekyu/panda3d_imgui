cmake_minimum_required(VERSION 3.12)
project(panda3d_imgui
    LANGUAGES CXX
)

# === configure ====================================================================================
option(${PROJECT_NAME}_ENABLE_RTTI "Enable Run-Time Type Information" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)    # Project Grouping

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(WIN32)
    include(windows_utils)
endif()

add_subdirectory("sample")
# ==================================================================================================

# === install ======================================================================================
# ==================================================================================================
