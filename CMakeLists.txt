cmake_minimum_required(VERSION 3.24)
project(simulates VERSION 0.1.0 LANGUAGES CXX)

add_library(simulates INTERFACE)
add_library(simulates::simulates ALIAS simulates)
target_include_directories(simulates INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_compile_features(simulates INTERFACE cxx_std_23)

if(PROJECT_IS_TOP_LEVEL)
  add_executable(simulates_examples examples/main.cpp examples/life.cpp examples/particles.cpp examples/civilisation.cpp)
  target_link_libraries(simulates_examples PRIVATE simulates)
endif()

include(GNUInstallDirs)
install(TARGETS simulates EXPORT simulates-config)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT simulates-config NAMESPACE simulates:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/simulates)
