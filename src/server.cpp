#include <iostream>
#include <thread>
#include "connection/Thread.h"
#include "connection/TCPStream.h"

// https://vichargrave.github.io/projects/

int main() {
    const int port = L_PORT_DEFAULT;
    const std :: string ip = L_IP_DEFAULT;
    unsigned int workers = std::thread::hardware_concurrency();
    if (workers == 0) {
        workers = 4;
    }

    workers = 1;

#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << "\n";
        return -1;
    }
#endif

    wqueue<WorkItem*> queue;
    for (int i = 0; i < workers; i++) {
        const auto handler = new ConnectionHandler(queue);

        handler->start();
    }

    // Create an acceptor then start listening for connections
    const auto connectionAcceptor = new TCPAcceptor(port, ip.c_str());

    if (connectionAcceptor->start() != 0) {
        printf("Could not create an connection acceptor\n");
        exit(1);
    }

    // Add a work item to the queue for each connection
    while (1) {
        TCPStream* connection = connectionAcceptor->accept();
        if (!connection) {
            printf("Could not accept a connection\n");
            continue;
        }
        const auto item = new WorkItem(connection);

        queue.add(item);
    }

    return 0;
}