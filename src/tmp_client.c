#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void print_usage() {
}

int main( int argc, char** argv ) {

    if (argc != 6) {
        printf("Usage: tmp_client host port oid offset length.\n");
        return -1;
    }

    char    *host = argv[1];
    int     port = atoi(argv[2]);
    char    *oid = argv[3];
    int64_t offset = (int64_t)atoi(argv[4]);
    int64_t length = (int64_t)atoi(argv[5]);
    char    *data = malloc(length);

    struct sockaddr_in server_addr_in;
    bzero(&server_addr_in, sizeof(server_addr_in) );
    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr_in.sin_addr);

    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    connect(cfd, (struct sockaddr *)&server_addr_in, sizeof(server_addr_in));

    int8_t op = 1;
    send(cfd, &op, sizeof(int8_t), 0);

    int16_t oid_size = strlen(oid);
    send(cfd, &oid_size, sizeof(int16_t), 0);
    send(cfd, oid, oid_size, 0);

    send(cfd, &offset, sizeof(int64_t), 0);

    send(cfd, &length, sizeof(int64_t), 0);
    send(cfd, data, length, 0);

    close(cfd);
    printf("Send req_write oid: %s, offset: %lu, length: %lu. \n",
            oid, offset, length);

    return 0;
}

