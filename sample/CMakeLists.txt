cmake_minimum_required(VERSION 3.12)
project(panda3d_imgui_sample
    DESCRIPTION "Empty example without Render Pipeline C++"
    LANGUAGES CXX
)

# === configure ====================================================================================
set_property(GLOBAL PROPERTY USE_FOLDERS ON)    # Project Grouping

# === project specific packages ===
find_package(panda3d REQUIRED p3framework p3direct)

find_package(imgui CONFIG REQUIRED)
set_target_properties(imgui::imgui PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
# ==================================================================================================

# === sources ======================================================================================
# set input files
set(sources_panda3d_imgui_files
    "${PROJECT_SOURCE_DIR}/../panda3d_imgui/panda3d_imgui.cpp"
    "${PROJECT_SOURCE_DIR}/../panda3d_imgui/panda3d_imgui.hpp"
)

set(sources_files
    "${PROJECT_SOURCE_DIR}/main.cpp"
)

# grouping
source_group("panda3d_imgui" FILES ${sources_panda3d_imgui_files})
# ==================================================================================================

# === target =======================================================================================
add_executable(${PROJECT_NAME} ${sources_files} ${sources_panda3d_imgui_files})

if(MSVC)
    windows_add_longpath_manifest(${PROJECT_NAME})
    target_compile_options(${PROJECT_NAME} PRIVATE /MP /wd4251 /utf-8
        $<$<VERSION_GREATER:${MSVC_VERSION},1900>:/permissive->
        $<$<NOT:$<BOOL:${rpcpp_samples_project_ENABLE_RTTI}>>:/GR->

        # note: windows.cmake in vcpkg
        $<$<CONFIG:Release>:/Oi /Gy /Z7>
    )
    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS_RELWITHDEBINFO    " /INCREMENTAL:NO /OPT:REF /OPT:ICF ")
    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS_RELEASE           " /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF ")
else()
    target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall
        $<$<NOT:$<BOOL:${rpcpp_samples_project_ENABLE_RTTI}>>:-fno-rtti>
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../panda3d_imgui")

target_link_libraries(${PROJECT_NAME}
    PRIVATE panda3d::p3framework panda3d::p3direct imgui::imgui
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "panda3d_imgui")
# ==================================================================================================

# === install ======================================================================================
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/../panda3d_imgui/shader" DESTINATION "bin")
# ==================================================================================================
