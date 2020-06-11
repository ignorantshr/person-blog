## 头文件
```c
//
// Created by root on 2/2/19.
//

#ifndef SOCKETTOY_SERVER_H
#define SOCKETTOY_SERVER_H
#define SERVER_IP "172.16.2.124"
#define SERVER_PORT 5900
#endif //SOCKETTOY_SERVER_H
```

## 客户端
```c
//
// Created by root on 2/2/19.
//
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include "server.h"

void handle_error(char *str){
    printf("Client %s error!\n", str);
    exit(1);
}

int main(void){
    int re;
    char str_buf[BUFSIZ] = {"sdsaf"};

    /* create socket */
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1){
        handle_error("create socket");
    }

    /* connect */
    struct sockaddr_in server_addr;
    struct in_addr buf;
    inet_pton(AF_INET, SERVER_IP, &buf);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
//    server_addr.sin_addr.s_addr = buf.s_addr;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    re = connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (re == -1){
        handle_error("connect");
    }

    printf("Input [exit] to exit the socket\n");
    while (1){
        printf("input your word: ");
        scanf("%s", str_buf);
        if (memcmp(str_buf, "exit", 4) == 0){
            break;
        }

        send(client_socket, str_buf, sizeof(str_buf), 0);
        recv(client_socket, str_buf, sizeof(str_buf), 0);
        printf("str_buf convert: %s\n", str_buf);
    }

    /* close */
    ssize_t r = send(client_socket, "exit", 4, 0);
    if (r == -1){
        handle_error("send");
    }
    close(client_socket);
    return 0;
}
```

## 服务端
```c
//
// Created by root on 2/2/19.
//
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <memory.h>
#include <pthread.h>
#include "server.h"

void handle_error(char *str){
    printf("Server %s error!\n", str);
    exit(1);
}

int main(void){
    int re;

    /* create socket */
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1){
        handle_error("create socket");
    }

    /* bind */
    struct sockaddr_in server_addr;
    struct in_addr buf;
    inet_pton(AF_INET, SERVER_IP, &buf);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
//    server_addr.sin_addr.s_addr = buf.s_addr;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    re = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (re != 0){
        handle_error("bind");
    }

    /* listen */
    //max = 128
    re = listen(server_socket, 10);
    if (re != 0){
        handle_error("listen");
    }

    /* accept */
    struct sockaddr_in client_addr;
    char str_buf[BUFSIZ];
    ssize_t n;
    int i;
    socklen_t in_out = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &in_out);

    printf("Client ip: %s, port: %d.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    while (1){
        n = recv(client_socket, str_buf, sizeof(str_buf), 0);
        if (memcmp(str_buf, "exit", 4) == 0){
            break;
        }
        for (i = 0; i < n; ++i){
            str_buf[i] = toupper(str_buf[i]);
        }
        send(client_socket, str_buf, (size_t)n, 0);
    }

    /* close */
    close(client_socket);
    close(server_socket);

    return 0;
}
```
