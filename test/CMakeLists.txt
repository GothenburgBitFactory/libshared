cmake_minimum_required (VERSION 2.8)

# See this CMake issue before complaining about the following.
# https://cmake.org/Bug/view.php?id=16062
if(POLICY CMP0037)
  cmake_policy(SET CMP0037 OLD)
endif()

include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/test
                     ${SHARED_INCLUDE_DIRS})

set (test_SRCS args.t autocomplete.t composite.t color.t shared.t configuration.t datetime.t duration.t format.t fs.t json.t json_test json2_test lexer.t list.t msg.t palette.t pig.t rx.t table.t timer.t tree.t unicode.t utf8.t)

add_custom_target (test ./run_all --verbose
                        DEPENDS ${test_SRCS}
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)

foreach (src_FILE ${test_SRCS})
  add_executable (${src_FILE} "${src_FILE}.cpp" test.cpp)
  target_link_libraries (${src_FILE} shared ${SHARED_LIBRARIES})
endforeach (src_FILE)

configure_file(run_all run_all COPYONLY)
configure_file(problems problems COPYONLY)

