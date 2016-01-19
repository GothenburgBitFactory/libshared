cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src)

set (common_HEADERS Color.h
                    FS.h
                    RX.h
                    Table.h
                    Timer.h
                    text.h
                    unicode.h
                    utf8.h)

set (common_SRCS Color.cpp
                 FS.cpp
                 RX.cpp
                 Table.cpp
                 Timer.cpp
                 text.cpp
                 unicode.cpp
                 utf8.cpp
                 wcwidth6.cpp
                 ${common_HEADERS})

add_library (common STATIC ${common_SRCS})

set (CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")
install (TARGETS common DESTINATION lib)
install (FILES ${common_HEADERS} DESTINATION include)
