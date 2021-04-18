#ifndef MESSAGE_MESSAGE_H_
#define MESSAGE_MESSAGE_H_

#include "protocol.h"
#include <iostream>

using namespace std;

class Message {
private:
    unsigned int type;
    unsigned int length;
    string content;
public:
    Message();
    Message(unsigned int t, unsigned int l, string c);

    void setMessage(unsigned int t, unsigned int l, string c);
    
    unsigned int getType();

    unsigned int getLength();

    string getContent();

    string getHeader();

    string getMsgString();
};

#endif // MESSAGE_MESSAGE_H_
