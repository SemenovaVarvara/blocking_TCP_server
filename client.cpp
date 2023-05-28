#include "diary1.h"

#include <arpa/inet.h>
//#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>
//#include <vector>

#include <signal.h>

#define  BUFLEN          1024

int writeToServer(int fd);
int readFromServer(int fd);

bool readAll(int fd, size_t N, vector<char> &buf);

volatile sig_atomic_t W;

void handler(int signo)
{
    W = signo;
}

int main() {
    struct sigaction a;
    a.sa_handler = handler;
    a.sa_flags = 0;
    sigemptyset(&a.sa_mask);

    if(sigaction(SIGPIPE, &a, 0) == -1)
        return -1;

    const char        *host = "127.0.0.1";
    uint16_t           port = 8000;
    struct sockaddr_in addr;
    int                fd;

    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if(inet_pton(AF_INET, host, &addr.sin_addr) < 1) {
        fprintf(stderr, "Wrong host\n");
        return -1;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        fprintf(stderr, "Can't create socket\n");
        return -1;
    }

    if(connect(fd, (struct sockaddr *)&addr, sizeof addr) == -1) {
        fprintf(stderr, "Can't connect to the server\n");
        if(close(fd))
            fprintf(stderr, "Can't realese file descriptor\n");
        return -1;
    }

    while(!W)
    {
        if(writeToServer(fd) < 0) break;
        if(readFromServer(fd) < 0) break;
    }

    if(shutdown(fd, 2) == -1)
        fprintf(stderr, "Can't shutdown socket\n");

    if(close(fd)) {
        fprintf(stderr, "Can't realese file descriptor\n");
        return -1;
    }

    return 0;
}

int writeToServer(int fd)
{
    int len;
    char buf[BUFLEN];

    try
    {
        fprintf(stdout, "> ");
        if (fgets(buf, BUFLEN, stdin) == nullptr)
        {
            fprintf(stdout, "the end\n");

            len = -1;
            if(write(fd, &len, sizeof len) != sizeof len)
                throw "Can't write a length";
            return -1;
        }

        buf[strlen(buf) - 1] = 0;
        len = strlen(buf) + 1;

        if(write(fd, &len, sizeof len) != sizeof len)
            throw "Can't write a length";

        if(write(fd, buf, len) != len)
        {
            return -1;
        }
//            throw "Can't write a text";
    }
    catch(const char *err)
    {
        fprintf(stderr, "%s\n", err);
    }
    return 0;
}

int readFromServer(int fd)
{
    int status, n;

    try
    {
        if(read(fd, &status, sizeof(int)) != sizeof(int))
            throw "Can't read the status";
        fprintf(stdout, "Server's replay:\n Status: %d\n", status);

        if(status < 0)
            return 0;

        if(read(fd, &n, sizeof(int)) != sizeof(int))
            throw "Can't read the number of filters";

        for(int i = 0; i < n; i++)
        {
            int len;
            if(read(fd, &len, sizeof(int)) != sizeof(int))
                throw "Can't read the length of the vector";

            vector<char> v(len);
            readAll(fd, len, v);

            diary1 record;
            toDiary1(record, v);
            print(record);
        }
    } catch(const char *err)
    {
        fprintf(stderr, "%s\n", err);
    }
    return 0;
}

bool readAll(int fd, size_t N, vector<char> &buf)
{
    buf.clear();
    buf.resize(N);
    size_t n = 0, nbytes;

    for(;;)
    {
        nbytes = read(fd, &buf[n], N - n);
        n += nbytes;

        if(n == N)
            return true;
    }
    return false;
}

