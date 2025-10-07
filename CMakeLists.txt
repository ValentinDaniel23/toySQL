cmake_minimum_required(VERSION 4.0)
project(toySQL)

set(CMAKE_CXX_STANDARD 17)

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
        src/storage/tuple.cpp
)

target_compile_definitions(server PRIVATE
        L_PORT_DEFAULT=12346
        L_IP_DEFAULT="127.0.0.1"
)

target_compile_definitions(client PRIVATE
        L_PORT_DEFAULT=12346
        L_IP_DEFAULT="127.0.0.1"
)


include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ---- Tests ----
# Test Serialize
add_executable(testsSerialize
        src/tests/test_serialize.cpp
        src/connection/protocol.cpp
        src/connection/protocol.h
)

#Tests BTree
add_executable(testsStorage
        src/tests/test_storage.cpp
        src/storage/tuple.cpp
)

#Tests OS_API
add_executable(testsOS_API
        src/tests/test_OS_API.cpp
        src/OS/specificOS.h
)

IF (WIN32)
    target_link_libraries(server ws2_32 nlohmann_json::nlohmann_json)
    target_link_libraries(client ws2_32 nlohmann_json::nlohmann_json)
    target_link_libraries(testsSerialize ws2_32 nlohmann_json::nlohmann_json GTest::gtest_main)
    target_link_libraries(testsStorage ws2_32 nlohmann_json::nlohmann_json GTest::gtest_main)
    target_link_libraries(testsOS_API ws2_32 GTest::gtest_main)
endif ()

include(GoogleTest)
gtest_discover_tests(testsSerialize)
gtest_discover_tests(testsStorage)
gtest_discover_tests(testsOS_API)