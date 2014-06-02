cmake_minimum_required (VERSION 2.6)
project(Tutte CXX)
set(Tutte_VERSION_MAJOR 1)
set(Tutte_VERSION_MINOR 3)

include_directories("${PROJECT_SOURCE_DIR}/include")

find_library(LIBGMP gmp REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(tutte src/main.cpp src/parse_graph.cpp)
set_target_properties(tutte PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -pedantic -O3")
target_link_libraries(tutte ${Boost_LIBRARIES} ${LIBGMP})
install(TARGETS tutte DESTINATION bin)

# Testing

enable_testing()

macro(do_test arg)
  add_test(test_${arg} tutte --input-file ${PROJECT_SOURCE_DIR}/tests/${arg}.input 2>/dev/null | diff - ${PROJECT_SOURCE_DIR}/tests/${arg}.output)
endmacro(do_test)

do_test(square4x4ff)
do_test(tutte_Gppp_N=1_L=4_M=1)
do_test(tutte_Gppp_N=1_L=4_M=2)
do_test(tutte_Gppp_N=2_L=4_M=1)
do_test(tutte_Gppp_N=2_L=4_M=2)

# http://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
# http://public.kitware.com/Bug/view.php?id=8774
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS tutte)

# Packaging

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "computes the Tutte Polynomial")
set(CPACK_PACKAGE_CONTACT "Andrea Bedini <andrea.bedini@gmail.com>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "${Tutte_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Tutte_VERSION_MINOR}")
include(CPack)
