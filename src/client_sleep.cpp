#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "common.h"
#include "util.h"

unsigned char buffer[BUFFER_SIZE];

std::chrono::milliseconds sleepTime(1);

class Client {
  public:
    Client() : sock_{-1}, addr_{} {
        if ((sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
            assert(false);
        }
        memset((void *)&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = htonl(INADDR_ANY);
        addr_.sin_port = htons(PORT);
    }
    ~Client() { close(sock_); }

    void communicate() {
        if (connect(sock_, (struct sockaddr *)&addr_, sizeof(addr_)) < 0) {
            assert(false);
        }
        int ret;
        for (int i = 0; i < 10000; ++i) {
            ret = write(sock_, buffer, BUFFER_SIZE);
            assert(ret == BUFFER_SIZE);
            ret = read(sock_, buffer, BUFFER_SIZE);
            assert(ret == BUFFER_SIZE);
            std::this_thread::sleep_for(sleepTime);
        }
    }

  private:
    int sock_;
    struct sockaddr_in addr_;
};

int main() {
    for (int i = 0; i < TEST_CNT; ++i) {
        Client client;
        auto start = getNanoEpochTime();
        client.communicate();
        auto end = getNanoEpochTime();
        std::cout << "Cost: " << end - start << std::endl;
    }

    return 0;
}
