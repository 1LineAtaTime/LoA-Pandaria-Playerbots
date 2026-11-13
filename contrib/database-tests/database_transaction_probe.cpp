/* Licensed under the GNU General Public License, version 2 or later. */
#include "MySQLConnection.h"
#include "DatabaseWorkerPool.h"
#include "Implementation/LoginDatabase.h"
#include "Implementation/CharacterDatabase.h"
#ifdef PLAYERBOTS
#include "Implementation/PlayerbotsDatabase.h"
#endif
#include "MySQLPreparedStatement.h"
#include "PreparedStatement.h"
#include "Field.h"
#include "QueryResult.h"
#include "Transaction.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

class ProbeConnection final : public MySQLConnection
{
public:
    explicit ProbeConnection(MySQLConnectionInfo& info) : MySQLConnection(info, CONNECTION_SYNCH) { }

private:
    void DoPrepareStatements() override
    {
        // Match production pools: reconnect reuses the original slot layout.
        if (!m_reconnecting)
            m_stmts.resize(1);
        PrepareStatement(0, "UPDATE mop_core_transaction_probe SET dp=dp-? WHERE id=1 AND dp>=?", CONNECTION_SYNCH);
    }
};

template<class T>
bool PreparePool(std::string const& info, char const* name)
{
    DatabaseWorkerPool<T> pool;
    pool.SetConnectionInfo(info, 1, 1);
    if (pool.Open())
        return false;
    bool const success = pool.PrepareStatements();
    pool.Close();
    std::cout << (success ? "PASS: " : "FAIL: ") << name << " actual prepared statements\n";
    return success;
}

int main(int argc, char** argv)
{
    if ((argc != 3 && argc != 4) || std::string_view(argv[2]).substr(0, 9) != "mop_test_" ||
        (argc == 4 && std::string_view(argv[3]) != "--schema-pools"))
    {
        std::cerr << "usage: database_transaction_probe PORT mop_test_DATABASE [--schema-pools]\n";
        return EXIT_FAILURE;
    }
    unsigned long const port = std::strtoul(argv[1], nullptr, 10);
    if (!port || port > 65535)
        return EXIT_FAILURE;
    // Exercise the actual client minimum/thread-safety/header-DLL checks too.
    DatabaseWorkerPool<LoginDatabaseConnection> clientValidation;
    char const* user = std::getenv("MOP_TEST_DB_USER");
    char const* password = std::getenv("MOP_TEST_DB_PASSWORD");
    std::string const prefix = "127.0.0.1;" + std::to_string(port) + ";" + (user ? user : "root") + ";" + (password ? password : "") + ";";
    MySQLConnectionInfo info(prefix + argv[2]);
    ProbeConnection connection(info);
    if (connection.Open() ||
        !connection.Execute("CREATE TABLE IF NOT EXISTS mop_core_transaction_probe (id INT PRIMARY KEY, dp INT NOT NULL) ENGINE=InnoDB") ||
        !connection.Execute("DELETE FROM mop_core_transaction_probe") ||
        !connection.Execute("INSERT INTO mop_core_transaction_probe VALUES (1,7)") || !connection.PrepareStatements())
        return EXIT_FAILURE;
    auto balance = [&]() -> int
    {
        std::unique_ptr<ResultSet> result(connection.Query("SELECT dp FROM mop_core_transaction_probe WHERE id=1"));
        return result && result->NextRow() ? (*result)[0].GetInt32() : -1;
    };
    auto guarded = std::make_shared<Transaction<ProbeConnection>>();
    guarded->Append("UPDATE mop_core_transaction_probe SET dp=0 WHERE id=1");
    guarded->AppendChecked("INSERT INTO mop_core_transaction_probe SELECT 2,0 WHERE 1=0", 1);
    if (!connection.ExecuteTransaction(guarded) || balance() != 7)
        return EXIT_FAILURE;

    auto duplicate = std::make_shared<Transaction<ProbeConnection>>();
    duplicate->Append("UPDATE mop_core_transaction_probe SET dp=0 WHERE id=1");
    duplicate->Append("INSERT INTO mop_core_transaction_probe VALUES (1,99)");
    if (!connection.ExecuteTransaction(duplicate) || balance() != 7)
        return EXIT_FAILURE;

    PreparedStatement<ProbeConnection> debit(0, 2);
    debit.setUInt32(0, 5); debit.setUInt32(1, 5);
    if (!connection.Execute(&debit, false) || connection.GetAffectedRows() != 1 || balance() != 2 ||
        !connection.Execute(&debit, false) || connection.GetAffectedRows() != 0)
        return EXIT_FAILURE;
    if (!connection.Execute("UPDATE mop_core_transaction_probe SET dp=7 WHERE id=1"))
        return EXIT_FAILURE;

    std::unique_ptr<ResultSet> idResult(connection.Query("SELECT CONNECTION_ID()"));
    if (!idResult || !idResult->NextRow())
        return EXIT_FAILURE;
    std::string const kill = "KILL " + std::to_string((*idResult)[0].GetUInt64());
    idResult.reset();
    auto interrupted = std::make_shared<Transaction<ProbeConnection>>();
    interrupted->Append("UPDATE mop_core_transaction_probe SET dp=0 WHERE id=1");
    interrupted->Append(kill.c_str());
    if (!connection.ExecuteTransaction(interrupted) || balance() != 7)
        return EXIT_FAILURE;
    if (!connection.Execute(&debit, false) || connection.GetAffectedRows() != 1 || balance() != 2)
        return EXIT_FAILURE;
    auto success = std::make_shared<Transaction<ProbeConnection>>();
    success->AppendChecked("UPDATE mop_core_transaction_probe SET dp=3 WHERE id=1", 1);
    if (connection.ExecuteTransaction(success) || balance() != 3)
        return EXIT_FAILURE;
    // Lose an idle connection so the next prepared Execute itself must clean
    // parameters, reconnect, rebuild statement slots, and retry safely.
    idResult.reset(connection.Query("SELECT CONNECTION_ID()"));
    if (!idResult || !idResult->NextRow()) return EXIT_FAILURE;
    std::string const idleKill = "KILL " + std::to_string((*idResult)[0].GetUInt64());
    idResult.reset();
    ProbeConnection killer(info);
    if (killer.Open() || !killer.Execute(idleKill.c_str())) return EXIT_FAILURE;
    debit.setUInt32(0, 1); debit.setUInt32(1, 1);
    if (!connection.Execute(&debit) || balance() != 2) return EXIT_FAILURE;
    if (!connection.Execute("DROP TABLE mop_core_transaction_probe"))
        return EXIT_FAILURE;
    std::cout << "PASS: core affected-row rollback, duplicate rollback, prepared affected rows, interrupted transaction rollback/reconnect, checked commit\n";
    if (argc == 4)
    {
        bool success = PreparePool<LoginDatabaseConnection>(prefix + "mop_test_auth", "Login");
        success = PreparePool<CharacterDatabaseConnection>(prefix + "mop_test_characters", "Character") && success;
#ifdef PLAYERBOTS
        success = PreparePool<PlayerbotsDatabaseConnection>(prefix + "mop_test_playerbots", "Playerbots") && success;
#endif
        if (!success)
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
