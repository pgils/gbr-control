#include <stdexcept>
#include <iostream>
#include <csignal>
#include <cassert>
#include "gbrdatabasehandler.h"
#include "gbrxml.h"

int HandleNewMessage(const std::string newXML, gbrDatabaseHandler *db)
{
    std::string	xml = newXML;
    gbrXML		*xmlReader;
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

        try {
            db->GetActiveNodes(&configs);
        } catch (std::runtime_error& e) {
            std::cout << e.what() << std::endl;
            return 1;
        }
        gbrXML xmlGenerator(&configs);

        break;
    }
    case gbrXMLMessageType::SETCONFIGS:
    {
        for( NodeConfig newConfig : *xmlReader->GetNodeConfigs() )
        {
            try {
                db->StoreNodeConfig(&newConfig);
            } catch (std::runtime_error& e) {
                std::cout << e.what() << std::endl;
                return 1;
            }
        }
        break;
    }
    case gbrXMLMessageType::NODECONFIG:
    {
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
        gbrXML xmlGenerator(xmlReader->GetSignal());
        break;
    }
    default:
        break;
    }

    if(nullptr != xmlReader) { delete xmlReader; }

    return 0;
}


int main()
{
    gbrDatabaseHandler *db;
    try {
        db = new gbrDatabaseHandler("/tmp/gbrTest.db");
    } catch (std::runtime_error&) {
        throw;
    }

    std::string strNodeConfig;
    strNodeConfig =
        "<messagetype>nodeconfig</messagetype>"
        "<node>"
            "<eui64>0011deadbeefcafe</eui64>"
            "<status>2</status>"
            "<role>2</role>"
            "<groups>"
                "<group>2</group>"
                "<group>25</group>"
            "</groups>"
            "<signal>2</signal>"
        "</node>";
    HandleNewMessage(strNodeConfig, db);

    strNodeConfig =
        "<messagetype>nodeconfig</messagetype>"
        "<node>"
            "<eui64>abcdef1122334455</eui64>"
            "<status>1</status>"
            "<role>1</role>"
            "<groups>"
                "<group>4</group>"
                "<group>5</group>"
            "</groups>"
            "<signal>3</signal>"
        "</node>";
    HandleNewMessage(strNodeConfig, db);

    strNodeConfig =
        "<messagetype>nodeconfig</messagetype>"
        "<node>"
            "<eui64>abcdef1122334455</eui64>"
            "<status>1</status>"
            "<role>1</role>"
            "<groups>"
                "<group>4</group>"
                "<group>5</group>"
                "<group>7</group>"
            "</groups>"
            "<signal>3</signal>"
        "</node>";
    HandleNewMessage(strNodeConfig, db);

    strNodeConfig =
        "<messagetype>nodeconfig</messagetype>"

        "<node>"
        "<eui64></eui64>"
        "<status>0</status>"
        "<role>0</role>"
        "<groups>"
            "<group>0</group>"
        "</groups>"
        "<signal>0</signal>"
        "</node>";
    HandleNewMessage(strNodeConfig, db);

    strNodeConfig =
        "<messagetype>nodeconfig</messagetype>"

        "<node>"
        "<status>0</status>"
        "<role>0</role>"
        "<groups>"
            "<group>0</group>"
        "</groups>"
        "<signal>0</signal>"
        "</node>";
    HandleNewMessage(strNodeConfig, db);

    std::cout << "TEST COMPLETE." << std::endl;

    delete db;
}
