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

set (test_SRCS args.t autocomplete.t charliteral.t composite.t color.t configuration.t dates.t datetime.t duration.t external.t format.t fs.t intrinsic.t json.t json_test lexer.t list.t msg.t negative.t palette.t peg.t pig.t plus.t positive.t question.t rx.t sax_test shared.t star.t stringliteral.t table.t timer.t tree.t unicode.t utf8.t)

add_custom_target (test ./run_all --verbose
                        DEPENDS ${test_SRCS}
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)

foreach (src_FILE ${test_SRCS})
  add_executable (${src_FILE} "${src_FILE}.cpp" test.cpp)
  target_link_libraries (${src_FILE} shared ${SHARED_LIBRARIES})
endforeach (src_FILE)

configure_file(run_all run_all COPYONLY)
configure_file(problems problems COPYONLY)

