#ifndef MESSAGE_MESSAGE_ADAPTER_H_
#define MESSAGE_MESSAGE_ADAPTER_H_

#include"protocol.h"
#include "message.h"
#include <unistd.h>
#include<arpa/inet.h>
#include<iostream>

#include <netinet/ip.h>
#include <netinet/tcp.h>

using namespace std;

/* 
    This class is used to read and write message using fd
 */
class MessageAdapter {
    //msg_type(1B) msg_length(4B) msg_content
private:
    int socket_fd_;
    char buffer_[TCP_BUFFER_SIZE];
    char* buf_ptr_;

    unsigned int parseInt(int len);

    string parseContent(unsigned int length);

    bool isConnected();

public:
    MessageAdapter();

    MessageAdapter(int fd);

    bool setSocket(int fd);

    bool getMessage(Message *msg);

    bool sendTextMessage(const string &text);

    // bool sendCtlMessage(const string &commen);
    
};

#endif // MESSAGE_MESSAGE_ADAPTER_H_

