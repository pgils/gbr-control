#ifndef GBRSOCKETLISTENER_H
#define GBRSOCKETLISTENER_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>

#define BUFLEN	65536

class gbrSocketListener
{
public:
    gbrSocketListener(unsigned short port);
    virtual ~gbrSocketListener();

private:
    gbrSocketListener( const gbrSocketListener& );
    gbrSocketListener& operator=( const gbrSocketListener& );

public:
    int ListenForMessage();
    int SendMultiCast(std::string *message);
    std::string GetLastMessage() const;
    std::string GetLastSender() const;

private:
    struct sockaddr_in6	localSi;
    struct sockaddr_in6 senderSi;
    struct sockaddr_in6 multicastSi;
    int					sock;
    socklen_t			slen;

    std::string			mLastMessage;
    std::string			mLastSender;
};

#endif // GBRSOCKETLISTENER_H
