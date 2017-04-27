cmake_minimum_required(VERSION 3.7)

INCLUDE(ExternalProject)
set(OPENSSL_BINDIR ${CMAKE_BINARY_DIR}/openssl)

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

MESSAGE("Binary DIR" ${CMAKE_BINARY_DIR})
MESSAGE("Open SSL Target Bin DIR" ${OPENSSL_BINDIR})
MESSAGE("CMAKE ROOT PATH" ${$CMAKE_FIND_ROOT_PATH})
EXTERNALPROJECT_ADD(
    openssl
    URL https://www.openssl.org/source/openssl-1.0.2k.tar.gz 
    URL_HASH SHA256=6b3977c61f2aedf0f96367dcfb5c6e578cf37e7b8d913b4ecb6643c3cb88d8c0
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/downloads
    # URL ${CMAKE_SOURCE_DIR}/vendor/openssl-1.0.2-latest.tar.gz
    CONFIGURE_COMMAND ./config no-shared no-idea no-mdc2 no-rc5 --prefix=${OPENSSL_BINDIR} 
    BUILD_COMMAND make depend && make
    INSTALL_COMMAND make install_sw
    BUILD_IN_SOURCE 1
    )

set(OPENSSL_ROOT_DIR ${OPENSSL_BINDIR})
set(OpenSSL_DIR ${OPENSSL_BINDIR})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

include(FindOpenSSL)
message("OPEN SSL ROOT PATH" ${OPENSSL_ROOT_DIR})
message("OPENSSL_FOUND " ${OPENSSL_FOUND})
message("OPENSSL_INCLUDE_DIR" ${OPENSSL_INCLUDE_DIR})
message("OPENSSL_CRYPTO_LIBRARY" ${OPENSSL_CRYPTO_LIBRARY})
message("OOPENSSL_CRYPTO_LIBRARYPENSSL_SSL_LIBRARY" ${OPENSSL_SSL_LIBRARY})
message("OPENSSL_LIBRARIES" ${OPENSSL_LIBRARI})
message("OPENSSL_VERSION" ${OPENSSL_VERSION})


EXTERNALPROJECT_ADD(
    boost_ep
    URL https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.zip
    URL_HASH SHA256=b99973c805f38b549dbeaf88701c0abeff8b0e8eaa4066df47cac10a32097523
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ${Boost_Bootstrap_Command}
    BUILD_COMMAND ${Boost_b2_Command} install
    --with-system --with-date_time --with-random --with-thread link=static runtime-link=shared threading=multi
    --prefix=${CMAKE_BINARY_DIR}/INSTALL
    --threading=single,multi
    --link=static
    --variant=release
    -j8
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1

    )

find_package(Boost ${BOOST_VER} REQUIRED COMPONENTS system date_time random thread)
EXTERNALPROJECT_ADD(
    socketio-ep
    GIT_REPOSITORY https://github.com/socketio/socket.io-client-cpp.git 
    BUILD_IN_SOURCE 1
    CMAKE_ARGS -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}
    )

ExternalProject_Get_Property(socketio-ep SOURCE_DIR)
find_library(SIO_LIB libsioclient_tls.a ${SOURCE_DIR})
message("*********SIO INSTALL_DIR is " ${SIO_LIB})
#set(SOURCE_DIR ${SIO_LIB})

add_executable(beame-agent ./src/main.cpp)

target_include_directories(beame-agent PRIVATE ${Boost_INCLUDE_DIRS} 
    ${SOURCE_DIR}/src/ 
    ${OPENSSL_INCLUDE_DIR}
    )

target_link_libraries(beame-agent ${SIO_LIB})
target_link_libraries(beame-agent crypto )
target_link_libraries(beame-agent ssl)
target_link_libraries(beame-agent ${Boost_LIBRARIES})
target_link_libraries(beame-agent pthread)
