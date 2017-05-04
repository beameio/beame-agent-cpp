cmake_minimum_required(VERSION 3.7)
#project(beame-agent)
MESSAGE("SIO_SOURCE_PATH" ${SIO_SOURCE_PATH} )
MESSAGE("HUJ" ${HUJ})


#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#   message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()


set(OPENSSL_BINDIR ${CMAKE_BINARY_DIR}/openssl)
#set(OPENSSL_ROOT_DIR ${OPENSSL_BINDIR})
set(OpenSSL_DIR ${OPENSSL_BINDIR})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

set (CMAKE_SHARED_LINKER_FLAGS "-lstatic")
set(CMAKE_BUILD_TYPE Debug)
find_package(Boost ${BOOST_VER} REQUIRED COMPONENTS system date_time program_options random thread filesystem )
#include(FindOpenSSL)
find_library(SIO_LIB libsioclient_tls.a ${SIO_SOURCE_PATH})

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#   message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()


add_executable(beame-agent main.cpp crypto.cpp agent.cpp proxyClient.cpp)


set_property(TARGET beame-agent PROPERTY CXX_STANDARD 11)
set_property(TARGET beame-agent PROPERTY CXX_STANDARD_REQUIRED ON)

#target_include_directories(beame-agent PRIVATE ${SIO_SOURCE_PATH}/src )
#target_include_directories(beame-agent PRIVATE ${Boost_INCLUDE_DIRS}
#    ${SOURCE_DIR}/src/
#    ${OPENSSL_INCLUDE_DIR}
#    )
include_directories(${Boost_INCLUDE_DIRS}
            ${SOURCE_DIR}/src/
            ${OPENSSL_INCLUDE_DIR} ${SIO_SOURCE_PATH}/src)

SET(BUILD_SHARED_LIBRARIES OFF)
#add_dependencies(beame-agent boost_ep socketio-ep openssl)
target_link_libraries(beame-agent ${SIO_LIB})
target_link_libraries(beame-agent ${OPENSSL_ROOT_DIR}/lib/libssl.a)
target_link_libraries(beame-agent ${OPENSSL_ROOT_DIR}/lib/libcrypto.a)
#target_link_libraries(beame-agent ${BOOST_LIBRARYDIR}/system )
#target_link_libraries(beame-agent ${BOOST_LIBRARYDIR}/date_time )
#target_link_libraries(beame-agent ${BOOST_LIBRARYDIR}/random )
#target_link_libraries(beame-agent ${BOOST_LIBRARYDIR}/thread)

target_link_libraries(beame-agent ${Boost_LIBRARIES})
target_link_libraries(beame-agent pthread)
target_link_libraries(beame-agent dl)
#install(FILES ${ALL_HEADERS} 
#    DESTINATION "${CMAKE_CURRENT_LIST_DIR}/build/include"
#    )
#
#install(TARGETS ${TARGET_LIBRARIES}
#    DESTINATION "${CMAKE_CURRENT_LIST_DIR}/build/lib/${CMAKE_BUILD_TYPE}"
#    )
#
#install(FILES ${Boost_LIBRARIES} 
#    DESTINATION "${CMAKE_CURRENT_LIST_DIR}/build/lib/${CMAKE_BUILD_TYPE}"
#)
