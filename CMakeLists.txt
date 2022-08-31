project(lsqueue)

cmake_minimum_required(VERSION 3.10)

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0 -g -mcx16)

aux_source_directory(sample SRC_SAMPLE)

aux_source_directory(src SRC)

include_directories(include)

add_executable(testcase ${SRC_SAMPLE})

target_link_libraries(testcase lsexception lstime lslogger lsspinlock pthread)

install(DIRECTORY include/ls DESTINATION include)
