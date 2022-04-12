cmake_minimum_required(VERSION 3.16) 
project (ZIP_PROJECT) 

set(CMAKE_CXX_STANDARD 20)

aux_source_directory(./src DIR_SRCS) 

include_directories(include)
add_executable(a.out ${DIR_SRCS})