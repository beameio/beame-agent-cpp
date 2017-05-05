cmake_minimum_required(VERSION 3.7)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
include(ExternalProject)
set(OPENSSL_ROOT_DIR ${CMAKE_BINARY_DIR}/openssl)
set(CMAKE_BUILD_TYPE Debug)

# --- OpenSSL ---

EXTERNALPROJECT_ADD(
    openssl
    URL http://www.openssl.org/source/openssl-1.0.2k.tar.gz
    URL_HASH SHA256=6b3977c61f2aedf0f96367dcfb5c6e578cf37e7b8d913b4ecb6643c3cb88d8c0
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ./config no-shared no-idea no-mdc2 no-rc5 --prefix=${OPENSSL_ROOT_DIR} && make depend
    BUILD_COMMAND make
    INSTALL_COMMAND make install_sw
    )

set(OpenSSL_DIR ${OPENSSL_ROOT_DIR})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# --- Boost ---

set(BOOST_ROOT_DIR  ${CMAKE_BINARY_DIR}/boost)
set(BOOST_INCLUDEDIR  ${BOOST_ROOT_DIR}/include)
set(BOOST_LIBRARYDIR  ${BOOST_ROOT_DIR}/lib)

if( UNIX )
    set( Boost_Bootstrap_Command ./bootstrap.sh )
    set( Boost_b2_Command ./b2 )
else()
    if( WIN32 )
        set( Boost_Bootstrap_Command bootstrap.bat )
        set( Boost_b2_Command b2.exe )
    else()
        # Fatal
    endif()
endif()


EXTERNALPROJECT_ADD(
    boost DEPENDS openssl
    URL http://dl.bintray.com/boostorg/release/1.63.0/source/boost_1_63_0.zip
    URL_HASH SHA256=97ce6635df1f44653a597343cd5757bb8b6b992beb3720f5fc761e3644bcbe7b
    BUILD_IN_SOURCE 1
    # CONFIGURE_COMMAND ${Boost_Bootstrap_Command} --prefix=${BOOST_ROOT_DIR}
    CONFIGURE_COMMAND ${Boost_Bootstrap_Command}
    BUILD_COMMAND ${Boost_b2_Command}
        --with-system --with-date_time --with-random  --with-program_options --with-filesystem --with-thread link=static runtime-link=shared variant=debug threading=multi
    INSTALL_COMMAND ${Boost_b2_Command} install --prefix=${BOOST_ROOT_DIR}
    )

set(SIO_ROOT_DIR ${CMAKE_BINARY_DIR}/sio)
EXTERNALPROJECT_ADD(
    socketio DEPENDS boost openssl
    GIT_REPOSITORY http://github.com/beameio/socket.io-client-cpp.git
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${SIO_ROOT_DIR} -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBOOST_INCLUDEDIR=${BOOST_INCLUDEDIR} -DBOOST_LIBRARYDIR=${BOOST_LIBRARYDIR} -DBOOST_VER:STRING=1.63.0
    INSTALL_COMMAND make install
    )

ExternalProject_Get_Property(socketio SOURCE_DIR)
set(SIO_SOURCE_PATH ${SOURCE_DIR})

set(BEAME_AGENT_ROOT_DIR ${CMAKE_BINARY_DIR}/beame-agent)
EXTERNALPROJECT_ADD(
    beame-agent DEPENDS boost openssl socketio
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/beame-agent
    CMAKE_ARGS -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSIO_SOURCE_PATH=${SIO_SOURCE_PATH} -DBoost_INCLUDE_DIR=${BOOST_INCLUDEDIR} -DBOOST_INCLUDEDIR=${BOOST_INCLUDEDIR} -DBOOST_VER:STRING=1.63.0 -DBOOST_LIBRARYDIR=${BOOST_LIBRARYDIR} -DBOOST_ROOT=${BOOST_ROOT_DIR} -DSIO_LIB=${SIO_ROOT_DIR}/lib -DOPENSSL_INCLUDE_DIR=${OPENSSL_ROOT_DIR}/include -DCMAKE_INSTALL_PREFIX=${BEAME_AGENT_ROOT_DIR}
    )
