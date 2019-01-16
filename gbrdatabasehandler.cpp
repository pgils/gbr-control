#include "gbrdatabasehandler.h"
#include <stdexcept>
#include <sstream>
#include <iostream>

gbrDatabaseHandler::gbrDatabaseHandler(const char* dbfile)
{
    if( SQLITE_OK != sqlite3_open_v2(dbfile, &dbHandle,
                                     SQLITE_OPEN_READWRITE, nullptr))
    {
        std::ostringstream oss;
        oss << "Error opening DB: " << dbfile << ": " <<
                     sqlite3_errmsg(dbHandle) << std::endl;

        sqlite3_close(dbHandle);
        throw std::runtime_error(oss.str());
    }
    // Enable Foreign keys
    sqlite3_exec(dbHandle, "PRAGMA foreign_keys = ON",
                                  nullptr, nullptr, nullptr);
}

gbrDatabaseHandler::~gbrDatabaseHandler()
{
    sqlite3_close(dbHandle);
}

int gbrDatabaseHandler::PrepareStatement(std::string sql, sqlite3_stmt **stmt)
{
    if( SQLITE_OK != sqlite3_prepare_v2(dbHandle, sql.c_str(), -1, stmt, nullptr) )
    {
        std::ostringstream oss;
        oss << "Error preparing statement: " << sql << ": " <<
                     sqlite3_errmsg(dbHandle) << std::endl;
        throw std::runtime_error(oss.str());
    }
    return 0;
}


DBResult gbrDatabaseHandler::GetNodeConfig(NodeConfig *conf)
{
    DBResult		ret;
    sqlite3_stmt	*stmt 	= nullptr;

    try {
        PrepareStatement("SELECT ipaddress, status_id, "
                         "role_id, signal_id FROM"
                         " tblNode WHERE eui64=?1", &stmt);
    } catch (const std::runtime_error&) {
        throw;
    }
    sqlite3_bind_int64(stmt, 1, conf->eui64);
    switch (sqlite3_step(stmt)) {
    case SQLITE_ROW:
        conf->ipaddress 	= std::string(reinterpret_cast<const char*>
                                    (sqlite3_column_text(stmt, 0)));
        conf->status		= sqlite3_column_int(stmt, 1);
        conf->role			= sqlite3_column_int(stmt, 2);
        conf->signal		= sqlite3_column_int(stmt, 3);

        GetNodeGroups(conf->eui64, &conf->groups);

        ret					= DBResult::OK;
        break;
    case SQLITE_DONE:
        ret					= DBResult::NOTFOUND;
        break;
    default:
        ret					= DBResult::NOOP;
        break;
    }
    sqlite3_finalize(stmt);

    return ret;
}

int gbrDatabaseHandler::GetNodeGroups(int64_t eui64, std::vector<int> *groups)
{
    sqlite3_stmt	*stmt	= nullptr;

    try {
        PrepareStatement("SELECT group_id FROM tblGroup_node WHERE node_eui64=?1", &stmt);
    } catch (const std::runtime_error&) {
        throw;
    }
    sqlite3_bind_int64(stmt, 1, eui64);

    while( SQLITE_OK == sqlite3_step(stmt) )
    {
        groups->push_back(sqlite3_column_int(stmt, 0));
    }
    sqlite3_finalize(stmt);

    return 0;
}


int gbrDatabaseHandler::GetSignalTargets(std::vector<NodeConfig> *configs,
                                       std::vector<int> groups)
{
    std::string		sql;
    sqlite3_stmt	*stmt	= nullptr;
    ulong			i;

    sql = "SELECT DISTINCT eui64,ipaddress,status_id,role_id,signal_id "
          "FROM tblNode INNER JOIN tblGroup_node ON "
          "tblGroup_node.node_eui64=tblNode.eui64 WHERE tblGroup_node.group_id IN (?";
    //TODO: check if 0 < i < 999 (SQLite bind_ limit)
    for (i = 0; i < groups.size()-1; ++i) {
        sql.append(",?");
    }
    sql.append(")");
    try {
        PrepareStatement(sql, &stmt);
    } catch (const std::runtime_error&) {
        throw;
    }

    for (i = 0; i < groups.size(); ++i) {
        sqlite3_bind_int(stmt, int(i+1), groups.at(i));
    }

    while( SQLITE_ROW == sqlite3_step(stmt) )
    {
        NodeConfig	conf;
        conf.eui64			= sqlite3_column_int64(stmt, 0);
        conf.ipaddress		= std::string(reinterpret_cast<const char*>
                                          (sqlite3_column_text(stmt, 1)));
        conf.status			= sqlite3_column_int(stmt, 2);
        conf.role			= sqlite3_column_int(stmt, 3);
        conf.signal			= sqlite3_column_int(stmt, 4);
        configs->push_back(conf);
    }
    sqlite3_finalize(stmt);

    return 0;
}

int gbrDatabaseHandler::GetActiveNodes(std::vector<NodeConfig> *configs)
{
    std::string		sql;
    sqlite3_stmt	*stmt	= nullptr;

    sql = "SELECT eui64,ipaddress,status_id,role_id,signal_id "
          "FROM tblNode WHERE active=1";
    try {
        PrepareStatement(sql, &stmt);
    } catch (const std::runtime_error&) {
        throw;
    }
    while( SQLITE_ROW == sqlite3_step(stmt) )
    {
        NodeConfig	conf;
        conf.eui64			= sqlite3_column_int64(stmt, 0);
        conf.ipaddress		= std::string(reinterpret_cast<const char*>
                                          (sqlite3_column_text(stmt, 1)));
        conf.status			= sqlite3_column_int(stmt, 2);
        conf.role			= sqlite3_column_int(stmt, 3);
        conf.signal			= sqlite3_column_int(stmt, 4);

        GetNodeGroups(conf.eui64, &conf.groups);
        configs->push_back(conf);
    }
    sqlite3_finalize(stmt);

    return 0;
}

// TODO: remember to set active before calling this
DBResult gbrDatabaseHandler::StoreNodeConfig(NodeConfig *conf)
{
    DBResult		ret		= DBResult::NOOP;
    sqlite3_stmt	*stmt 	= nullptr;
    NodeConfig		tmpConf = *conf;

    switch (GetNodeConfig(conf)) {
    case DBResult::OK:
        ret					= ( tmpConf == *conf ) ? DBResult::NOCHANGE : DBResult::UPDATED;
    break;
    case DBResult::NOTFOUND:
        //TODO: always enter as uninitialized?
        try {
            PrepareStatement("INSERT INTO tblNode (eui64, ipaddress, active, status_id, role_id, signal_id)"
                            " VALUES(?1, ?2, ?3, ?4, ?5, ?6)", &stmt);
        } catch (std::runtime_error&) {
            throw;
        }
        sqlite3_bind_int64	(stmt, 1, conf->eui64);
        sqlite3_bind_text	(stmt, 2, conf->ipaddress.c_str(),
                            int(conf->ipaddress.length()), SQLITE_TRANSIENT);
        sqlite3_bind_int	(stmt, 3, conf->active);
        sqlite3_bind_int	(stmt, 4, conf->status);
        sqlite3_bind_int	(stmt, 5, conf->role);
        sqlite3_bind_int	(stmt, 6, conf->signal);

        sqlite3_step(stmt); //TODO: handle this?
        sqlite3_finalize(stmt);

        for( int group : conf->groups )
        {
            CreateGroup(group);
            AddNodeToGroup(conf->eui64, group);
        }

        ret = DBResult::INSERTED;
    break;
    default:
        ret = DBResult::NOOP;
        break;
    }

    if( DBResult::UPDATED == ret )
    {
        std::ostringstream sql;
        sql << "UPDATE tblNode SET ";
        // IP-address isn't a mandatory element. (GUI doesn't send ip-addresses)
        // so only update it if one is provided.
        if( 0 < tmpConf.ipaddress.length() )
        {
            sql << "ipaddress=?1, ";
        }
        sql << "status_id=?2, role_id=?3, signal_id=?4 ";
        sql << "WHERE eui64=?5";

        try {
           PrepareStatement(sql.str(), &stmt);
        } catch (const std::runtime_error&) {
            throw;
        }
        sqlite3_bind_text(stmt, 1, tmpConf.ipaddress.c_str(),
                int(tmpConf.ipaddress.length()), SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, tmpConf.status);
        sqlite3_bind_int(stmt, 3, tmpConf.role);
        sqlite3_bind_int(stmt, 4, tmpConf.signal);
        sqlite3_bind_int64(stmt, 5, tmpConf.eui64);
        sqlite3_step(stmt);
        try {
            std::string msg		= "UPDATE-ing node: ";
            msg.append(std::to_string(tmpConf.eui64));

            FinalizeStatement(msg, &stmt);
        } catch (std::runtime_error&) {
            throw;
        }

        RemoveNodeFromAllGroups(tmpConf.eui64);
        for( int group : tmpConf.groups )
        {
            CreateGroup(group);
            AddNodeToGroup(tmpConf.eui64, group);
        }
    }
    return ret;
}

int gbrDatabaseHandler::DeleteNode(int64_t eui64)
{
    sqlite3_stmt	*stmt	= nullptr;
    int				sqliteRet;

    try {
        PrepareStatement("DELETE FROM tblNode WHERE eui64=?1", &stmt);
    } catch (std::runtime_error&) {
        throw;
    }
    sqlite3_bind_int64(stmt, 1, eui64);

    sqliteRet = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return sqliteRet;
}

int gbrDatabaseHandler::CreateGroup(int group, bool remove)
{
    std::string		sql;
    sqlite3_stmt	*stmt	= nullptr;
    int				sqliteRet;

    sql = remove ? "DELETE FROM tblGroup WHERE group_id=?1"
                 : "INSERT OR IGNORE INTO tblGroup (group_id, groupname) VALUES (?1, ?1)";

    try {
        PrepareStatement(sql, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }
    sqlite3_bind_int(stmt, 1, group);

    sqliteRet = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return sqliteRet;
}

int gbrDatabaseHandler::RemoveGroup(int group)
{
    return CreateGroup(group, true);
}

int gbrDatabaseHandler::AddNodeToGroup(int64_t eui64, int group, bool remove)
{
    std::string		sql;
    sqlite3_stmt	*stmt	= nullptr;
    int				sqliteRet;

    sql = remove ? "DELETE FROM tblGroup_node WHERE group_id=?1 AND node_eui64=?2"
                 : "INSERT INTO tblGroup_node (group_id, node_eui64) VALUES (?1, ?2)";

    try {
        PrepareStatement(sql, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }
    sqlite3_bind_int(stmt, 1, group);
    sqlite3_bind_int64(stmt, 2, eui64);

    sqliteRet = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return sqliteRet;
}
int gbrDatabaseHandler::RemoveNodeFromGroup(int64_t eui64, int group)
{
    return AddNodeToGroup(eui64, group, true);
}

int gbrDatabaseHandler::RemoveNodeFromAllGroups(int64_t eui64)
{
    std::string		sql;
    sqlite3_stmt	*stmt	= nullptr;
    int				sqliteRet;

    sql 		= "DELETE FROM tblGroup_node WHERE node_eui64=?1";

    try {
        PrepareStatement(sql, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }
    sqlite3_bind_int64(stmt, 1, eui64);

    sqliteRet	= sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return sqliteRet;
}
