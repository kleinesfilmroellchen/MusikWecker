project(tiff_test)

cmake_minimum_required(VERSION 3.25)

include_directories(${CMAKE_SOURCE_DIR}/..)

set(SOURCES
	${SOURCES}
	main.cpp
	${CMAKE_SOURCE_DIR}/../SRLV.cpp
)

add_compile_definitions(__LINUX__)

add_executable(tiff_test ${SOURCES})
set_property(TARGET tiff_test PROPERTY CXX_STANDARD 20)
