cmake_minimum_required(VERSION 3.25)
project(reframework-d2d)
include (cmake/CPM.cmake)

CPMAddPackage("gh:nemtrif/utfcpp@4.0.5")
CPMAddPackage("gh:ThePhD/sol2@3.3.0")
CPMAddPackage(
    NAME lua
    GITHUB_REPOSITORY lua/lua
    VERSION 5.4.6
    DOWNLOAD_ONLY YES
)

if (lua_ADDED)
    FILE(GLOB lua_sources ${lua_SOURCE_DIR}/*.c)
    list(REMOVE_ITEM lua_sources "${lua_SOURCE_DIR}/lua.c" "${lua_SOURCE_DIR}/luac.c" "${lua_SOURCE_DIR}/onelua.c")
    add_library(lua STATIC ${lua_sources})

    target_include_directories(lua
        PUBLIC
        $<BUILD_INTERFACE:${lua_SOURCE_DIR}>
    )
endif()


add_library(reframework-d2d SHARED
    src/D2DFont.cpp
    src/D2DImage.cpp
    src/D2DPainter.cpp
    src/D3D12Renderer.cpp
    src/DrawList.cpp
    src/Plugin.cpp
    src/D3D12CommandContext.cpp
    src/D2DCommand.cpp
)
target_include_directories(reframework-d2d PRIVATE 
    src
    deps/reframework/include
)
target_compile_features(reframework-d2d PRIVATE cxx_std_20)
target_link_libraries(reframework-d2d PRIVATE utf8cpp sol2::sol2 lua d2d1 dwrite d3d11 d3d12 dxgi d3dcompiler)

install(
    TARGETS reframework-d2d
    DESTINATION bin
    COMPONENT reframework-d2d
)
