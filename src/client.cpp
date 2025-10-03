#include <iostream>
#include <string>
#include <memory>
#include "connection/Thread.h"
#include "connection/TCPStream.h"
#include "connection/protocol.h"
#include "input/repl.h"

int main() {
    int port = L_PORT_DEFAULT;
    std :: string ip = L_IP_DEFAULT;

#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << "\n";
        return -1;
    }
#endif

    int len;
    std :: string message;
    char line[256];
    std :: unique_ptr<TCPConnector> connector = std :: make_unique<TCPConnector>();
    std :: unique_ptr<TCPStream> stream(connector->connect(port, ip.c_str()));

    InputBuffer input_buffer = InputBuffer();

    if (stream == nullptr) {
        std :: cerr << "no connection\n";
        return 0;
    }

    while (true) {
        print_prompt();
        read_input(input_buffer);

        if ("exit" == input_buffer.get())
            break;

        if (!input_buffer.get().empty()) {

            Lexer l(input_buffer.get());
            Parser p(l);

            auto json = p.serialize();

            std :: cout << json << '\n';

            // stream->send(input_buffer.get().c_str(), input_buffer.get().size());
            // len = stream->receive(line, sizeof(line));
            // line[len] = 0;
            // printf("received - %s\n", line);
        }
    }

    return 0;
}