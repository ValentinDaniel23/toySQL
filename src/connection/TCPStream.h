#ifndef TOYSQL_TCPSTREAM_H
#define TOYSQL_TCPSTREAM_H

#include <string>
#include <memory>
#include "thread.h"
#include "wqueue.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

using socket_t = SOCKET;

#define CLOSE_SOCKET(s) closesocket(s)
#define READ(s, buf, len) ::recv(s, buf, static_cast<int>(len), 0);
#define WRITE(s, buf, len) ::send(m_sd, buf, static_cast<int>(len), 0);
#define checkSocket(s, r) if (s == INVALID_SOCKET) {perror("socket failed"); return r;}

#else

#include <sys/socket.h>
#include <unistd.h>

using socket_t = int;

#define CLOSE_SOCKET(s) close(s)
#define READ(s, buf, len) ::read(s, buf, len);
#define WRITE(s, buf, len) ::write(m_sd, buf, len);
#define checkSocket(s, r) if (s < 0) {perror("socket failed"); return r;}

#endif

constexpr int lbacklog = 5;

class TCPStream {
    socket_t m_sd;
    std :: string  m_peerIP;
    int     m_peerPort;

public:
    friend class TCPAcceptor;
    friend class TCPConnector;

    ~TCPStream();

    ssize_t send(const char* buffer, const size_t len) const {
        return WRITE(m_sd, buffer, len);
    }
    ssize_t receive(char* buffer, const size_t len) const {
        return READ(m_sd, buffer, len);
    }

    [[nodiscard]] std :: string getPeerIP() const {
        return m_peerIP;
    }
    [[nodiscard]] int getPeerPort() const {
        return m_peerPort;
    }

private:
    TCPStream(socket_t sd, sockaddr_in* address);
};

class TCPConnector
{
public:
    TCPStream* connect(int port, const char* server);

private:
    int resolveHost(const char* host, in_addr* addr);
};

class TCPAcceptor
{
    socket_t m_lsd;
    std :: string m_address;
    int    m_port;
    bool   m_listening;

public:
    TCPAcceptor(int port, const char* address);
    ~TCPAcceptor();

    int        start();
    TCPStream* accept();
};

class WorkItem
{
    TCPStream* m_stream;

public:
    explicit WorkItem(TCPStream* stream) : m_stream(stream) {}
    ~WorkItem() { delete m_stream; }

    TCPStream* getStream() { return m_stream; }
};

class ConnectionHandler : public Thread
{
    wqueue<WorkItem*>& m_queue;

public:
    explicit ConnectionHandler(wqueue<WorkItem*>& queue) : m_queue(queue) {}

    void* run();
};

#endif //TOYSQL_TCPSTREAM_H