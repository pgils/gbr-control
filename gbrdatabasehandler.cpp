#include "gbrdatabasehandler.h"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <ctime>

gbrDatabaseHandler::gbrDatabaseHandler(const char* dbfile)
{
    if( SQLITE_OK != sqlite3_open_v2(dbfile, &dbHandle,
                                     SQLITE_OPEN_READWRITE, nullptr))
    {
        std::ostringstream oss;
        oss << "ERROR opening DB: " << dbfile << ": " <<
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
        oss << "ERROR preparing statement: " << sql << ": " <<
                     sqlite3_errmsg(dbHandle) << std::endl;
        throw std::runtime_error(oss.str());
    }
    return 0;
}


int gbrDatabaseHandler::FinalizeStatement(const std::string message, sqlite3_stmt **stmt)
{
    switch(sqlite3_finalize(*stmt))
    {
    case SQLITE_OK:
    case SQLITE_DONE:
        return 0;
    default:
        std::ostringstream oss;
        oss << "ERROR " << message << ": ";
        oss << sqlite3_errmsg(dbHandle) << std::endl;

        throw std::runtime_error(oss.str());
    }
}


DBResult gbrDatabaseHandler::GetNodeConfig(NodeConfig *conf)
{
    DBResult		ret;
    sqlite3_stmt	*stmt 	= nullptr;

    try {
        PrepareStatement("SELECT status_id, "
                         "role_id, signal_id FROM"
                         " tblNode WHERE eui64=?1", &stmt);
    } catch (const std::runtime_error&) {
        throw;
    }
    sqlite3_bind_text(stmt, 1, conf->eui64.c_str(), int(conf->eui64.length()), SQLITE_TRANSIENT);
    switch (sqlite3_step(stmt)) {
    case SQLITE_ROW:
        conf->status		= sqlite3_column_int(stmt, 0);
        conf->role			= sqlite3_column_int(stmt, 1);
        conf->signal		= sqlite3_column_int(stmt, 2);

        GetNodeGroups(conf->eui64, &conf->groups);

        ret					= DBResult::OK;
        break;
    case SQLITE_DONE:
        ret					= DBResult::NOTFOUND;
        break;
    default:
        ret					= DBResult::NOOP;
    }
    try {
        std::string msg		= "SELECT-ing nodeconfig for eui64: ";
        msg.append(conf->eui64);

        FinalizeStatement(msg, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }

    return ret;
}

int gbrDatabaseHandler::GetNodeIndex(std::string eui64)
{
    int				eui64_id;
    std::string		sql;
    sqlite3_stmt	*stmt	= nullptr;

    sql = "SELECT eui64_id FROM tblNode WHERE eui64=?1 LIMIT 1";

    try {
        PrepareStatement(sql, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }

    sqlite3_bind_text(stmt, 1, eui64.c_str(), int(eui64.length()), SQLITE_TRANSIENT);

    if( SQLITE_ROW == sqlite3_step(stmt) )
    {
        eui64_id = sqlite3_column_int(stmt, 0);
    } else {
        throw std::runtime_error("node does not exist: " + eui64 + " : " + sqlite3_errmsg(dbHandle));
    }

    try {
        std::string msg = "SELECT-ing id for EUI-64: " + eui64;
        FinalizeStatement(msg, &stmt);
    } catch (std::runtime_error&) {
       throw;
    }

    return eui64_id;
}


int gbrDatabaseHandler::GetNodeGroups(std::string eui64, std::vector<int> *groups)
{
    sqlite3_stmt	*stmt	= nullptr;
    int				eui64_id;

    try {
        PrepareStatement("SELECT group_id FROM tblGroup_node WHERE eui64_id=?1", &stmt);
        eui64_id = GetNodeIndex(eui64);
    } catch (const std::runtime_error&) {
        throw;
    }
    sqlite3_bind_int(stmt, 1, eui64_id);

    // clear the vector first, to prevent duplicates.
    groups->clear();
    while( SQLITE_ROW == sqlite3_step(stmt) )
    {
        groups->push_back(sqlite3_column_int(stmt, 0));
    }
    try {
        std::string msg		= "SELECT-ing groups for eui64: ";
        msg.append(eui64);

        FinalizeStatement(msg, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }

    return 0;
}


int gbrDatabaseHandler::GetSignalTargets(std::vector<NodeConfig> *configs,
                                       std::vector<int> groups)
{
    std::string		sql;
    sqlite3_stmt	*stmt	= nullptr;
    ulong			i;

    sql = "SELECT DISTINCT eui64,status_id,role_id,signal_id "
          "FROM tblNode INNER JOIN tblGroup_node ON "
          "tblGroup_node.eui64_id=tblNode.eui64_id WHERE tblGroup_node.group_id IN (?";
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
        conf.eui64			= std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        conf.status			= sqlite3_column_int(stmt, 1);
        conf.role			= sqlite3_column_int(stmt, 2);
        conf.signal			= sqlite3_column_int(stmt, 3);
        configs->push_back(conf);
    }
    try {
        std::string msg		= "SELECT-ing signal targets: ";

        FinalizeStatement(msg, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }

    return 0;
}

int gbrDatabaseHandler::GetActiveNodes(std::vector<NodeConfig> *configs)
{
    std::string		sql;
    sqlite3_stmt	*stmt	= nullptr;

    sql = "SELECT eui64,status_id,role_id,signal_id "
          "FROM tblNode WHERE lastseen > ?1";
    try {
        PrepareStatement(sql, &stmt);
    } catch (const std::runtime_error&) {
        throw;
    }
    sqlite3_bind_int64(stmt, 1, time(nullptr));

    while( SQLITE_ROW == sqlite3_step(stmt) )
    {
        NodeConfig	conf;
        conf.eui64			= std::string(reinterpret_cast<const char*>
                                          (sqlite3_column_text(stmt, 0)));
        conf.status			= sqlite3_column_int(stmt, 1);
        conf.role			= sqlite3_column_int(stmt, 2);
        conf.signal			= sqlite3_column_int(stmt, 3);

        GetNodeGroups(conf.eui64, &conf.groups);
        configs->push_back(conf);
    }
    try {
        std::string msg		= "SELECT-ing active nodes";

        FinalizeStatement(msg, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }

    return 0;
}

DBResult gbrDatabaseHandler::StoreNodeConfig(NodeConfig *conf)
{
    DBResult		ret		= DBResult::NOOP;
    sqlite3_stmt	*stmt 	= nullptr;
    NodeConfig		tmpConf	= *conf;

    switch (GetNodeConfig(&tmpConf)) {
    case DBResult::OK:
        ret					= ( tmpConf == *conf ) ? DBResult::NOCHANGE : DBResult::UPDATED;
    break;
    case DBResult::NOTFOUND:
        //TODO: always enter as uninitialized?
        try {
            PrepareStatement("INSERT INTO tblNode (eui64, status_id, role_id, signal_id)"
                            " VALUES(?1, ?2, ?3, ?4)", &stmt);
        } catch (std::runtime_error&) {
            throw;
        }
        sqlite3_bind_text	(stmt, 1, conf->eui64.c_str(),
                            int(conf->eui64.length()), SQLITE_TRANSIENT);
        sqlite3_bind_int	(stmt, 2, conf->status);
        sqlite3_bind_int	(stmt, 3, conf->role);
        sqlite3_bind_int	(stmt, 4, conf->signal);

        sqlite3_step(stmt);
        try {
            std::string msg		= "INSERT-ing node: ";
            msg.append(conf->eui64);

            FinalizeStatement(msg, &stmt);
        } catch (std::runtime_error&) {
            throw;
        }

        RemoveNodeFromAllGroups(conf->eui64);
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
        sql << "status_id=?2, role_id=?3, signal_id=?4 ";
        sql << "WHERE eui64=?5";

        try {
           PrepareStatement(sql.str(), &stmt);
        } catch (const std::runtime_error&) {
            throw;
        }
        sqlite3_bind_text(stmt, 5, conf->eui64.c_str(),
                int(conf->eui64.length()), SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, conf->status);
        sqlite3_bind_int(stmt, 3, conf->role);
        sqlite3_bind_int(stmt, 4, conf->signal);
        sqlite3_step(stmt);
        try {
            std::string msg		= "UPDATE-ing node: ";
            msg.append(conf->eui64);

            FinalizeStatement(msg, &stmt);
        } catch (std::runtime_error&) {
            throw;
        }

        RemoveNodeFromAllGroups(conf->eui64);
        for( int group : conf->groups )
        {
            CreateGroup(group);
            AddNodeToGroup(conf->eui64, group);
        }
    }
        return ret;
}

int gbrDatabaseHandler::TimeoutNodes()
{
    sqlite3_stmt	*stmt	= nullptr;

    try {
        PrepareStatement("UPDATE tblNode SET status_id=1 WHERE lastseen < ?2", &stmt);
    } catch (std::runtime_error&) {
        throw;
    }
    sqlite3_bind_int64(stmt, 2, time(nullptr)-NODE_TIMEOUT);

    sqlite3_step(stmt);

    try {
        FinalizeStatement("Timing out nodes.", &stmt);
    } catch (std::runtime_error&) {
        throw;
    }

    return 0;
}

int gbrDatabaseHandler::SetNodeLastSeen(NodeConfig *conf)
{
    sqlite3_stmt	*stmt	= nullptr;

    try {
        PrepareStatement("UPDATE tblNode SET lastseen=?1 WHERE eui64=?2", &stmt);
    } catch (std::runtime_error&) {
        throw;
    }
    sqlite3_bind_text(stmt, 2, conf->eui64.c_str(), int(conf->eui64.length()), SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 1, time(nullptr));

    sqlite3_step(stmt);
    try {
        FinalizeStatement("UPDATE-ing `lastseen` for eui64: " + conf->eui64, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }

    return 0;
}

int gbrDatabaseHandler::DeleteNode(std::string eui64)
{
    sqlite3_stmt	*stmt	= nullptr;

    try {
        PrepareStatement("DELETE FROM tblNode WHERE eui64=?1", &stmt);
    } catch (std::runtime_error&) {
        throw;
    }
    sqlite3_bind_text(stmt, 1, eui64.c_str(), int(eui64.length()), SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    try {
        std::string msg		= "DELETE-ing node: ";
        msg.append(eui64);

        FinalizeStatement(msg, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }

    return 0;
}

int gbrDatabaseHandler::CreateGroup(int group, bool remove)
{
    std::string		sql;
    sqlite3_stmt	*stmt	= nullptr;

    sql = remove ? "DELETE FROM tblGroup WHERE group_id=?1"
                 : "INSERT OR IGNORE INTO tblGroup (group_id) VALUES (?1)";

    try {
        PrepareStatement(sql, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }
    sqlite3_bind_int(stmt, 1, group);

    sqlite3_step(stmt);
    try {
        std::string msg = remove ? "DELETE-ing" : "INSERT-ing";
        msg.append(" group: ");
        msg.append(std::to_string(group));

        FinalizeStatement(msg, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }

    return 0;
}

int gbrDatabaseHandler::RemoveGroup(int group)
{
    return CreateGroup(group, true);
}


int gbrDatabaseHandler::AddNodeToGroup(std::string eui64, int group, bool remove)
{
    std::string		sql;
    sqlite3_stmt	*stmt	= nullptr;
    int				eui64_id;

    sql = remove ? "DELETE FROM tblGroup_node WHERE group_id=?1 AND eui64_id=?2"
                 : "INSERT INTO tblGroup_node (group_id, eui64_id) VALUES (?1, ?2)";

    try {
        PrepareStatement(sql, &stmt);
        eui64_id = GetNodeIndex(eui64);
    } catch (std::runtime_error&) {
        throw;
    }
    sqlite3_bind_int(stmt, 1, group);
    sqlite3_bind_int(stmt, 2, eui64_id);

    sqlite3_step(stmt);
    try {
        std::string msg = remove ? "DELETE-ing" : "INSERT-ing";
        msg.append(" node: ");
        msg.append(eui64);
        msg.append(" to group: ");
        msg.append(std::to_string(group));

        FinalizeStatement(msg, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }

    return 0;
}
int gbrDatabaseHandler::RemoveNodeFromGroup(std::string eui64, int group)
{
    return AddNodeToGroup(eui64, group, true);
}

int gbrDatabaseHandler::RemoveNodeFromAllGroups(std::string eui64)
{
    std::string		sql;
    sqlite3_stmt	*stmt	= nullptr;
    int				eui64_id;

    sql 		= "DELETE FROM tblGroup_node WHERE eui64_id=?1";

    try {
        PrepareStatement(sql, &stmt);
        eui64_id = GetNodeIndex(eui64);
    } catch (std::runtime_error&) {
        throw;
    }
    sqlite3_bind_int(stmt, 1, eui64_id);

    sqlite3_step(stmt);
    try {
        std::string msg = "DELETE-ing from all groups: ";
        msg.append(eui64);

        FinalizeStatement(msg, &stmt);
    } catch (std::runtime_error&) {
        throw;
    }

    return 0;
}
