cmake_minimum_required(VERSION 3.7)

INCLUDE(ExternalProject)
set(OPENSSL_BINDIR ${CMAKE_BINARY_DIR}/openssl)


MESSAGE("Binary DIR" ${CMAKE_BINARY_DIR})
MESSAGE("Open SSL Target Bin DIR" ${OPENSSL_BINDIR})
MESSAGE("CMAKE ROOT PATH" ${$CMAKE_FIND_ROOT_PATH})
EXTERNALPROJECT_ADD(
    openssl
    URL https://www.openssl.org/source/openssl-1.1.0e.tar.gz 
    DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/downloads
    # URL ${CMAKE_SOURCE_DIR}/vendor/openssl-1.0.2-latest.tar.gz
    CONFIGURE_COMMAND ./config no-shared no-idea no-mdc2 no-rc5 --prefix=${OPENSSL_BINDIR} 
    BUILD_COMMAND make depend && make
    INSTALL_COMMAND make install_dev
    BUILD_IN_SOURCE 1
    )
find_package(OpenSSL REQUIRED)
