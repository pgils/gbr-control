#ifndef GBRSOCKETLISTENER_H
#define GBRSOCKETLISTENER_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>

#define		SOCKET_TIMEO	5		// socket timeout in seconds
#define		BUFLEN			65536	// buffer length in bytes

class gbrSocketListener
{
public:
    gbrSocketListener(unsigned short port);
    virtual ~gbrSocketListener();

private:
    gbrSocketListener( const gbrSocketListener& );
    gbrSocketListener& operator=( const gbrSocketListener& );

public:
    long ListenForMessage();
    int SendMessage(std::string *message, sockaddr_in6 *sockIn);
    int SendMultiCast(std::string *message);
    int SendLocal(std::string *message);
    std::string GetLastMessage() const;

private:
    struct sockaddr_in6	receiverSi;
    struct sockaddr_in6	multicastSi;
    struct sockaddr_in6	remoteSi;
    int					sock;
    socklen_t			rlen;

    std::string			mLastMessage;
};

#endif // GBRSOCKETLISTENER_H
