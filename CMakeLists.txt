cmake_minimum_required(VERSION 4.0)
project(toySQL)

set(CMAKE_CXX_STANDARD 20)

add_executable(server
        src/server.cpp
        src/connection/Thread.cpp
        src/connection/Thread.h
        src/connection/wqueue.h
        src/connection/TCPStream.cpp
        src/connection/TCPStream.h
)

add_executable(client
        src/client.cpp
        src/connection/Thread.cpp
        src/connection/Thread.h
        src/connection/wqueue.h
        src/connection/TCPStream.cpp
        src/connection/TCPStream.h
)

IF (WIN32)
    target_link_libraries(server ws2_32)
    target_link_libraries(client ws2_32)
endif ()

target_compile_definitions(server PRIVATE
        L_PORT_DEFAULT=12346
        L_IP_DEFAULT="127.0.0.1"
)

target_compile_definitions(client PRIVATE
        L_PORT_DEFAULT=12346
        L_IP_DEFAULT="127.0.0.1"
)