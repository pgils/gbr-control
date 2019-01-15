// INET6 socket listener
// based on the example:
// https://www.binarytides.com/programming-udp-sockets-c-linux/


#include "gbrsocketlistener.h"

#include <stdexcept>
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
    memset( reinterpret_cast<char*>(&this->localSi), 0, sizeof(this->localSi) );
    memset( reinterpret_cast<char*>(&this->multicastSi), 0, sizeof(this->multicastSi) );

    this->slen						= sizeof(this->senderSi);

    // Set up sockaddr for receiving messages
    this->localSi.sin6_family		= AF_INET6;
    this->localSi.sin6_port			= htons(port);
    this->localSi.sin6_addr			= in6addr_any;

    // Set up sockaddr for multicasting
    this->multicastSi.sin6_family	= AF_INET6;
    this->multicastSi.sin6_port		= htons(port);
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

    // Bind socket to port
    if( -1 == bind(this->sock, reinterpret_cast<struct sockaddr*>(&this->localSi),
                   sizeof (this->localSi)) )
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

int gbrSocketListener::ListenForMessage()
{
    long	bytesReceived;
    char	messageBuf[BUFLEN];
    char	senderBuf[INET6_ADDRSTRLEN];

    if( -1 == (bytesReceived = recvfrom(this->sock, messageBuf, BUFLEN, 0,
                                        reinterpret_cast<struct sockaddr*>(&this->senderSi),
                                        &this->slen)) )
    {
        throw std::runtime_error("Failed to receive message.");
    }

    //Get the sender's IP-address
    inet_ntop(AF_INET6, &this->senderSi.sin6_addr, senderBuf, sizeof(this->senderSi));

    this->mLastMessage	= messageBuf;
    this->mLastSender	= senderBuf;

    return 0;
}

int gbrSocketListener::SendMultiCast(std::string *message)
{
    if( -1 == sendto(this->sock, message->c_str(), message->length(), 0,
                  reinterpret_cast<struct sockaddr*>(&this->multicastSi),
                  sizeof(this->multicastSi)) )
    {
        return 1;
    }
    return 0;
}

std::string gbrSocketListener::GetLastMessage() const
{
    return this->mLastMessage;
}

std::string gbrSocketListener::GetLastSender() const
{
    return this->mLastSender;
}
