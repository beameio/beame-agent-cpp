cmake_minimum_required(VERSION 3.7)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
INCLUDE(ExternalProject)
set(OPENSSL_BINDIR ${CMAKE_BINARY_DIR}/openssl)
set(CMAKE_BUILD_TYPE Debug)

set( Boost_Bootstrap_Command )
if( UNIX )
    set( Boost_Bootstrap_Command ./bootstrap.sh )
    set( Boost_b2_Command ./b2 )
else()
    if( WIN32 )
        set( Boost_Bootstrap_Command bootstrap.bat )
        set( Boost_b2_Command b2.exe )
    endif()
endif()

MESSAGE("Binary DIR1" ${CMAKE_BINARY_DIR})
MESSAGE("Open SSL Target Bin DIR" ${OPENSSL_BINDIR})
MESSAGE("CMAKE ROOT PATH" $${CMAKE_FIND_ROOT_PATH})
EXTERNALPROJECT_ADD(
    openssl
    URL http://www.openssl.org/source/openssl-1.0.2k.tar.gz
    URL_HASH SHA256=6b3977c61f2aedf0f96367dcfb5c6e578cf37e7b8d913b4ecb6643c3cb88d8c0
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/downloads
    # URL ${CMAKE_SOURCE_DIR}/vendor/openssl-1.0.2-latest.tar.gz
    CONFIGURE_COMMAND ./config no-shared no-idea no-mdc2 no-rc5 --prefix=${OPENSSL_BINDIR} && make depend
        BUILD_COMMAND make
    INSTALL_COMMAND make install_sw
    BUILD_IN_SOURCE 1
    )

set(OPENSSL_ROOT_DIR ${OPENSSL_BINDIR})
set(OpenSSL_DIR ${OPENSSL_BINDIR})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})



EXTERNALPROJECT_ADD(
    boost_ep DEPENDS openssl
    URL http://dl.bintray.com/boostorg/release/1.63.0/source/boost_1_63_0.zip
    URL_HASH SHA256=97ce6635df1f44653a597343cd5757bb8b6b992beb3720f5fc761e3644bcbe7b
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ${Boost_Bootstrap_Command}
    BUILD_COMMAND ${Boost_b2_Command} install
    --with-system --with-date_time --with-random  --with-program_options --with-filesystem --with-thread link=static runtime-link=shared threading=multi
    --prefix=${CMAKE_BINARY_DIR}/INSTALL
    --threading=single,multi
    --link=staticZ
    --variant=release
    -j8
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1

    )

set(BOOST_ROOT  ${CMAKE_BINARY_DIR}/INSTALL)
set(BOOST_INCLUDEDIR  ${BOOST_ROOT}/include/)
set(BOOST_LIBRARYDIR  ${BOOST_ROOT}/lib/)

#find_package(Boost ${BOOST_VER} REQUIRED COMPONENTS system date_time random thread)
##include(FindOpenSSL)
message("CMAKE_BINARY_DIR " ${CMAKE_BINARY_DIR})
message("BOOST_ROOT " ${BOOST_ROOT})
message("BOOST_INCLUDEDIR " ${BOOST_INCLUDEDIR})
message("BOOST_LIBRARYDIR " ${BOOST_LIBRARYDIR})

EXTERNALPROJECT_ADD(
    socketio-ep  DEPENDS   boost_ep openssl
    GIT_REPOSITORY http://github.com/beameio/socket.io-client-cpp.git
    BUILD_IN_SOURCE 1
    CMAKE_ARGS -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBOOST_INCLUDEDIR=${BOOST_INCLUDEDIR} -DBOOST_LIBRARYDIR=${BOOST_LIBRARYDIR}  -DBOOST_VER:STRING=1.64.0
    )

ExternalProject_Get_Property(socketio-ep SOURCE_DIR)
set(SIO_SOURCE_PATH  ${SOURCE_DIR})

EXTERNALPROJECT_ADD(
    beame-agent DEPENDS  openssl boost_ep  socketio-ep
    SOURCE_DIR ./src 
    BUILD_IN_SOURCE 1
    CMAKE_ARGS -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}  -DSIO_SOURCE_PATH=${SIO_SOURCE_PATH} -DBoost_INCLUDE_DIR=${BOOST_INCLUDEDIR} -DBOOST_INCLUDEDIR=${BOOST_INCLUDEDIR} -DBOOST_VER:STRING=1.63.0 -DBOOST_LIBRARYDIR=${BOOST_LIBRARYDIR} -DBOOST_ROOT=${BOOST_ROOT}
    INSTALL_COMMAND ""
    CONFIGURE_COMMAND ""
    )
