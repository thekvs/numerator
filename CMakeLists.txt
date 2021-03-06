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
include(FindPkgConfig)

CHECK_INCLUDE_FILE("inttypes.h" HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE("netinet/in.h" HAVE_NETINET_IN_H)

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

pkg_check_modules(GLOG REQUIRED libglog)

set(boost_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/boost)
ExternalProject_Add(
    boost
    PREFIX ${boost_PREFIX}
    URL "https://dl.bintray.com/boostorg/release/1.63.0/source/boost_1_63_0.tar.gz"
    URL_MD5 "7b493c08bc9557bbde7e29091f28b605"
    CONFIGURE_COMMAND ${boost_PREFIX}/src/boost/bootstrap.sh --with-libraries=thread,system --prefix=${boost_PREFIX}
    BUILD_COMMAND ./bjam link=static
    INSTALL_COMMAND ./bjam link=static install
    BUILD_IN_SOURCE 1
)
include_directories(SYSTEM ${boost_PREFIX}/include)
set(BOOST_LIBRARIES ${boost_PREFIX}/lib/libboost_thread.a)
list(APPEND BOOST_LIBRARIES ${boost_PREFIX}/lib/libboost_system.a)

set(judy_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/judy)
ExternalProject_Add(
    judy
    PREFIX ${judy_PREFIX}
    URL "https://netix.dl.sourceforge.net/project/judy/judy/Judy-1.0.5/Judy-1.0.5.tar.gz"
    URL_MD5 "115a0d26302676e962ae2f70ec484a54"
    CONFIGURE_COMMAND CXX=${CMAKE_CXX_COMPILER} CC=${CMAKE_C_COMPILER} ${judy_PREFIX}/src/judy/configure --prefix=${judy_PREFIX} --enable-64-bit
    BUILD_COMMAND ${MAKE_PROGRAM}
    INSTALL_COMMAND ${MAKE_PROGRAM} install
    BUILD_IN_SOURCE 1
)
include_directories(SYSTEM ${judy_PREFIX}/include)
set(JUDY_LIBRARIES ${judy_PREFIX}/lib/libJudy.a)

set(snappy_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/snappy)
ExternalProject_Add(
    snappy
    PREFIX ${snappy_PREFIX}
    # Snappy 1.1.3
    URL "https://github.com/google/snappy/releases/download/1.1.3/snappy-1.1.3.tar.gz"
    URL_MD5 "7358c82f133dc77798e4c2062a749b73"
    CONFIGURE_COMMAND CXX=${CMAKE_CXX_COMPILER} CC=${CMAKE_C_COMPILER} ${snappy_PREFIX}/src/snappy/autogen.sh && ${snappy_PREFIX}/src/snappy/configure --prefix=${snappy_PREFIX}
    BUILD_COMMAND $(MAKE)
    BUILD_IN_SOURCE 1
)
include_directories(SYSTEM ${snappy_PREFIX}/include)
set(SNAPPY_LIBRARIES ${snappy_PREFIX}/lib/libsnappy.a)

set(leveldb_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/leveldb)
ExternalProject_Add(
    leveldb
    DEPENDS snappy
    PREFIX ${leveldb_PREFIX}
    URL "https://github.com/google/leveldb/archive/v1.18.tar.gz"
    URL_MD5 "73770de34a2a5ab34498d2e05b2b7fa0"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND CXXFLAGS=-I${snappy_PREFIX}/include CXX=${CMAKE_CXX_COMPILER} CC=${CMAKE_C_COMPILER} $(MAKE) libleveldb.a
    INSTALL_COMMAND mkdir -p ${leveldb_PREFIX}/lib/ && cp ${leveldb_PREFIX}/src/leveldb/libleveldb.a ${leveldb_PREFIX}/lib/ && cp -r ${leveldb_PREFIX}/src/leveldb/include ${leveldb_PREFIX}/
    BUILD_IN_SOURCE 1
)
include_directories(SYSTEM ${leveldb_PREFIX}/include)
set(LEVELDB_LIBRARIES ${leveldb_PREFIX}/lib/libleveldb.a)

set(thrift_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/thrift)
ExternalProject_Add(
    thrift
    PREFIX ${thrift_PREFIX}
    URL "http://www.eu.apache.org/dist/thrift/0.9.3/thrift-0.9.3.tar.gz"
    URL_MD5 "88d667a8ae870d5adeca8cb7d6795442"
    CONFIGURE_COMMAND CXX=${CMAKE_CXX_COMPILER} CC=${CMAKE_C_COMPILER} ${thrift_PREFIX}/src/thrift/configure --prefix=${thrift_PREFIX} --with-boost=${boost_PREFIX} --enable-shared=no --with-libevent=no --with-c_glib=no --with-java=no --with-erlang=no --with-python=no --with-perl=no --with-php=no --with-php_extension=no --with-ruby=no --with-haskell=no --with-go=no --with-d=no --enable-tests=no
    BUILD_COMMAND $(MAKE)
    INSTALL_COMMAND $(MAKE) install
    BUILD_IN_SOURCE 1
)
include_directories(SYSTEM ${thrift_PREFIX}/include)
set(THRIFT_LIBRARIES ${thrift_PREFIX}/lib/libthrift.a)
set(THRIFT_GENERATOR ${thrift_PREFIX}/bin/thrift)
add_dependencies(thrift boost)

set(LINKLIBS
    ${THRIFT_LIBRARIES}
    ${LEVELDB_LIBRARIES}
    ${SNAPPY_LIBRARIES}
    ${JUDY_LIBRARIES}
    ${BOOST_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${GLOG_LIBRARIES}
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
add_dependencies(numerator leveldb judy)

target_link_libraries(numerator numerator-static ${LINKLIBS})
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(numerator rt)
endif()

add_executable(numexport ${numerator_SOURCE_DIR}/numexport.cpp)
add_dependencies(numexport leveldb judy)
target_link_libraries(numexport numerator-static ${LINKLIBS})
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(numexport rt)
endif()

add_executable(numimport ${numerator_SOURCE_DIR}/numimport.cpp)
add_dependencies(numimport leveldb judy)
target_link_libraries(numimport numerator-static ${LINKLIBS})
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(numimport rt)
endif()

add_executable(numeratorcli ${numerator_SOURCE_DIR}/numeratorcli.cpp)
target_link_libraries(numeratorcli numerator-static ${LINKLIBS})
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(numimport rt)
endif()

install(PROGRAMS ${numerator_BINARY_DIR}/numerator DESTINATION .)
install(PROGRAMS ${numerator_BINARY_DIR}/numexport DESTINATION .)
install(PROGRAMS ${numerator_BINARY_DIR}/numimport DESTINATION .)
install(PROGRAMS ${numerator_BINARY_DIR}/numeratorcli DESTINATION .)
