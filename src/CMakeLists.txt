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
                    Lexer.h
                    Log.h
                    Msg.h
                    Packrat.h
                    Palette.h
                    PEG.h
                    Pig.h
                    RX.h
                    Table.h
                    Timer.h
                    Tree.h
                    shared.h
                    format.h
                    unicode.h
                    utf8.h
                    wcwidth.h)

set (shared_SRCS Args.cpp
                 Color.cpp
                 Composite.cpp
                 Configuration.cpp
                 Datetime.cpp
                 Duration.cpp
                 FS.cpp
                 JSON.cpp
                 Lexer.cpp
                 Log.cpp
                 Msg.cpp
                 Packrat.cpp
                 Palette.cpp
                 PEG.cpp
                 Pig.cpp
                 RX.cpp
                 SAX.cpp
                 Table.cpp
                 Timer.cpp
                 Tree.cpp
                 format.cpp
                 ip.cpp
                 shared.cpp
                 unicode.cpp
                 utf8.cpp
                 ${shared_HEADERS})

add_library (shared STATIC ${shared_SRCS})

set (CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")
install (TARGETS shared DESTINATION lib)
install (FILES ${shared_HEADERS} DESTINATION include)

add_executable (lex_executable lex.cpp)
target_link_libraries (lex_executable shared)
set_property (TARGET lex_executable PROPERTY OUTPUT_NAME "lex")
