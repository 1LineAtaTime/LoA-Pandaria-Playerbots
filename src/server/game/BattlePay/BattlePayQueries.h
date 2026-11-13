/* GNU GPL version 2 or later; see COPYING.md. */
#ifndef BATTLE_PAY_QUERIES_H
#define BATTLE_PAY_QUERIES_H

namespace BattlePayQueries
{
    // Kept as complete statements so the opt-in database tests execute the
    // exact production charge/refund operations, including their predicates.
    constexpr char Charge[] = R"SQL(UPDATE account a INNER JOIN battlepay_purchase p ON a.id=p.account_id SET a.dp=a.dp-p.price,p.state=1,p.updated_at=CURRENT_TIMESTAMP WHERE p.id=%llu AND p.account_id=%u AND p.realm_id=%u AND p.state=0 AND p.created_at>=CURRENT_TIMESTAMP-INTERVAL 5 MINUTE AND a.dp>=0 AND a.dp>=p.price AND p.price<=2147483647)SQL";
    constexpr char Refund[] = R"SQL(UPDATE account a INNER JOIN battlepay_purchase p ON a.id=p.account_id SET a.dp=a.dp+p.price,p.state=4,p.updated_at=CURRENT_TIMESTAMP WHERE p.id=%llu AND p.account_id=%u AND p.state=1 AND a.dp>=0 AND a.dp<=2147483647-p.price)SQL";
    constexpr char TokenCredit[] = R"SQL(UPDATE account a INNER JOIN battlepay_token_credit t ON a.id=t.account_id SET a.dp=a.dp+t.amount,t.applied=1 WHERE t.realm_id=%u AND t.item_guid=%u AND t.account_id=%u AND t.amount=%llu AND t.applied=0 AND a.dp>=0 AND a.dp<=2147483647-t.amount)SQL";
}
#endif
