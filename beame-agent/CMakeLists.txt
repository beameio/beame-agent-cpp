cmake_minimum_required(VERSION 3.7)
project(beame-agent)

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#   message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
### set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
set(Boost_USE_STATIC_LIBS   ON)
set(OPENSSL_BINDIR ${CMAKE_BINARY_DIR}/openssl)
#set(OPENSSL_ROOT_DIR ${OPENSSL_BINDIR})
set(OpenSSL_DIR ${OPENSSL_BINDIR})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

set (CMAKE_SHARED_LINKER_FLAGS "-lstatic")
set(CMAKE_BUILD_TYPE Debug)
### find_package(Boost ${BOOST_VER} REQUIRED COMPONENTS system date_time program_options random thread filesystem )
#include(FindOpenSSL)
# find_library(SIO_LIB libsioclient_tls.a ${SIO_SOURCE_PATH})

message(STATUS "SIO_LIB ${SIO_SOURCE_PATH} -> ${SIO_LIB}")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#   message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()


add_executable(beame-agent main.cpp crypto.cpp agent.cpp proxyClient.cpp https_client.cpp)


set_property(TARGET beame-agent PROPERTY CXX_STANDARD 11)
set_property(TARGET beame-agent PROPERTY CXX_STANDARD_REQUIRED ON)

include_directories(${Boost_INCLUDE_DIRS} ${SOURCE_DIR}/src/ ${OPENSSL_INCLUDE_DIR} ${SIO_SOURCE_PATH}/src ${BOOST_INCLUDEDIR})

SET(BUILD_SHARED_LIBRARIES OFF)
target_link_libraries(beame-agent  ${SIO_LIB}/libsioclient_tls.a)
target_link_libraries(beame-agent  ${OPENSSL_ROOT_DIR}/lib/libssl.a)
target_link_libraries(beame-agent  ${OPENSSL_ROOT_DIR}/lib/libcrypto.a)
target_link_libraries(beame-agent ${BOOST_LIBRARYDIR}/libboost_date_time.a)
target_link_libraries(beame-agent ${BOOST_LIBRARYDIR}/libboost_filesystem.a)
target_link_libraries(beame-agent ${BOOST_LIBRARYDIR}/libboost_program_options.a)
target_link_libraries(beame-agent ${BOOST_LIBRARYDIR}/libboost_random.a)
target_link_libraries(beame-agent ${BOOST_LIBRARYDIR}/libboost_system.a)
target_link_libraries(beame-agent ${BOOST_LIBRARYDIR}/libboost_thread.a)


# target_link_libraries(beame-agent  ${Boost_LIBRARIES})
target_link_libraries(beame-agent  pthread)
target_link_libraries(beame-agent  dl)
install(TARGETS beame-agent DESTINATION bin)
install(FILES ${Boost_LIBRARIES} DESTINATION lib)
# empty # install(FILES ${ALL_HEADERS} DESTINATION include)
# empty # install(TARGETS ${TARGET_LIBRARIES} DESTINATION lib)
