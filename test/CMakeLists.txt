cmake_minimum_required (VERSION 2.8)
if(POLICY CMP0037)
  cmake_policy(SET CMP0037 OLD)
endif()

include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/test
                     ${COMMON_INCLUDE_DIRS})

set (test_SRCS args.t autocomplete.t color.t common.t configuration.t datetime.t format.t fs.t list.t msg.t pig.t rx.t table.t unicode.t utf8.t)

add_custom_target (test ./run_all --verbose
                        DEPENDS ${test_SRCS}
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)

foreach (src_FILE ${test_SRCS})
  add_executable (${src_FILE} "${src_FILE}.cpp" test.cpp)
  target_link_libraries (${src_FILE} common ${COMMON_LIBRARIES})
endforeach (src_FILE)

configure_file(run_all run_all COPYONLY)
configure_file(problems problems COPYONLY)

