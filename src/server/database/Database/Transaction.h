/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _TRANSACTION_H
#define _TRANSACTION_H

#include "Define.h"
#include "DatabaseEnvFwd.h"
#include "StringFormat.h"
#include <functional>
#include <mutex>
#include <optional>
#include <variant>
#include <vector>

class MySQLConnection;

struct TransactionData
{
    std::variant<std::unique_ptr<PreparedStatementBase>, std::string> query;
    std::optional<uint64> expectedAffectedRows;

    template<typename... Args>
    TransactionData(Args&&... args) : query(std::forward<Args>(args)...) { }

    static PreparedStatementBase* ToExecutable(std::unique_ptr<PreparedStatementBase> const& stmt) { return stmt.get(); }
    static char const* ToExecutable(std::string const& sql) { return sql.c_str(); }
};

/*! Transactions, high level class. */
class TC_DATABASE_API TransactionBase
{
    friend class TransactionTask;
    friend class MySQLConnection;

    template <typename T>
    friend class DatabaseWorkerPool;

    public:
        TransactionBase() : _cleanedUp(false) { }
        TransactionBase(TransactionBase const&) = delete;
        TransactionBase(TransactionBase &&) noexcept = default;
        TransactionBase& operator=(TransactionBase const&) = delete;
        TransactionBase& operator=(TransactionBase &&) noexcept = default;        
        virtual ~TransactionBase() { Cleanup(); }

        void Append(char const* sql);
        // Roll back the entire transaction if this statement does not affect
        // exactly the expected row count (for ownership/idempotency guards).
        void AppendChecked(char const* sql, uint64 expectedAffectedRows);
        template<typename Format, typename... Args>
        void PAppend(Format&& sql, Args&&... args)
        {
            Append(Trinity::StringFormat(std::forward<Format>(sql), std::forward<Args>(args)...).c_str());
        }

        std::size_t GetSize() const { return m_queries.size(); }

    protected:
        void AppendPreparedStatement(PreparedStatementBase* statement);
        void Cleanup();
        std::vector<TransactionData> m_queries;

    private:
        bool _cleanedUp;
};

template<typename T>
class Transaction : public TransactionBase
{
public:
    using TransactionBase::Append;
    void Append(PreparedStatement<T>* statement)
    {
        AppendPreparedStatement(statement);
    }
};

/*! Low level class*/
class TC_DATABASE_API TransactionTask
{
public:
    static bool Execute(MySQLConnection* conn, std::shared_ptr<TransactionBase> trans);

private:
    static int TryExecute(MySQLConnection* conn, std::shared_ptr<TransactionBase> trans);

    static std::mutex _deadlockLock;
};

class TC_DATABASE_API TransactionCallback
{
public:
    TransactionCallback(TransactionFuture&& future) : m_future(std::move(future)) { }
    TransactionCallback(TransactionCallback&&) = default;

    TransactionCallback& operator=(TransactionCallback&&) = default;

    void AfterComplete(std::function<void(bool)> callback) &
    {
        m_callback = std::move(callback);
    }

    bool InvokeIfReady();

    TransactionFuture m_future;
    std::function<void(bool)> m_callback;
};

#endif
