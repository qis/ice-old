cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(ice VERSION 0.4.0 LANGUAGES CXX)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.hpp)
file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.hpp src/*.cpp)

add_library(ice STATIC ${headers} ${sources})
target_compile_features(ice PRIVATE cxx_std_20)
target_include_directories(ice PRIVATE src PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

if(WIN32)
  target_compile_definitions(ice PRIVATE _UNICODE UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.hpp")
install(TARGETS ice EXPORT ice LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(EXPORT ice FILE ice-config.cmake NAMESPACE ice:: DESTINATION lib/cmake/ice)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/ice-config-version.cmake
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ice-config-version.cmake DESTINATION lib/cmake/ice)

add_library(ice::ice ALIAS ice)
