#include <iostream>
#include <string>
#include "connection/Thread.h"
#include "connection/TCPStream.h"

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
    auto connector = new TCPConnector();
    auto stream = connector->connect(port, ip.c_str());
    if (stream) {
        message = "Is there life on Mars?";
        stream->send(message.c_str(), message.size());
        printf("sent - %s\n", message.c_str());
        len = stream->receive(line, sizeof(line));
        line[len] = 0;
        printf("received - %s\n", line);
        delete stream;
    }

    stream = connector->connect(port, ip.c_str());
    if (stream) {
        message = "Why is there air?";
        stream->send(message.c_str(), message.size());
        printf("sent - %s\n", message.c_str());
        len = stream->receive(line, sizeof(line));
        line[len] = 0;
        printf("received - %s\n", line);
        delete stream;
    }

    exit(0);
}