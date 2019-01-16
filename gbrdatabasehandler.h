#ifndef GBRDATABASEHANDLER_H
#define GBRDATABASEHANDLER_H


#include <sqlite3.h>
#include <vector>
#include <string>
#include "gbrxml.h"
//TODO: MUTEX?
//TODO: recursion
//TODO: does the add/remove group work?

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

public:
    DBResult StoreNodeConfig(NodeConfig *conf);
    DBResult GetNodeConfig(NodeConfig *conf);
    int GetNodeGroups(int64_t eui64, std::vector<int> *groups);
    int DeleteNode(int64_t eui64);
    int CreateGroup(int group, bool remove = false);
    int RemoveGroup(int group);
    int AddNodeToGroup(int64_t eui64, int group, bool remove = false);
    int RemoveNodeFromGroup(int64_t eui64, int group);
    int RemoveNodeFromAllGroups(int64_t eui64);
    int GetSignalTargets(std::vector<NodeConfig> *configs, std::vector<int> groups);
    int GetActiveNodes(std::vector<NodeConfig> *configs);

};

#endif // GBRDATABASEHANDLER_H
