#include <iostream>
#include <string>
#include "gbrsocketlistener.h"


int main()
{
    gbrSocketListener *listener = new gbrSocketListener(8012);

    std::string msg = "Hello Mesh!";
    listener->SendMultiCast(&msg);

    listener->ListenForMessage();

    std::cout << "Received from: " << listener->GetLastSender() << " : " <<
                 listener->GetLastMessage() << std::endl;

    delete listener;

    return 0;
}
