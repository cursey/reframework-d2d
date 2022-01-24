# This file is automatically generated from cmake.toml - DO NOT EDIT
# See https://github.com/build-cpp/cmkr for more information

cmake_minimum_required(VERSION 3.15)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-tree builds are not supported. Run CMake from a separate directory: cmake -B build")
endif()

# Regenerate CMakeLists.txt automatically in the root project
set(CMKR_ROOT_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	set(CMKR_ROOT_PROJECT ON)

	# Bootstrap cmkr
	include(cmkr.cmake OPTIONAL RESULT_VARIABLE CMKR_INCLUDE_RESULT)
	if(CMKR_INCLUDE_RESULT)
		cmkr()
	endif()

	# Enable folder support
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# Create a configure-time dependency on cmake.toml to improve IDE support
if(CMKR_ROOT_PROJECT)
	configure_file(cmake.toml cmake.toml COPYONLY)
endif()

project(reframework-d2d)

set(VCPKG_TARGET_TRIPLET x64-windows-static)

if(CMKR_ROOT_PROJECT AND NOT CMKR_DISABLE_VCPKG)
	include(FetchContent)
	message(STATUS "Fetching vcpkg...")
	FetchContent_Declare(vcpkg URL "https://github.com/microsoft/vcpkg/archive/refs/tags/2022.01.01.tar.gz")
	FetchContent_MakeAvailable(vcpkg)
	include("${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake")
endif()

# Packages
find_package(sol2 REQUIRED CONFIG)

find_package(Lua REQUIRED)

find_package(utf8cpp REQUIRED CONFIG)

# Target reframework
set(CMKR_TARGET reframework)
set(reframework_SOURCES "")

set(CMKR_SOURCES ${reframework_SOURCES})
add_library(reframework INTERFACE)

if(reframework_SOURCES)
	target_sources(reframework INTERFACE ${reframework_SOURCES})
endif()

target_include_directories(reframework INTERFACE
	"deps/reframework/include"
)

unset(CMKR_TARGET)
unset(CMKR_SOURCES)

# Target reframework-d2d
set(CMKR_TARGET reframework-d2d)
set(reframework-d2d_SOURCES "")

list(APPEND reframework-d2d_SOURCES
	"src/D2DRenderer.cpp"
	"src/D3D12Renderer.cpp"
	"src/Plugin.cpp"
	"src/D2DRenderer.hpp"
	"src/D3D12Renderer.hpp"
	"src/D3D12Shaders.hpp"
)

list(APPEND reframework-d2d_SOURCES
	cmake.toml
)

set(CMKR_SOURCES ${reframework-d2d_SOURCES})
add_library(reframework-d2d SHARED)

if(reframework-d2d_SOURCES)
	target_sources(reframework-d2d PRIVATE ${reframework-d2d_SOURCES})
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${reframework-d2d_SOURCES})

target_compile_features(reframework-d2d PUBLIC
	cxx_std_20
)

target_include_directories(reframework-d2d PUBLIC
	${LUA_INCLUDE_DIR}
)

target_link_libraries(reframework-d2d PUBLIC
	sol2::sol2
	${LUA_LIBRARIES}
	utf8cpp
	reframework
	d2d1
	dwrite
	d3d11
	d3d12
	dxgi
	d3dcompiler
)

unset(CMKR_TARGET)
unset(CMKR_SOURCES)
