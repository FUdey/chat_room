#include "message_adapter.h"

bool MessageAdapter::isConnected()
{
    if (socket_fd_ <= 0)
        return false;
    tcp_info info{};
    int len = sizeof(info);
    getsockopt(socket_fd_, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
    if (info.tcpi_state == TCP_ESTABLISHED)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool MessageAdapter::getMessage(Message *msg)
{
    int size = read(socket_fd_, buffer_, HEADER_SIZE);
    if (size <= 0)
    {
        return false;
    }
    cout << "[DEBUG] header size" << size << endl;
    buf_ptr_ = buffer_;
    unsigned int type = parseInt(DATA_TYPE_SIZE);
    unsigned int length = parseInt(DATA_LENGTH_SIZE);
    if (type == JSON_TYPE)
    {
        cout << "[DEBUG] get json data length:" << length;
        string json_data = parseContent(length);
        msg->setMessage(type, length, json_data);
    }
    else
    {
        cout << "[ERROR] Unknown data type:" << type;
        cout << "   length:" << length << endl;
        int size = read(socket_fd_, buffer_, TCP_BUFFER_SIZE);
        cout << "[DEBUG] unknown content size" << size << endl;
        return false;
    }
    return true;
}

bool MessageAdapter::sendTextMessage(const string &text)
{
    Message text_msg(JSON_TYPE, text.length(), text);
    string msg_str = text_msg.getMsgString();
    if (isConnected() == 0)
    {
        return false;
    }
    send(socket_fd_, msg_str.data(), msg_str.length(), MSG_NOSIGNAL);
    return true;
}

unsigned int MessageAdapter::parseInt(int len)
{
    unsigned int sum = 0;
    unsigned int i = 0;
    for (const char *end = buf_ptr_ + len - 1; buf_ptr_ <= end; end--)
    {
        sum = sum + (((unsigned int)((unsigned char)(*end))) << i);
        i += 8;
    }
    buf_ptr_ = buf_ptr_ + len;
    return sum;
}

MessageAdapter::MessageAdapter(int fd)
{
    socket_fd_ = fd;
    buf_ptr_ = buffer_;
}

MessageAdapter::MessageAdapter()
{
    cout << "Empty MessageAdapter";
}

bool MessageAdapter::setSocket(int fd)
{
    socket_fd_ = fd;
    buf_ptr_ = buffer_;
}

string MessageAdapter::parseContent(unsigned int length)
{
    unsigned int saved_legnth = 0;
    int size = 0;
    unsigned int buffSize = TCP_BUFFER_SIZE;
    string data;
    while (saved_legnth < length)
    {
        if (isConnected() == 0)
        {
            cout << "[ERROR] connection fail " << socket_fd_ << endl;
            break;
        }
        size = read(socket_fd_, buffer_, min(buffSize, length - saved_legnth));
        cout << "[DEBUG] size:" << size << endl;
        if (size <= 0)
        {
            break;
        }

        saved_legnth += size;
        data += string(buffer_, size);
    }
    cout << "[DEBUG] content:" << data << endl;
    return data;
}