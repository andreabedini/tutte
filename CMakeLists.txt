cmake_minimum_required (VERSION 2.6)
project (Tutte)
include_directories("${PROJECT_SOURCE_DIR}/include")
add_executable(tutte src/main.cpp src/parse_graph.cpp)
set_target_properties(tutte PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pedantic -O3")
target_link_libraries(tutte gmp boost_program_options)
install (TARGETS tutte DESTINATION bin)

enable_testing()

macro (do_test arg)
  add_test (NAME test_${arg} COMMAND tutte --input-file ${PROJECT_SOURCE_DIR}/tests/${arg}.input 2>/dev/null | diff - ${PROJECT_SOURCE_DIR}/tests/${arg}.output)
endmacro (do_test)

do_test(square4x4ff)
do_test(tutte_Gppp_N=1_L=4_M=1)
do_test(tutte_Gppp_N=1_L=4_M=2)
do_test(tutte_Gppp_N=2_L=4_M=1)
do_test(tutte_Gppp_N=2_L=4_M=2)
