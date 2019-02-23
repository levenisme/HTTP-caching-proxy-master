#include <sys/select.h>
#include <unistd.h>
#include <netdb.h> // gethostbyname
#include <netinet/in.h>
#include <mutex>
#include <thread>
#include "function.h"

#define DEVELOPMENT 1

std::mutex mymutex;

int main(int argc, char **argv)
{
    // Create socket
    int status;
    int sockfd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    char hostname[128];
    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        std::cout << "Hostname access fail" << std::endl;
        exit(1);
    }
    const char *port = "12345";

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family = AF_INET;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0)
    {
        std::cerr << "Error: cannot get address info for host" << std::endl;
        std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
        return -1;
    } //if

    sockfd = socket(host_info_list->ai_family,
                    host_info_list->ai_socktype,
                    host_info_list->ai_protocol);
    if (sockfd == -1)
    {
        std::cerr << "Error: cannot create socket" << std::endl;
        std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
        return -1;
    } //if

    int yes = 1;
    status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(sockfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1)
    {
        std::cerr << "Error: cannot bind socket" << std::endl;
        std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
        return -1;
    } //if

    // Listening
    if (listen(sockfd, 10) != 0)
    {
        std::cout << "Error in listening" << std::endl;
        exit(1);
    }

    while (1)
    {
        // Get address info
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);

        // Accept request
        int reqfd = accept(sockfd, (struct sockaddr *)&socket_addr, &socket_addr_len);
        if (reqfd < 0)
        {
            std::cout << "Error in accept" << std::endl;
            exit(1);
        }

        std::thread mythread(handlehttp, reqfd);
        mythread.detach();
    }

    freeaddrinfo(host_info_list);
    close(sockfd);
    return 0;
}
