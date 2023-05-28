#include "diary1.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cinttypes>
#include <csignal>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <vector>

#include <climits>

#define BUFLEN  1024

static int signalIgnoring();
static void loop(int ld, int N, vector<diary1> &record);

int readFromClient(int fd, char* buf);
void writeToClient(int fd, char *buf, int N, vector<diary1> &record);

int toInt(const char* str, int* ptr);

int parser(string str);
int isDate(string str);
int isTime(string str);

volatile sig_atomic_t W;

int main(int ac, char* av[]) {
    int N, err;
    if (ac == 1 || ac > 2)
    {
        perror ("Server: enter parameter N");
        exit(EXIT_FAILURE);
    }

    err = toInt(av[1], &N);
    if(err == -1 || N <= 0)
    {
        perror ("Server: wrong parameter N");
        exit(EXIT_FAILURE);
    }

    vector<diary1> record(N);
    for(int i = 0; i < N; i++)
    {
        record[i].setRand();
        print(record[i]);
    }

    const char *host = "127.0.0.1";
    uint16_t    port = 8000;
    sockaddr_in addr;
    int         ld   = -1;
    int         on;

    if(signalIgnoring() != 0)
        return -1;

    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if(inet_pton(AF_INET, host, &addr.sin_addr) < 1) {
        fprintf(stderr, "Wrong host\n");
        return -1;
    }

    ld = socket(AF_INET, SOCK_STREAM, 0);
    if(ld == -1) {
        fprintf(stderr, "Can't create socket\n");
        return -1;
    }

    on = 1;
    if(setsockopt(ld, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) == -1) {
        fprintf(stderr, "Can't setsockopt\n");
        if(close(ld))
            fprintf(stderr, "Can't realese file descriptor\n");
        return -1;
    }

    if(bind(ld, (sockaddr *)&addr, sizeof addr) == -1) {
        fprintf(stderr, "Can't bind\n");
        if(close(ld))
            fprintf(stderr, "Can't realese file descriptor\n");
        return -1;
    }

    if(listen(ld, 5) == -1) {
        fprintf(stderr, "Can't listen\n");
        if(close(ld))
            fprintf(stderr, "Can't realese file descriptor\n");
        return -1;
    }

    loop(ld, N, record);

    if(close(ld)) {
        fprintf(stderr, "Can't realese file descriptor\n");
        return -1;
    }

    puts("Done.");

    return 0;
}

static void handler(int signo);

static int signalIgnoring() {
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);

    if(sigaction(SIGINT, &act, 0) == -1) {
        fprintf(stderr, "Can't setup SIGINT ignoring\n");
        return -1;
    }

    if(sigaction(SIGPIPE, &act, 0) == -1) {
        fprintf(stderr, "Can't setup SIGPIPE ignoring\n");
        return -1;
    }

    return 0;
}

static void handler(int signo) {
    (void)signo;
}

static void loop(int ld, int N, vector<diary1> &record) {
    for(int fd = -1;; fd = -1) {
        try {
            sockaddr_in addr;
            socklen_t   addrlen;

            memset(&addr, 0, sizeof addr);
            addrlen = sizeof addr;

            fd = accept(ld, (struct sockaddr *)&addr, &addrlen);
            if(fd == -1) {
                if(errno == EINTR)
                    return;
                else
                {
                    fprintf(stderr, "Can't accept\n");
                    continue;
                }
            }

            char buf[BUFLEN];
            int err;

            while(1)
            {
                err = readFromClient(fd, buf);
                if(err == -1)
                    break;
                else if(err == -2)
                    return;
                writeToClient(fd, buf, N, record);
            }
        }
        catch(const char *err) {
            fprintf(stderr, "%s\n", err);
        }
//        catch(std::bad_alloc) {
//            fprintf(stderr, "Memory allocation error\n");
//        }

        if(fd != -1) {
            if(shutdown(fd, 2) == -1)
                fprintf(stderr, "Can't shutdown socket\n");

            if(close(fd))
                fprintf(stderr, "Can't realese file descriptor\n");
        }
    }
}

int readFromClient(int fd, char* buf)
{
    int len;

    try
    {
        if(read(fd, &len, sizeof(int)) != sizeof(int)) {
            return -2;
        }

        if (len < 0)
            return -1;

        if(read(fd, buf, len) != len)
            throw "Can't read the message";

        fprintf(stdout, "Server got message:\n    %s\n", buf);
    } catch(const char* err)
    {
        fprintf(stderr, "%s\n", err);
    }

    return 0;
}

void writeToClient(int fd, char *buf, int N, vector<diary1> &record)
{
    string str(buf);
    int n;

    n = parser(str);

    try
    {
        if (n > N)
        {
            int k = -1;
            fprintf(stdout,"Write back:\n Status: -1\n\n");

            if(write(fd, &k, sizeof k) != sizeof k)
                throw "Can't write a status";

        } else if (n < 0)
        {
            int k = -2;
            fprintf(stdout,"Write back:\n Status: -2\n\n");

            if(write(fd, &k, sizeof k) != sizeof k)
                throw "Can't write a status";
        } else
        {
            int k = 0;
            fprintf(stdout,"Write back:\n Status: 0\n");

            if(write(fd, &k, sizeof k) != sizeof k)
                throw "Can't write a status";

            fprintf(stdout," The number of filters: %d\n", n);

            if(write(fd, &n, sizeof n) != sizeof n)
                throw "Can't write the number of filters";

            for(int i = 0; i < n; i++)
            {
                vector<char> v;
                toChar(record[i], v);

                int j = v.size();
                fprintf(stdout, " Size of vector: %d\n", j);

                if(write(fd, &j, sizeof j) != sizeof j)
                    throw "Can't write the size of vector";

                print(record[i]);

                if(write(fd, &v[0], v.size()) != (int)v.size())
                    throw "Can't write the vector";
            }

            fprintf(stdout, "\n");
        }
    } catch (const char *err)
    {
        fprintf(stderr, "%s\n", err);
    }
}

int toInt(const char* str, int* ptr)
{
    long L;
    char* e;

    errno = 0;
    L = strtol(str, &e, 10);

    if (!errno && *e == '\0')
        if (INT_MIN <= L && L <= INT_MAX)
        {
            *ptr = (int)L;
            return 0;
        }
        else
            return -1;
    else
        return -1;
}

int parser(string str)
{
    string substr, num;
    int filters = -1, flag = -1, dort;

    if(str.compare(0, 6, "select") == 0)
    {
        flag = 1;

        if(str.size() == 6)
            filters = 0;
        else if (str[6] != ' ')
            flag = -1;
    } else
    {
        flag = -1;
    }

    if(flag != -1 && filters != 0)
    {
        size_t pos = 6;
        substr = str.substr(pos);
        filters = 0;

        while(substr.compare(0, 7, " date=[") == 0 || substr.compare(0, 7, " time=[") == 0)
        {
            if(substr.compare(0, 7, " date=[") == 0)
                dort = 1;
            else
                dort = 2;

            pos += 7;
            substr = str.substr(pos);

            if(dort == 1)
            {
                if(isDate(substr) == -1)
                    flag = -1;

                pos += 10;
            }
            else
            {
                if(isTime(substr) == -1)
                    flag = -1;

                pos += 5;
            }

            if(str[pos] != ',')
            {
                flag = -1;
                break;
            }
            substr = str.substr(pos + 1);
            pos++;

            if(dort == 1)
            {
                if(isDate(substr) == -1)
                    flag = -1;

                pos += 10;
            }
            else
            {
                if(isTime(substr) == -1)
                    flag = -1;

                pos += 5;
            }

            if(str[pos] != ']')
            {
                flag = -1;
                break;
            }

            filters++;
            pos++;
            substr = str.substr(pos);
        }
    }

    if(substr.size() == 1 && substr[0] == ' ')
        flag = flag;
    else if(substr.size() != 0)
        flag = -1;

    if (flag == 1)
        return filters;
    else
        return -1;
}

int isDate(string str)
{
    string num;
    int n;

    if(str.size() < 10)
        return -1;

    if(str[0] < '0' || str[0] > '9')
        return -1;
    if(str[1] < '0' || str[1] > '9')
        return -1;

    num = str.substr(0, 2);
    n = stoi(num);
    if (n <= 0 || 31 < n)
        return -1;

    if(str[2] != '.')
        return  -1;

    if(str[3] < '0' || str[3] > '9')
        return -1;
    if(str[4] < '0' || str[4] > '9')
        return -1;

    num = str.substr(3, 2);
    n = stoi(num);
    if (n <= 0 || 12 < n)
        return -1;

    if(str[5] != '.')
        return  -1;

    if(str[6] < '0' || str[6] > '9')
        return -1;
    if(str[7] < '0' || str[7] > '9')
        return -1;
    if(str[8] < '0' || str[8] > '9')
        return -1;
    if(str[9] < '0' || str[9] > '9')
        return -1;

    num = str.substr(6, 4);
    n = stoi(num);
    if (n <= 0 || 2023 < n)
        return -1;

    return 0;
}

int isTime(string str)
{
    string num;
    int n;

    if (str.size() < 5)
        return -1;

    if(str[0] < '0' || str[0] > '9')
        return -1;
    if(str[1] < '0' || str[1] > '9')
        return -1;

    num = str.substr(0, 2);
    n = stoi(num);
    if (n < 0 || 23 < n)
        return -1;

    if(str[2] != '.')
        return  -1;

    if(str[3] < '0' || str[3] > '9')
        return -1;
    if(str[4] < '0' || str[4] > '9')
        return -1;

    num = str.substr(3, 2);
    n = stoi(num);
    if (n < 0 || 59 < n)
        return -1;

    return 0;
}