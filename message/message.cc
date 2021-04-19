#include "message.h"

Message::Message()
{
}

Message::Message(unsigned int t, unsigned int l, string c) : type(t), length(l), content(c)
{
}

void Message::setMessage(unsigned int t, unsigned int l, string c)
{
    type = t;
    length = l;
    content = c;
}

unsigned int Message::getType()
{
    return type;
}

unsigned int Message::getLength()
{
    return length;
}

string Message::getContent()
{
    return content;
}

string Message::getHeader()
{
    char header[HEADER_SIZE];

    char *header_ptr = header + DATA_TYPE_SIZE - 1;
    *header_ptr = (char)(type & 0xff);
    // cout << "[DEBUG] type:" << type <<endl;
    // cout << "[DEBUG] HEADER:" << int(header[0]) <<endl;
    header_ptr = header + DATA_TYPE_SIZE + DATA_LENGTH_SIZE - 1;
    for (int i = 0; i < DATA_LENGTH_SIZE; i++)
    {
        *header_ptr = (char)((length >> 8 * i) & 0xff);
        header_ptr--;
    }
    // cout << "[DEBUG] HEADER1:" << int(header[1]) <<endl;
    // cout << "[DEBUG] HEADER2:" << int(header[2]) <<endl;
    // cout << "[DEBUG] HEADER3:" << int(header[3]) <<endl;
    // cout << "[DEBUG] HEADER4:" << int(header[4]) <<endl;
    // cout << "[DEBUG] HEADER:" << string(header,sizeof(header)) <<endl;
    return string(header, sizeof(header));
}

string Message::getMsgString()
{
    return getHeader() + getContent();
}