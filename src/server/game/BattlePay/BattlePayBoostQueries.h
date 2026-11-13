/* GNU GPL version 2 or later; see COPYING.md. */
#ifndef BATTLE_PAY_BOOST_QUERIES_H
#define BATTLE_PAY_BOOST_QUERIES_H
namespace BattlePayBoostQueries
{
    constexpr char Reserve[] = R"SQL(UPDATE account_boost b INNER JOIN battlepay_boost_use u ON b.id=u.account_id AND b.realmid=u.entitlement_realm SET b.counter=b.counter-1,u.state=1 WHERE u.id=%llu AND u.account_id=%u AND u.realm_id=%u AND u.state=0 AND b.counter>0)SQL";
    constexpr char Restore[] = R"SQL(UPDATE account_boost b INNER JOIN battlepay_boost_use u ON b.id=u.account_id AND b.realmid=u.entitlement_realm SET b.counter=b.counter+1,u.state=3 WHERE u.id=%llu AND u.account_id=%u AND u.realm_id=%u AND u.state=1 AND b.counter<4294967295)SQL";
}
#endif
