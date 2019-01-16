// INET6 socket listener
// based on the example:
// https://www.binarytides.com/programming-udp-sockets-c-linux/


#include "gbrsocketlistener.h"

#include <stdexcept>
#include <cerrno>		// for errno.
#include <cstring>		// for memset.
#include <unistd.h>		// for close.
#include <net/if.h>		// for if_nametoindex.

gbrSocketListener::gbrSocketListener(unsigned short port)
{
    if( -1 == (this->sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) )
    {
        throw std::runtime_error("Failed to create socket.");
    }

    //zero out the sockaddrs
    memset( reinterpret_cast<char*>(&this->receiverSi), 0, sizeof(this->receiverSi) );
    memset( reinterpret_cast<char*>(&this->multicastSi), 0, sizeof(this->multicastSi) );

    this->rlen						= sizeof(this->remoteSi);

    // Set up sockaddr for receiving messages
    this->receiverSi.sin6_family		= AF_INET6;
    this->receiverSi.sin6_port			= htons(port);
    this->receiverSi.sin6_addr			= in6addr_any;

    // Set up sockaddr for sending messages
    this->multicastSi.sin6_family		= AF_INET6;
    this->multicastSi.sin6_port			= htons(port);
    inet_pton(AF_INET6, "ff03::1", &this->multicastSi.sin6_addr);

    // Get Interface index for wpan0
    unsigned int ifindex;
    ifindex = if_nametoindex("wpan0");

    // Set multicast interface for the socket
    if( -1 == setsockopt(this->sock, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                         &ifindex, sizeof(ifindex)))
    {
        throw std::runtime_error("Failed to set multicast interface.");
    }

    // Disable loopback so we don't receive messages sent by this host
    unsigned long loopback = 0; // long b/c `setsockopt` expects optlen of 4...
    if( -1 == setsockopt(this->sock, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                         &loopback, sizeof (loopback)))
    {
        throw std::runtime_error("Failed to disable loopback for socket.");
    }

    // Set receive timeout for the socket.
    // This way the gbrControl can be stopped cleanly if running as a daemon.
    struct timeval tv;
    tv.tv_sec	= 5;
    tv.tv_usec	= 0;
    if( -1 == setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<const char*>(&tv), sizeof(tv)))
    {
        throw std::runtime_error("Failed to set socket receive timeout.");
    }

    // Bind socket to port
    if( -1 == bind(this->sock, reinterpret_cast<struct sockaddr*>(&this->receiverSi),
                   sizeof (this->receiverSi)) )
    {
        throw std::runtime_error("Failed to bind to port.");
    }

    // Join multicast group "ff03::1" on the interface
    struct ipv6_mreq mreq;
    inet_pton( AF_INET6, "ff03::1", &mreq.ipv6mr_multiaddr.s6_addr);
    mreq.ipv6mr_interface = ifindex;

    if( -1 == setsockopt(this->sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                         reinterpret_cast<char*>(&mreq), sizeof(mreq)))
    {
        throw std::runtime_error("Failed to join multicast group.");
    }

}

gbrSocketListener::~gbrSocketListener()
{
    close(this->sock);
}

long gbrSocketListener::ListenForMessage()
{
    long	bytesReceived;
    char	messageBuf[BUFLEN];
    char	remoteBuf[INET6_ADDRSTRLEN];

    bytesReceived = recvfrom(this->sock, messageBuf, BUFLEN, 0,
                             reinterpret_cast<struct sockaddr*>(&this->remoteSi),
                             &this->rlen);
    // Ignore EAGAIN(11)
    // recvfrom returns EAGAIN if a timeout occurs
    if( -1 == bytesReceived && EAGAIN != errno)
    {
        throw std::runtime_error("Failed to receive message.");
    }
    else if( 0 < bytesReceived )
    {
        //Get the sender's IP-address
        inet_ntop(AF_INET6, &this->remoteSi.sin6_addr, remoteBuf, sizeof(this->remoteSi));

        messageBuf[bytesReceived] = '\0';
        this->mLastMessage	= messageBuf;
        this->mLastSender	= remoteBuf;
    }

    return bytesReceived;
}

int gbrSocketListener::SendMessage(std::string *message, sockaddr_in6 *sockIn)
{
    if( -1 == sendto(this->sock, message->c_str(), message->length(), 0,
                  reinterpret_cast<struct sockaddr*>(sockIn),
                  sizeof(*sockIn)) )
    {
        return 1;
    }
    return 0;
}


int gbrSocketListener::SendLocal(std::string *message)
{
    return SendMessage(message, &this->remoteSi);
}

int gbrSocketListener::SendMultiCast(std::string *message)
{
    return SendMessage(message, &this->multicastSi);
}

std::string gbrSocketListener::GetLastMessage() const
{
    return this->mLastMessage;
}

std::string gbrSocketListener::GetLastSender() const
{
    return this->mLastSender;
}
