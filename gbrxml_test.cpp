#include <iostream>
#include <string>
#include <sstream>
#include <tinyxml2.h>
#include "gbrxml.h"

std::ostream& operator<<(std::ostream& stream, const NodeConfig& conf)
{
        stream << "EUI-64	: " << conf.eui64 << std::endl;
        stream << "IP		: " << conf.ipaddress << std::endl;
        stream << "Status	: " << conf.status << std::endl;
        stream << "Role		: " << conf.status << std::endl;
        stream << "Signal	: " << conf.signal << std::endl;
        for( int group : conf.groups )
        {
            stream << "Group	: " << group << std::endl;
        }
        return stream;
}

int main()
{
    std::string xml = "<messagetype>getnodeconfig</messagetype>";
    gbrXML *x1 = new gbrXML(&xml);

    std::cout << "xml1: " << *x1->GetXML() << std::endl;

    std::vector<int> groups = {2, 3, 5};
    NodeConfig node1 = { 0x00B40, "2001::cafe:db30/64", 1, 1, groups, 1, 0 };

    std::cout << node1 << std::endl;
    gbrXML *x2 = new gbrXML(&node1);

    std::cout << "ip2: " << x2->GetNodeConfig()->ipaddress << std::endl;
    std::cout << "xml2: " << *x2->GetXML() << std::endl;

    gbrXMLMessageType type = gbrXMLMessageType::GETCONFIGS;
    gbrXML *x3 = new gbrXML(&type);

    std::cout << "xml3: " << *x3->GetXML() << std::endl;

    NodeConfig node2 = { 0x00F34, "2001::cafe:7de1/64", 1, 2, groups, 2, 1 };
    NodeConfig node3 = { 0x00999, "2001::cafe:4c0f/64", 0, 1, groups, 1, 2 };
    NodeConfig node4 = { 0x00677, "2001::cafe:944f/64", 1, 2, groups, 1, 2 };
    NodeConfig node5 = { 0x00122, "2001::cafe:4fb7/64", 1, 1, groups, 1, 1 };

    std::vector<NodeConfig> configs = { node1, node2, node3, node4, node5 };

    gbrXML *x4 = new gbrXML(&configs);

    std::cout << "xml4: " << *x4->GetXML() << std::endl;

    Signal sig = { 3, groups };

    gbrXML *x5 = new gbrXML(&sig);

    std::cout << "xml5: " << *x5->GetXML() << std::endl;

    std::string xml6 = "<messagetype>signal</messagetype>"
                       "<signal>2</signal>"
                       "<groups>"
                       "<group>4</group>"
                       "<group>7</group>"
                       "<group>13</group>"
                       "</groups>";
    gbrXML *x6 = new gbrXML(&xml6);

    std::cout << "signal: " << x6->GetSignal()->signal << std::endl;
    for( int group : x6->GetSignal()->groups )
    {
        std::cout << "group: " << group << std::endl;
    }
    std::string xml7 = "<messagetype>signal</messagetype>";
    gbrXML *x7;
    try {
        x7 = new gbrXML(&xml7);
    } catch (std::runtime_error& e) {
        std::cout << e.what() << std::endl;
    }

    std::string xml8 = "<messagetype>nodeconfig</messagetype>"
                       "<node>"
                       "<eui64>555</eui64>"
                       "<status>1</status>"
                       "<role>2</role>"
                       "<signal>0</signal>"
                       "</node>";
    gbrXML *x8;
    try {
        x8 = new gbrXML(&xml8);
    } catch (std::runtime_error& e) {
        std::cout << e.what() << std::endl;
    }

    std::cout << *x8->GetNodeConfig() << std::endl;

    std::string xml9 = "<messagetype>nodeconfig</messagetype>"
                       "<node>"
                       "<eui64>666</eui64>"
                       "<ipaddress>ff22:dead:beef/64</ipaddress>"
                       "<status>1</status>"
                       "<role>2</role>"
                       "<groups>"
                       "<group>40</group>"
                       "</groups>"
                       "<signal>0</signal>"
                       "</node>";
    gbrXML *x9 = new gbrXML(&xml9);

    std::cout << *x9->GetNodeConfig() << std::endl;

    std::string xml10 =
            "<messagetype>setconfigs</messagetype>"
            "<node>"
            "<eui64>123</eui64>"
            "<status>1</status>"
            "<role>1</role>"
            "<signal>5</signal>"
            "<groups>"
            "<group>2</group>"
            "<group>72</group>"
            "</groups>"
            "</node>"
            "<node>"
            "<eui64>456</eui64>"
            "<status>2</status>"
            "<role>2</role>"
            "<signal>0</signal>"
            "</node>"
            "<node>"
            "<eui64>66666</eui64>"
            "<status>2</status>"
            "<role>1</role>"
            "<signal>10</signal>"
            "<groups>"
            "<group>8</group>"
            "</groups>"
            "<ipaddress>ff22:dead:beef:cafe:2019::1/64</ipaddress>"
            "</node>";

    gbrXML *x10 = new gbrXML(&xml10);

    for( NodeConfig conf : *x10->GetNodeConfigs() )
    {
        std::cout << conf << std::endl;
    }

    std::string xml11 = "<mess";
    gbrXML *x11 = new gbrXML(&xml11);
    std::cout << "Type: " << static_cast<int>(x11->GetType()) << std::endl;


    std::string xml12 = "neessagetype>getconfigs</messagetype>";
    gbrXML *x12 = new gbrXML(&xml12);
    std::cout << "Type: " << static_cast<int>(x12->GetType()) << std::endl;

    std::string xml13 = "<messagetype></messagetype>";
    gbrXML *x13 = new gbrXML(&xml13);
    std::cout << "Type: " << static_cast<int>(x13->GetType()) << std::endl;

    delete x1; delete x2; delete x3; delete x4; delete x5;
    delete x6; delete x8; delete x9; delete x10;
    delete x11; delete x12; delete x13;


    std::cout << "TEST COMPLETE" << std::endl;

    return 0;
}
