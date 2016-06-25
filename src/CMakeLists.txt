cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src)

set (shared_HEADERS Args.h
                    Color.h
                    Composite.h
                    Configuration.h
                    Datetime.h
                    Duration.h
                    FS.h
                    JSON.h
                    JSON2.h
                    Lexer.h
                    Log.h
                    Msg.h
                    Palette.h
                    Pig.h
                    RX.h
                    Table.h
                    Timer.h
                    shared.h
                    format.h
                    unicode.h
                    utf8.h)

set (shared_SRCS Args.cpp
                 Color.cpp
                 Composite.cpp
                 Configuration.cpp
                 Datetime.cpp
                 Duration.cpp
                 FS.cpp
                 JSON.cpp
                 JSON2.cpp
                 Lexer.cpp
                 Log.cpp
                 Msg.cpp
                 Palette.cpp
                 Pig.cpp
                 RX.cpp
                 Table.cpp
                 Timer.cpp
                 shared.cpp
                 format.cpp
                 unicode.cpp
                 utf8.cpp
                 wcwidth6.cpp
                 ${shared_HEADERS})

add_library (shared STATIC ${shared_SRCS})

set (CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")
install (TARGETS shared DESTINATION lib)
install (FILES ${shared_HEADERS} DESTINATION include)
