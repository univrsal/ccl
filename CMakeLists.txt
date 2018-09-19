cmake_minimum_required(VERSION 2.8)
project(ccl)

set(ccl_test_SOURCES main.cpp
	ccl.cpp
	ccl.hpp)

add_executable(ccl_test ${ccl_test_SOURCES})
