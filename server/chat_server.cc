#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/epoll.h>
#include <json/json.h>
#include "message/message.h"
#include "message/message_adapter.h"
#include "service/online_service.h"
#include "config/server_config.h"

using namespace std;

int main()
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{}, clientAddr{};
    int opt = 1;
    if (-1 == setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        cout << "setsockopt fail" << endl;
        exit(-1);
    }

    int epfd = epoll_create(MAX_CONNECTIONS);
    epoll_event ev{}, events[MAX_CONNECTIONS];
    ev.data.fd = lfd;
    ev.events = EPOLLIN;
    if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev))
    {
        cout << "epoll_ctl fail" << endl;
        exit(-1);
    }
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, HOST, &serverAddr.sin_addr); //convert addr
    //bind socket
    if (-1 == bind(lfd, (sockaddr *)&serverAddr, sizeof(serverAddr)))
    {
        cout << "bind fail" << endl;
        exit(-1);
    }
    if (-1 == listen(lfd, MAX_CONNECTIONS))
    {
        cout << "listen fail" << endl;
        exit(-1);
    }
    cout << "listening..." << endl;

    char ipAddress[BUFSIZ];
    OnlineService online_service;
    while (true)
    {
        int ready_size = epoll_wait(epfd, events, MAX_CONNECTIONS, -1);
        if (ready_size < 0)
        {
            cout << "epoll_wait error" << endl;
            exit(-1);
        }
        cout << "epoll get " << ready_size << " event" << endl;
        for (int i = 0; i < ready_size; i++)
        {
            int fd = events[i].data.fd;
            if (fd == lfd)
            {
                cout << "New connection" << endl;
                socklen_t len = sizeof(clientAddr);
                int cfd = accept(lfd, (sockaddr *)&clientAddr, &len);
                cout << "New fd:" << cfd << endl;
                ev.data.fd = cfd;
                ev.events = EPOLLIN;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(clientAddr));
                //set time out
                struct timeval timeout = {1, 0};
                setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
            }
            else if (events[i].events & EPOLLIN)
            {
                MessageAdapter message_adapter(fd); // msg reader
                Message msg;
                if (message_adapter.getMessage(&msg) &&
                    online_service.process(msg.getContent(), fd))
                {
                    cout << "Request process done." << endl;
                }
                else
                {
                    // close socket if receive empty or wrong EPOLLIN
                    sleep(1);
                    ev.data.fd = fd;
                    ev.events = EPOLLIN;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
                    close(fd);
                    cout << "Close fd:" << fd << endl;
                }
            }
        }
    }

    close(lfd);
    return 0;
}
