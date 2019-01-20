#include "gbrcontrol.h"
#include <stdexcept>
#include <iostream>
#include <csignal>
#include <cassert>

gbrControl::gbrControl(const char* database, unsigned short port)
{
    try {
        db = new gbrDatabaseHandler(database);
    } catch (std::runtime_error&) {
        throw;
    }

    try {
        listener = new gbrSocketListener(port);
    } catch (std::runtime_error&) {
        throw;
    }
}

gbrControl::~gbrControl()
{
    delete db; delete listener;
}

int gbrControl::Run()
{
    std::cout << "Starting gbrControl." << std::endl;
    running = true;

    // Build and multicast a request for info/config of the active nodes
    // in the mesh network.
    gbrXMLMessageType	initMessage	= gbrXMLMessageType::GETNODECONFIG;
    gbrXML				*getNodeXml	= new gbrXML(&initMessage);

    std::cout << "Requesting configuration from active nodes in the network." << std::endl;
    listener->SendMultiCast(getNodeXml->GetXML());
    delete getNodeXml;

    long				messageLength = 0;

    while (running) {
        try {
            messageLength = listener->ListenForMessage();
        } catch (std::runtime_error& e) {
            std::cout << "ListenForMessage() failed: " << e.what() << std::endl;
        }

        if( 0 < messageLength )
        {
            HandleNewMessage();
        }
    }

    std::cout << "Stopped" << std::endl;

    return 0;
}


int gbrControl::HandleNewMessage()
{
    std::string	xml = listener->GetLastMessage();
    gbrXML		*xmlReader;

    std::cout << "Received:" << std::endl << listener->GetLastMessage() << std::endl;

    try {
        xmlReader = new gbrXML(&xml);
    } catch (std::runtime_error& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    switch(xmlReader->GetType())
    {
    case gbrXMLMessageType::GETCONFIGS:
    {
        std::vector<NodeConfig>	configs;
        std::string				xml;

        std::cout << "Preparing to send active node configs." << std::endl;

        try {
            db->GetActiveNodes(&configs);
        } catch (std::runtime_error& e) {
            std::cout << e.what() << std::endl;
            return 1;
        }
        gbrXML xmlGenerator(&configs);

        std::cout << "Sending active node configs." << std::endl;
        listener->SendLocal(xmlGenerator.GetXML());

        break;
    }
    case gbrXMLMessageType::SETCONFIGS:
    {
        std::cout << "Received new configs." << std::endl;
        for( NodeConfig newConfig : *xmlReader->GetNodeConfigs() )
        {
            std::cout << "Storing new config for node: " << newConfig.eui64 << std::endl;
            try {
                if( DBResult::UPDATED == db->StoreNodeConfig(&newConfig) )
                {
                    std::cout << "Configuration for: " << newConfig.eui64 << " has changed. Sending new config." << std::endl;
                    gbrXML xmlGenerator(&newConfig);
                    std::cout << *xmlGenerator.GetXML() << std::endl;
                    listener->SendMultiCast(xmlGenerator.GetXML());
                }
            } catch (std::runtime_error& e) {
                std::cout << e.what() << std::endl;
                return 1;
            }
        }
        break;
    }
    case gbrXMLMessageType::NODECONFIG:
    {
        std::cout << "Received config from: " << xmlReader->GetNodeConfig()->eui64 << std::endl;
        try {
            db->StoreNodeConfig(xmlReader->GetNodeConfig());
        } catch (std::runtime_error& e) {
            std::cout << e.what() << std::endl;
            return 1;
        }
        break;
    }
    case gbrXMLMessageType::SENDSIGNAL:
    {
        std::cout << "Broadcasting signal: " << xmlReader->GetSignal()->signal << std::endl;
        gbrXML xmlGenerator(xmlReader->GetSignal());
        listener->SendMultiCast(xmlGenerator.GetXML());
        break;
    }
    default:
        break;
    }
    delete xmlReader;

    return 0;
}


void gbrControl::Stop()
{
    std::cout << "Stopping.." << std::endl;
    running = false;
}


void HandleSignal(int signal)
{
    switch(signal)
    {
    case SIGINT:
    case SIGTERM:
        gbrcontrol->Stop();
    }
}


gbrControl *gbrcontrol;

int main()
{

    try {
        gbrcontrol = new gbrControl("/var/local/gbr-control.db", 8012);
    } catch (std::runtime_error& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    signal(SIGINT, HandleSignal);
    signal(SIGTERM, HandleSignal);

    gbrcontrol->Run();

    delete gbrcontrol;

    return 0;
}
