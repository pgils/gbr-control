#include <iostream>
#include <stdexcept>
#include <vector>
#include "gbrdatabasehandler.h"

//TODO: specifiy output

using namespace std;

int main()
{
    gbrDatabaseHandler *db;
    db = nullptr;
    try {
        db = new gbrDatabaseHandler("/tmp/test.db");
    } catch (const std::runtime_error& e) {
        std::cout << e.what() << std::endl;
        //TODO: cleanup
        return 1;
    }

    // Test creating nodes
    NodeConfig node1 = { 0x00B40, "2001::cafe:db30/64", 1, 1, 1, 0 };
    NodeConfig node2 = { 0x00F34, "2001::cafe:7de1/64", 1, 2, 2, 1 };
    NodeConfig node3 = { 0x00999, "2001::cafe:4c0f/64", 0, 1, 1, 2 };
    NodeConfig node4 = { 0x00677, "2001::cafe:944f/64", 1, 2, 1, 2 };
    NodeConfig node5 = { 0x00122, "2001::cafe:4fb7/64", 1, 1, 1, 1 };

    db->StoreNodeConfig(&node1);
    db->StoreNodeConfig(&node2);
    db->StoreNodeConfig(&node3);
    db->StoreNodeConfig(&node4);
    db->StoreNodeConfig(&node5);

    //Test modifying nodes
    node2.ipaddress = "2001::cafe:c035/64";
    db->StoreNodeConfig(&node2);

    // Test adding nodes to groups
    db->AddNodeToGroup(0x00B40, 1);
    db->AddNodeToGroup(0x00B40, 2);
    db->AddNodeToGroup(0x00B40, 3);
    db->AddNodeToGroup(0x00999, 2);
    db->AddNodeToGroup(0x00999, 3);
    db->AddNodeToGroup(0x00677, 5);
    db->AddNodeToGroup(0x00677, 6);
    db->AddNodeToGroup(0x00677, 256);

    // Test removing nodes from group
    db->RemoveNodeFromGroup(0x00B40, 2);

    // Test deleting node (and DB cascading)
    db->DeleteNode(0x00B40);

    // Test Getting signal targets
    vector<NodeConfig> configs;
    vector<int> groups;

    groups.push_back(1);
    groups.push_back(2);
    groups.push_back(3);
    groups.push_back(5);
    groups.push_back(13);
    groups.push_back(256);

    db->GetSignalTargets(&configs, groups);

    for ( NodeConfig c : configs )
    {
        std::cout << c.eui64 << " : " << c.ipaddress << std::endl;
    }

    if( db != nullptr )
        delete db;

    return 0;
}
