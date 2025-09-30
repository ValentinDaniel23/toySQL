#include <iostream>
#include "TCPStream.h"

TCPStream::TCPStream(socket_t sd, sockaddr_in* address) : m_sd(sd) {
    char ip[50];
    inet_ntop(PF_INET, (in_addr*)&(address->sin_addr.s_addr),
              ip, sizeof(ip)-1);
    m_peerIP = ip;
    m_peerPort = ntohs(address->sin_port);
}

TCPStream::~TCPStream()
{
    CLOSE_SOCKET(m_sd);
}

TCPStream* TCPConnector::connect(int port, const char* server)
{
    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (resolveHost(server, &(address.sin_addr)) != 0) {
        inet_pton(PF_INET, server, &(address.sin_addr));
    }

    const socket_t sd = socket(AF_INET, SOCK_STREAM, 0);
    checkSocket(sd, nullptr)

    if (::connect(sd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        return nullptr;
    }

    return new TCPStream(sd, &address);
}

int TCPConnector::resolveHost(const char* host, in_addr* addr)
{
    addrinfo *res;

    const int result = getaddrinfo (host, nullptr, nullptr, &res);
    if (result == 0) {
        memcpy(addr, &(reinterpret_cast<sockaddr_in *>(res->ai_addr))->sin_addr,
               sizeof(in_addr));
        freeaddrinfo(res);
    }
    return result;
}

TCPAcceptor::TCPAcceptor(int port, const char* address)
    : m_lsd(0), m_address(address), m_port(port), m_listening(false) {}

TCPAcceptor::~TCPAcceptor()
{
    if (m_lsd > 0) {
        CLOSE_SOCKET(m_lsd);
    }
}

int TCPAcceptor::start()
{
    if (m_listening == true) {
        return 0;
    }

    m_lsd = socket(PF_INET, SOCK_STREAM, 0);
    checkSocket(m_lsd, 0)

    sockaddr_in address{};
    address.sin_family = PF_INET;
    address.sin_port = htons(m_port);

    if (m_address.empty() || inet_pton(PF_INET, m_address.c_str(), &(address.sin_addr)) < 1) {
        address.sin_addr.s_addr = INADDR_ANY;
    }

    int optval = 1;
    setsockopt(m_lsd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&optval), sizeof optval);

    int result = bind(m_lsd, reinterpret_cast<sockaddr*>(&address), sizeof(address));
    if (result != 0) {
        perror("bind() failed");
        return result;
    }
    result = listen(m_lsd, lbacklog);
    if (result != 0) {
        perror("listen() failed");
        return result;
    }
    m_listening = true;
    return result;
}

TCPStream* TCPAcceptor::accept()
{
    if (m_listening == false) {
        return nullptr;
    }

    sockaddr_in address{};
    socklen_t len = sizeof(address);

    const socket_t sd = ::accept(m_lsd, reinterpret_cast<sockaddr*>(&address), &len);
    checkSocket(sd, nullptr)

    return new TCPStream(sd, &address);
}

void * ConnectionHandler::run() {

    for (int i = 0;; i++) {
        printf("thread %lu, loop %d - waiting for item...\n",
               (long unsigned int)self(), i);

        std::unique_ptr<WorkItem> item(m_queue.remove());

        printf("thread %lu, loop %d - got one item\n",
               (long unsigned int)self(), i);

        const auto stream = item->getStream();

        // Echo messages back the client until the connection is closed
        char input[256];
        ssize_t len;
        while ((len = stream->receive(input, sizeof(input)-1)) > 0 ){
            input[len] = 0;
            stream->send(input, len);
            printf("thread %lu, echoed '%s' back to the client\n",
                   (long unsigned int)self(), input);
        }
    }

    return nullptr;
}