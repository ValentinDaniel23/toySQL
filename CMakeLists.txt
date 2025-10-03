cmake_minimum_required(VERSION 4.0)
project(toySQL)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

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
        src/input/repl.h
        src/input/repl.cpp
        src/connection/protocol.cpp
        src/connection/protocol.h
)

IF (WIN32)
    target_link_libraries(server ws2_32 nlohmann_json::nlohmann_json)
    target_link_libraries(client ws2_32 nlohmann_json::nlohmann_json)
endif ()

target_compile_definitions(server PRIVATE
        L_PORT_DEFAULT=12346
        L_IP_DEFAULT="127.0.0.1"
)

target_compile_definitions(client PRIVATE
        L_PORT_DEFAULT=12346
        L_IP_DEFAULT="127.0.0.1"
)