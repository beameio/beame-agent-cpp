cmake_minimum_required(VERSION 3.7)
project(beame-agent)
set(OPENSSL_BINDIR ${CMAKE_BINARY_DIR}/openssl)
set(OPENSSL_ROOT_DIR ${OPENSSL_BINDIR})
set(OpenSSL_DIR ${OPENSSL_BINDIR})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

find_package(Boost ${BOOST_VER} REQUIRED COMPONENTS system date_time random thread)
include(FindOpenSSL)
find_library(SIO_LIB libsioclient_tls.a ${SIO_SOURCE_PATH})
MESSAGE("***************************************222" ${SIO_SOURCE_PATH} )
add_executable(beame-agent main.cpp)
target_include_directories(beame-agent PRIVATE ${SIO_SOURCE_PATH}/src )
target_include_directories(beame-agent PRIVATE ${Boost_INCLUDE_DIRS} 
    ${SOURCE_DIR}/src/ 
    ${OPENSSL_INCLUDE_DIR}
    )
#add_dependencies(beame-agent boost_ep socketio-ep openssl)
target_link_libraries(beame-agent ${SIO_LIB})
target_link_libraries(beame-agent crypto )
target_link_libraries(beame-agent ssl)
target_link_libraries(beame-agent ${Boost_LIBRARIES})
##target_link_libraries(beame-agent ${Boost_LIBRARIES})
target_link_libraries(beame-agent pthread)
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
