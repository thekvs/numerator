project(numerator)

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Doesn't support non 64-bit platform")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)

include(FindThreads)
include(CheckTypeSize)
include(CheckIncludeFile)
include(CheckCXXCompilerFlag)
include(ExternalProject)

CHECK_INCLUDE_FILES("inttypes.h" HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES("netinet/in.h" HAVE_NETINET_IN_H)

if (HAVE_INTTYPES_H)
    add_definitions("-DHAVE_INTTYPES_H")
endif()

if (HAVE_NETINET_IN_H)
    add_definitions("-DHAVE_NETINET_IN_H")
endif()

include_directories(${numerator_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -W")

if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(MAKE_PROGRAM "gmake")
else()
    set(MAKE_PROGRAM "make")
endif()

set(boost_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/boost)
ExternalProject_Add(
    boost
    PREFIX ${boost_PREFIX}
    URL "http://heanet.dl.sourceforge.net/project/boost/boost/1.55.0/boost_1_55_0.tar.gz"
    URL_MD5 "93780777cfbf999a600f62883bd54b17"
    CONFIGURE_COMMAND ${boost_PREFIX}/src/boost/bootstrap.sh --with-libraries=thread,system --prefix=${boost_PREFIX}
    BUILD_COMMAND ./bjam link=static
    INSTALL_COMMAND ./bjam link=static install
    BUILD_IN_SOURCE 1
)
include_directories(${boost_PREFIX}/include)
set(BOOST_LIBRARIES ${boost_PREFIX}/lib/libboost_thread.a)
list(APPEND BOOST_LIBRARIES ${boost_PREFIX}/lib/libboost_system.a)

set(judy_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/judy)
ExternalProject_Add(
    judy
    PREFIX ${judy_PREFIX}
    URL "http://heanet.dl.sourceforge.net/project/judy/judy/Judy-1.0.5/Judy-1.0.5.tar.gz"
    URL_MD5 "115a0d26302676e962ae2f70ec484a54"
    CONFIGURE_COMMAND CXX=${CMAKE_CXX_COMPILER} CC=${CMAKE_C_COMPILER} ${judy_PREFIX}/src/judy/configure --prefix=${judy_PREFIX} --enable-64-bit
    BUILD_COMMAND ${MAKE_PROGRAM}
    INSTALL_COMMAND ${MAKE_PROGRAM} install
    BUILD_IN_SOURCE 1
)
include_directories(${judy_PREFIX}/include)
set(JUDY_LIBRARIES ${judy_PREFIX}/lib/libJudy.a)

set(snappy_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/snappy)
ExternalProject_Add(
    snappy
    PREFIX ${snappy_PREFIX}
    URL "http://snappy.googlecode.com/files/snappy-1.1.1.tar.gz"
    URL_MD5 "8887e3b7253b22a31f5486bca3cbc1c2"
    CONFIGURE_COMMAND CXX=${CMAKE_CXX_COMPILER} CC=${CMAKE_C_COMPILER} ${snappy_PREFIX}/src/snappy/configure --prefix=${snappy_PREFIX}
    BUILD_COMMAND make
    BUILD_IN_SOURCE 1
)
include_directories(${snappy_PREFIX}/include)
set(SNAPPY_LIBRARIES ${snappy_PREFIX}/lib/libsnappy.a)

set(leveldb_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/leveldb)
ExternalProject_Add(
    leveldb
    DEPENDS snappy
    PREFIX ${leveldb_PREFIX}
    GIT_REPOSITORY "http://code.google.com/p/leveldb/"
    GIT_TAG "269fc6ca9416129248db5ca57050cd5d39d177c8"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND CXXFLAGS=-I${snappy_PREFIX}/include CXX=${CMAKE_CXX_COMPILER} CC=${CMAKE_C_COMPILER} ${MAKE_PROGRAM} libleveldb.a
    INSTALL_COMMAND mkdir -p ${leveldb_PREFIX}/lib/ && cp ${leveldb_PREFIX}/src/leveldb/libleveldb.a ${leveldb_PREFIX}/lib/ && cp -r ${leveldb_PREFIX}/src/leveldb/include ${leveldb_PREFIX}/
    BUILD_IN_SOURCE 1
)
include_directories(${leveldb_PREFIX}/include)
set(LEVELDB_LIBRARIES ${leveldb_PREFIX}/lib/libleveldb.a)

set(glog_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/glog)
ExternalProject_Add(
    glog
    PREFIX ${glog_PREFIX}
    URL "http://google-glog.googlecode.com/files/glog-0.3.3.tar.gz"
    URL_MD5 "a6fd2c22f8996846e34c763422717c18"
    CONFIGURE_COMMAND CXX=${CMAKE_CXX_COMPILER} CC=${CMAKE_C_COMPILER} ${glog_PREFIX}/src/glog/configure --prefix=${glog_PREFIX} --enable-shared=no
    BUILD_COMMAND make
    BUILD_IN_SOURCE 1
)
include_directories(${glog_PREFIX}/include)
set(GLOG_LIBRARIES ${glog_PREFIX}/lib/libglog.a)

set(thrift_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/thrift)
ExternalProject_Add(
    thrift
    PREFIX ${thrift_PREFIX}
    URL "http://www.eu.apache.org/dist/thrift/0.9.1/thrift-0.9.1.tar.gz"
    URL_MD5 "d2e46148f6e800a9492dbd848c66ab6e"
    CONFIGURE_COMMAND CXX=${CMAKE_CXX_COMPILER} CC=${CMAKE_C_COMPILER} ${thrift_PREFIX}/src/thrift/configure --prefix=${thrift_PREFIX} --with-boost=${boost_PREFIX} --enable-shared=no --with-libevent=no --with-c_glib=no --with-java=no --with-erlang=no --with-python=no --with-perl=no --with-php=no --with-php_extension=no --with-ruby=no --with-haskell=no --with-go=no --with-d=no
    BUILD_COMMAND ${MAKE_PROGRAM}
    INSTALL_COMMAND ${MAKE_PROGRAM} install
    BUILD_IN_SOURCE 1
)
include_directories(${thrift_PREFIX}/include)
set(THRIFT_LIBRARIES ${thrift_PREFIX}/lib/libthrift.a)
set(THRIFT_GENERATOR ${thrift_PREFIX}/bin/thrift)
add_dependencies(thrift boost)

set(LINKLIBS    ${THRIFT_LIBRARIES}
                ${LEVELDB_LIBRARIES}
                ${SNAPPY_LIBRARIES}
                ${GLOG_LIBRARIES}
                ${JUDY_LIBRARIES}
                ${BOOST_LIBRARIES}
                ${CMAKE_THREAD_LIBS_INIT}
)

add_custom_command(
    DEPENDS ${numerator_SOURCE_DIR}/numerator.thrift
    COMMAND ${THRIFT_GENERATOR}
    ARGS -r -gen py:new_style -gen cpp -o ${numerator_SOURCE_DIR} ${numerator_SOURCE_DIR}/numerator.thrift
    OUTPUT  "${numerator_SOURCE_DIR}/gen-cpp/Numerator.cpp"
            "${numerator_SOURCE_DIR}/gen-cpp/numerator_constants.cpp"
            "${numerator_SOURCE_DIR}/gen-cpp/numerator_types.cpp"
    COMMENT "Executing Thrift compiler"
)
set_source_files_properties(${numerator_SOURCE_DIR}/gen-cpp/Numerator.cpp
                            ${numerator_SOURCE_DIR}/gen-cpp/numerator_constants.cpp
                            ${numerator_SOURCE_DIR}/gen-cpp/numerator_types.cpp
                            ${numerator_SOURCE_DIR}/gen-cpp/Numerator.h
                            ${numerator_SOURCE_DIR}/gen-cpp/numerator_constants.h
                            ${numerator_SOURCE_DIR}/gen-cpp/numerator_types.h
                            PROPERTIES GENERATED TRUE
)

set(NUMERATOR_PROTOCOL_SOURCES  ${numerator_SOURCE_DIR}/gen-cpp/Numerator.cpp
                                ${numerator_SOURCE_DIR}/gen-cpp/numerator_constants.cpp
                                ${numerator_SOURCE_DIR}/gen-cpp/numerator_types.cpp
)

set(NUMERATOR_SOURCES   ${NUMERATOR_PROTOCOL_SOURCES}
                        ${numerator_SOURCE_DIR}/disk_storage.cpp
                        ${numerator_SOURCE_DIR}/memory_storage.cpp
                        ${numerator_SOURCE_DIR}/signal_handler.cpp
)

add_library(numerator-static STATIC ${NUMERATOR_SOURCES})
add_dependencies(numerator-static thrift)
set_target_properties(numerator-static PROPERTIES OUTPUT_NAME numerator)
set_target_properties(numerator-static PROPERTIES LINKER_LANGUAGE CXX)

add_executable(numerator ${numerator_SOURCE_DIR}/numerator.cpp)
add_dependencies(numerator leveldb glog judy)

target_link_libraries(numerator numerator-static ${LINKLIBS})
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(numerator rt)
endif()

add_executable(numexport ${numerator_SOURCE_DIR}/numexport.cpp)
add_dependencies(numexport leveldb glog judy)
target_link_libraries(numexport numerator-static ${LINKLIBS})
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(numexport rt)
endif()

add_executable(numimport ${numerator_SOURCE_DIR}/numimport.cpp)
add_dependencies(numimport leveldb glog judy)
target_link_libraries(numimport numerator-static ${LINKLIBS})
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(numimport rt)
endif()

install(PROGRAMS ${numerator_BINARY_DIR}/numerator DESTINATION .)
install(PROGRAMS ${numerator_BINARY_DIR}/numexport DESTINATION .)
install(PROGRAMS ${numerator_BINARY_DIR}/numimport DESTINATION .)
