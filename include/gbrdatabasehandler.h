#ifndef GBRDATABASEHANDLER_H
#define GBRDATABASEHANDLER_H


#include <sqlite3.h>
#include <vector>
#include <string>
#include "gbrxml.h"

#define		NODE_TIMEOUT	100	// timeout is seconds

enum class DBResult
{
    INSERTED,
    UPDATED,
    NOCHANGE,
    OK,
    NOTFOUND,
    ERROR,
    NOOP,
};

class gbrDatabaseHandler
{
public:
    gbrDatabaseHandler( const char* dbfile );
    virtual ~gbrDatabaseHandler();

private:
    gbrDatabaseHandler( const gbrDatabaseHandler& );
    gbrDatabaseHandler& operator=( const gbrDatabaseHandler& );

private:
    sqlite3 *dbHandle;

    int PrepareStatement(const std::string sql, sqlite3_stmt **stmt);
    int FinalizeStatement(const std::string message, sqlite3_stmt **stmt);

public:
    DBResult StoreNodeConfig(NodeConfig *conf);
    DBResult GetNodeConfig(NodeConfig *conf);
    int GetNodeConfigs(std::vector<NodeConfig> *configs);
    int SetNodeLastSeen(NodeConfig *conf);
    int TimeoutNodes();
    int GetNodeIndex(std::string eui64);
    int GetNodeGroups(std::string eui64, std::vector<int> *groups);
    int DeleteNode(std::string eui64);
    int CreateGroup(int group, bool remove = false);
    int RemoveGroup(int group);
    int AddNodeToGroup(std::string eui64, int group, bool remove = false);
    int RemoveNodeFromGroup(std::string eui64, int group);
    int RemoveNodeFromAllGroups(std::string eui64);
    int GetSignalTargets(std::vector<NodeConfig> *configs, std::vector<int> groups);
    int GetActiveNodes(std::vector<NodeConfig> *configs);

};

#endif // GBRDATABASEHANDLER_H
