cmake_minimum_required(VERSION 3.23)
if(NOT PROJECT_NAME AND NOT PROJECT_IS_TOP_LEVEL)
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
	message("Toolchain=${CMAKE_TOOLCHAIN_FILE}")
endif()
project(grpcxx VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(cmake/dependencies.cmake)

add_subdirectory(lib)
add_subdirectory(src)
