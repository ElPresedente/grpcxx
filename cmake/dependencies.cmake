include(FetchContent)

# libuv
find_package(libuv CONFIG REQUIRED)

# nghttp2
include(FindPkgConfig)
pkg_search_module(NGHTTP2 REQUIRED libnghttp2)
set(libnghttp2_lib ${NGHTTP2_LIBRARIES})
include_directories(SYSTEM PUBLIC ${NGHTTP2_INCLUDE_DIRS})

# protobuf
find_package(Protobuf 3.15.0 REQUIRED)

# fmt
find_package(fmt CONFIG REQUIRED)
