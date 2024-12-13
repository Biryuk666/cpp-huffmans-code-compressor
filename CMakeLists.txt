cmake_minimum_required(VERSION 3.11)

project(HCC CXX)
set(CMAKE_CXX_STANDARD 17)

set(HCC_MAIN_FILES 
    	"huffmans-code-compressor/huffmans_code_compressor.h"
	"huffmans-code-compressor/huffmans_code_compressor.cpp"
	"huffmans-code-compressor/saveload.h")

if (CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

add_executable(huffmans_code_compressor "huffmans-code-compressor/main.cpp" ${HCC_MAIN_FILES})