/* Licensed under the GNU General Public License, version 2 or later. */
#ifdef _WIN32
#include <winsock2.h>
#endif
#include <mysql.h>
#include "DatabaseServerVersion.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

// Requires an explicitly named disposable database and loopback endpoint.
// Password is read from the environment, never from argv or printed.
int main(int argc, char** argv)
{
    if (argc != 3 || std::strncmp(argv[2], "mop_test_", 9))
    {
        std::cerr << "usage: database_connector_probe PORT mop_test_DATABASE\n";
        return EXIT_FAILURE;
    }
    unsigned long port = std::strtoul(argv[1], nullptr, 10);
    if (!port || port > 65535)
        return EXIT_FAILURE;
    using Connection = std::unique_ptr<MYSQL, decltype(&mysql_close)>;
    Connection connection(mysql_init(nullptr), &mysql_close);
    if (!connection)
        return EXIT_FAILURE;
    mysql_options(connection.get(), MYSQL_SET_CHARSET_NAME, "utf8mb4");
    char const* password = std::getenv("MOP_TEST_DB_PASSWORD");
    char const* user = std::getenv("MOP_TEST_DB_USER");
    if (!mysql_real_connect(connection.get(), "127.0.0.1", user ? user : "root", password ? password : "", argv[2], static_cast<unsigned>(port), nullptr, 0))
    {
        std::cerr << "connect failed: " << mysql_error(connection.get()) << '\n';
        return EXIT_FAILURE;
    }
    auto query = [&](char const* sql)
    {
        if (mysql_query(connection.get(), sql))
        {
            std::cerr << "query failed: " << mysql_error(connection.get()) << '\n';
            return false;
        }
        return true;
    };
    if (mysql_set_character_set(connection.get(), "utf8mb4") || !query("SELECT VERSION(), @@version_comment, @@character_set_connection"))
        return EXIT_FAILURE;
    MYSQL_RES* identity = mysql_store_result(connection.get());
    MYSQL_ROW row = identity ? mysql_fetch_row(identity) : nullptr;
    auto version = row && row[0] && row[1] ? DatabaseServer::ParseVersion(row[0], row[1]) : std::nullopt;
    bool const charset = row && row[2] && std::strcmp(row[2], "utf8mb4") == 0;
    if (version)
        std::cout << "connector=" << mysql_get_client_info() << " handshake-id=" << mysql_get_server_version(connection.get())
            << " server=" << row[0] << " family="
            << (version->Product == DatabaseServer::Family::MariaDB ? "MariaDB" : "MySQL") << " id=" << version->Number << '\n';
    if (identity)
        mysql_free_result(identity);
    if (!version || !version->IsSupported() || !charset)
        return EXIT_FAILURE;

    // Temporary tables never alter account data or permanent application tables.
    if (!query("CREATE TEMPORARY TABLE connector_probe (id INT NOT NULL PRIMARY KEY, dp INT NOT NULL, text_value VARCHAR(20) CHARACTER SET utf8mb4) ENGINE=InnoDB") ||
        !query("INSERT INTO connector_probe VALUES (1, 7, '')"))
        return EXIT_FAILURE;
    using Statement = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>;
    Statement statement(mysql_stmt_init(connection.get()), &mysql_stmt_close);
    char const* sql = "UPDATE connector_probe SET dp=dp-?, text_value=? WHERE id=? AND dp>=?";
    if (!statement || mysql_stmt_prepare(statement.get(), sql, static_cast<unsigned long>(std::strlen(sql))))
        return EXIT_FAILURE;
    int price = 5, id = 1;
    char text[] = "\xF0\x9F\x98\x80";
    unsigned long textLength = 4;
    MYSQL_BIND binds[4]{};
    binds[0].buffer_type = MYSQL_TYPE_LONG; binds[0].buffer = &price;
    binds[1].buffer_type = MYSQL_TYPE_STRING; binds[1].buffer = text; binds[1].buffer_length = 4; binds[1].length = &textLength;
    binds[2].buffer_type = MYSQL_TYPE_LONG; binds[2].buffer = &id;
    binds[3].buffer_type = MYSQL_TYPE_LONG; binds[3].buffer = &price;
    if (mysql_stmt_bind_param(statement.get(), binds) || mysql_stmt_execute(statement.get()) || mysql_stmt_affected_rows(statement.get()) != 1)
        return EXIT_FAILURE;
    if (mysql_stmt_execute(statement.get()) || mysql_stmt_affected_rows(statement.get()) != 0)
        return EXIT_FAILURE;
    if (!query("START TRANSACTION") || !query("UPDATE connector_probe SET dp=0 WHERE id=1") || !query("ROLLBACK") ||
        !query("SELECT dp, HEX(text_value) FROM connector_probe WHERE id=1"))
        return EXIT_FAILURE;
    MYSQL_RES* result = mysql_store_result(connection.get());
    row = result ? mysql_fetch_row(result) : nullptr;
    bool const success = row && row[0] && row[1] && std::strcmp(row[0], "2") == 0 && std::strcmp(row[1], "F09F9880") == 0;
    if (result)
        mysql_free_result(result);
    if (!success)
        return EXIT_FAILURE;
    std::cout << "PASS: runtime identity, utf8mb4 roundtrip, binary prepared statement, guarded debit, rollback\n";
    return EXIT_SUCCESS;
}
