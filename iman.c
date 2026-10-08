#include "iman.h"

#ifndef MAN_HOST
#define MAN_HOST "man.he.net"
#endif
#ifndef MAN_PORT
#define MAN_PORT "80"
#endif

void iman(char* command)
{
    char man[LEN], request[LEN];
    int i, sockfd;
    struct addrinfo hints, *res;
    char* buffer;
    size_t size, cap;
    ssize_t content;
    char* body;
    int in;

    /* Only the first argument is used */
    i = 0;
    if(!next_word(command, &i, man))
    {
        printf(RED "Usage: iMan <command_name>" RESET "\n");
        return;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if(getaddrinfo(MAN_HOST, MAN_PORT, &hints, &res) != 0)
    {
        printf(RED "Error : could not resolve " MAN_HOST RESET "\n");
        return;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(sockfd < 0)
    {
        printf(RED "Error" RESET "\n");
        freeaddrinfo(res);
        return;
    }

    /* A failed connection must not kill the shell */
    if(connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
        printf(RED "Error : connection failed" RESET "\n");
        close(sockfd);
        freeaddrinfo(res);
        return;
    }
    freeaddrinfo(res);

    snprintf(request, sizeof(request),
        "GET /?topic=%s&section=all HTTP/1.0\r\n"
        "Host: " MAN_HOST "\r\n"
        "Connection: close\r\n"
        "\r\n", man);

    if(send(sockfd, request, strlen(request), 0) < 0)
    {
        printf(RED "Error" RESET "\n");
        close(sockfd);
        return;
    }

    cap = 65536;
    size = 0;
    buffer = (char*)malloc(cap);
    content = recv(sockfd, buffer, cap - 1, 0);
    while(content > 0)
    {
        size += content;
        if(cap - size < 4096)
        {
            cap = cap * 2;
            buffer = (char*)realloc(buffer, cap);
        }
        content = recv(sockfd, buffer + size, cap - size - 1, 0);
    }
    if(content < 0)
        printf(RED "Error" RESET "\n");
    buffer[size] = '\0';
    close(sockfd);

    /* Skip the HTTP header (it ends at the first blank line) */
    body = strstr(buffer, "\r\n\r\n");
    if(body != NULL)
        body = body + 4;
    else
        body = buffer;

    /* HTML tags are dropped, the text is printed as it is */
    in = 0;
    for(i=0 ; body[i] != '\0' ; i++)
    {
        if(body[i] == '<')
            in = 1;
        if(!in)
            putchar(body[i]);
        if(body[i] == '>')
            in = 0;
    }
    fflush(stdout);

    free(buffer);
    return;
}
