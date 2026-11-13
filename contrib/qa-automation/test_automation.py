import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
from checkpoint import validate_plan, private_path
from diagnostics import parse, compare, normalize
from prioritize import rank
from pipeline import data_comparison, choose_suites, validate_gameplay_contract
from evidence import write_json
from run_lock import RunLock
from recover_accounts import recover
from evidence import write_json


class AutomationTests(unittest.TestCase):
    def test_removed_gameplay_contract_is_rejected(self):
        with self.assertRaises(ValueError):validate_gameplay_contract(dict(scenarios=[]),[],['was-passing'])

    def test_untested_cannot_replace_previous_gameplay_pass(self):
        with self.assertRaises(ValueError):validate_gameplay_contract(dict(scenarios=[dict(id='q')]),[dict(id='q',status='untested')],['q'])

    def test_incomplete_gameplay_results_are_rejected(self):
        with self.assertRaises(ValueError):validate_gameplay_contract(dict(scenarios=[dict(id='q')]),[],[])

    def test_planned_account_without_identity_is_not_deleted(self):
        from unittest.mock import Mock
        with tempfile.TemporaryDirectory() as d:
            write_json(Path(d)/'trace-0.json',dict(account='QZABCDEF',name='Qzabcdef',account_created=False))
            db=Mock();db.q.return_value=[['1']];command=Mock()
            with self.assertRaisesRegex(ValueError,'without confirmed'):recover(db,d,command)
            command.assert_not_called()
    def test_recovery_refuses_reused_account_id(self):
        from unittest.mock import Mock
        with tempfile.TemporaryDirectory() as d:
            write_json(Path(d)/'trace-0.json',dict(account='QZABCDEF',name='Qzabcdef',account_created=True,account_id=1))
            db=Mock();db.q.return_value=[['2']];command=Mock()
            with self.assertRaisesRegex(ValueError,'identity changed'):recover(db,d,command)
            command.assert_not_called()

    def test_recovery_refuses_prefix_only_without_identity(self):
        from unittest.mock import Mock
        with tempfile.TemporaryDirectory() as d:
            write_json(Path(d)/'trace-0.json',dict(account='QZABCDEF',name='Qzabcdef',account_created=True))
            command=Mock()
            with self.assertRaises(ValueError):recover(Mock(),d,command)
            command.assert_not_called()

    def test_recovery_refuses_unexpected_character(self):
        from unittest.mock import Mock
        with tempfile.TemporaryDirectory() as d:
            write_json(Path(d)/'trace-0.json',dict(account='QZABCDEF',name='Qzabcdef',account_created=True,account_id=1))
            db=Mock();db.q.side_effect=[[['1']],[['42','Otherplayer']]];command=Mock()
            with self.assertRaisesRegex(ValueError,'Unexpected character'):recover(db,d,command)
            command.assert_not_called()
    def test_parallel_runs_block_and_release(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'run.lock'
            with RunLock(path):
                with self.assertRaises(RuntimeError):
                    with RunLock(path):pass
            with RunLock(path):pass
    def test_atomic_evidence_retries_sharing_violation(self):
        with tempfile.TemporaryDirectory() as d:
            with patch('evidence.os.replace',side_effect=[PermissionError(),PermissionError(),None]) as replace,patch('evidence.time.sleep'):
                write_json(Path(d)/'run.json',{'status':'passed'})
                self.assertEqual(replace.call_count,3)

    def test_atomic_evidence_does_not_hide_persistent_failure(self):
        with tempfile.TemporaryDirectory() as d:
            with patch('evidence.os.replace',side_effect=PermissionError()),patch('evidence.time.sleep'):
                with self.assertRaises(PermissionError):write_json(Path(d)/'run.json',{})
    def test_backup_identifier_injection_rejected(self):
        with self.assertRaises(ValueError):validate_plan([dict(database='World',table='x;DELETE',key='id',ids=[1])])

    def test_backup_ids_are_typed_and_bounded(self):
        for ids in ([],['1 OR 1'],[True],[-1],[1,1],[2**64]):
            with self.assertRaises(ValueError):validate_plan([dict(database='World',table='x',key='id',ids=ids)])

    def test_full_snapshot_and_row_selectors_do_not_mix(self):
        with self.assertRaises(ValueError):validate_plan([dict(database='World',full=True,table='x')])

    def test_backup_schema_cannot_be_arbitrary(self):
        with self.assertRaises(ValueError):validate_plan([dict(database='mysql',full=True)])

    def test_private_output_cannot_escape_out(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaises(ValueError):private_path(d,Path(d)/'doc/private')
            self.assertEqual(private_path(d,Path(d)/'out/a'),(Path(d)/'out/a').resolve())

    def test_sql_values_redacted_and_deadlock_attributed(self):
        rows=parse("2026-11-08_10:00:00 ERROR [sql.sql] SQL(p): INSERT INTO account (name) VALUES ('private-name')\n [ERROR]: [1213] Deadlock found\n")
        self.assertNotIn('private-name',str(rows));self.assertEqual(rows[0]['category'],'database_deadlock')

    def test_guid_is_normalized_but_spell_id_is_not(self):
        self.assertEqual(normalize('GUID 2 spell 42'),'GUID <guid> spell 42')
        self.assertNotEqual(normalize('GUID 2 spell 42'),normalize('GUID 3 spell 43'))

    def test_recurring_warning_is_not_a_new_signature(self):
        row=parse('x WARN [maps] missing spell 42')
        a={'a':dict(records=row)};b={'a':dict(records=[dict(row[0],count=3)])}
        r=compare(a,b);self.assertFalse(r['new_signatures']);self.assertEqual(r['recurring'][0]['additional_occurrences'],2)

    def test_multiple_appenders_do_not_double_count(self):
        row=parse('x ERROR [maps] missing spell 42')
        r=compare({}, {'a':dict(records=row),'b':dict(records=row)})
        self.assertEqual(r['new_signatures'][0]['count'],1)

    def test_replaced_failure_cannot_hide_behind_same_count(self):
        r=data_comparison({'a':[['2']]},dict(audits=[dict(id='a',rows=[['1']])]))
        self.assertFalse(r['regression_free'])

    def test_lost_audit_is_not_a_pass(self):
        self.assertFalse(data_comparison({},dict(audits=[dict(id='a',rows=[])]))['regression_free'])

    def test_full_profile_includes_all_system_suites(self):
        self.assertEqual(len(choose_suites('full',[])),4)

    def test_targeted_core_change_adds_invalid_request_regression(self):
        self.assertIn('invalid_requests',choose_suites('targeted',['src/QuestHandler.cpp']))

    def test_spawned_starter_before_large_unspawned_group(self):
        q=[dict(quest='1',quest_sort='154',min_level='1',title='Starter')]
        base=dict(quests='1',fixed_spawn_count='2',status='native_binding_review',quest_count='1',source='s',target_id='1')
        out=rank(q,[dict(base,fixed_spawn_count='0',quest_count='100',source='a'),base])
        self.assertEqual(out[0]['source'],'s')

    def test_retirement_needs_provenance(self):
        with self.assertRaises(ValueError):rank([],[],{'1':dict(status='retired')})

    def test_known_data_debt_does_not_claim_ready(self):
        r=data_comparison({'a':[['1']]},dict(audits=[dict(id='a',rows=[['1']])]))
        self.assertTrue(r['regression_free']);self.assertFalse(r['content_ready'])


if __name__=='__main__':unittest.main()
