#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include "common.h"
#include "util.h"

thread_local struct {
    long long read_start;
    long long read_end;
    long long write_start;
    long long write_end;
    void report() const {
#define MY_PRINT(var) std::cout << #var ": " << var << std::endl;
        MY_PRINT(read_start);
        MY_PRINT(read_end);
        MY_PRINT(write_start);
        MY_PRINT(write_end);
#undef MY_PRINT
    }
} TracePoint;

#define TRACEPOINT(s) TracePoint.s = getNanoEpochTime();

unsigned char buffer[BUFFER_SIZE];

class Server {
  public:
    Server() : sock_{-1}, addr_{} {
        if ((sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0) {
            assert(false);
        }

        int enabled = 1;
        int idleTime = 30;
        int interval = 5;
        int count = 3;
        if (setsockopt(sock_, SOL_SOCKET, SO_KEEPALIVE, &enabled, sizeof(enabled)) != 0) assert(false);
        if (setsockopt(sock_, SOL_TCP, TCP_KEEPIDLE, &idleTime, sizeof(idleTime)) != 0) assert(false);
        if (setsockopt(sock_, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) != 0) assert(false);
        if (setsockopt(sock_, SOL_TCP, TCP_KEEPCNT, &count, sizeof(count)) != 0) assert(false);

        memset((void *)&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = htonl(INADDR_ANY);
        addr_.sin_port = ntohs(PORT);

        int flags = fcntl(sock_, F_GETFL, 0);
        if (flags == -1) assert(false);
        flags |= O_NONBLOCK;
        auto ret = fcntl(sock_, F_SETFL, flags);
        assert(ret != -1);

        int flag = 1;
        setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int));

        if (bind(sock_, (struct sockaddr *)&addr_, sizeof(addr_)) < 0) {
            assert(false);
        }

        if (listen(sock_, SOMAXCONN) < 0) {
            assert(false);
        }
    }
    ~Server() { close(sock_); }

    void join() {
        int addrlen = sizeof(addr_);
        int ret;
        while (true) {
            int new_sock = -1;
            new_sock = accept(sock_, (struct sockaddr *)&addr_, (socklen_t *)&addrlen);
            if (new_sock == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                } else {
                    assert(false);
                }
            }

            for (int i = 0; i < 10000; ++i) {
                TRACEPOINT(read_start);
                ret = read(new_sock, buffer, BUFFER_SIZE);
                assert(ret == BUFFER_SIZE);
                TRACEPOINT(read_end);

                TRACEPOINT(write_start);
                ret = send(new_sock, (const void *)buffer, BUFFER_SIZE, MSG_DONTWAIT | MSG_NOSIGNAL);
                assert(ret == BUFFER_SIZE);
                TRACEPOINT(write_end);

                TracePoint.report();
                memset(&TracePoint, 0, sizeof(TracePoint));
            }

            close(new_sock);
        }
    }

  private:
    int sock_;
    struct sockaddr_in addr_;
};

int main() {
    Server server;
    server.join();
    return 0;
}
