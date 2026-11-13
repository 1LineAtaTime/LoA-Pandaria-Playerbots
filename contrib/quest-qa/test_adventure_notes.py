import tempfile,unittest,json,sqlite3
from types import SimpleNamespace
from unittest.mock import patch
from pathlib import Path
from adventure_notes import Notes,describe
from adventure_campaign import release_dead_worker_lease


class LiveNotesTests(unittest.TestCase):


    def test_dead_worker_cleanup_preserves_replacement_and_live_leases(self):
        with tempfile.TemporaryDirectory() as d:
            episode=Path(d);db=sqlite3.connect(':memory:')
            db.execute('CREATE TABLE leases(account_id INTEGER PRIMARY KEY,owner TEXT,expires REAL)')
            db.execute('INSERT INTO leases VALUES(12,?,9999999999)',('newer-owner',));db.commit()
            (episode/'worker-control.json').write_text(json.dumps({'worker_pid':50,'account_id':12,'lease_owner':'old-owner'}))
            pool=SimpleNamespace(registry=db)
            with patch('adventure_campaign.process_alive',return_value=False):
                release_dead_worker_lease(pool,episode,50)
                self.assertEqual(db.execute('SELECT owner FROM leases').fetchone()[0],'newer-owner')
                db.execute('UPDATE leases SET owner=?',('old-owner',));db.commit()
                release_dead_worker_lease(pool,episode,50)
                self.assertFalse(db.execute('SELECT owner FROM leases').fetchall())
            db.close()

    def test_notes_distinguish_completion_reward_and_suspected_failure(self):
        self.assertIn('awaiting turn-in',describe(dict(event='quest_state_changed',quest=25127,actual=1)))
        self.assertIn('reward persisted',describe(dict(event='quest_rewarded',quest=25127)))
        self.assertIn('REVIEW:',describe(dict(event='suspected_stall',reason='stationary_without_progress',seconds_without_progress=181)))
        self.assertIsNone(describe(dict(event='controller_command',command='botscenario save')))

    def test_journal_is_readable_and_preserves_one_record_per_line(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'notes.md';n=Notes(p,'test',120)
            n.record(dict(event='equipment_changed',timestamp='2026-11-10T10:00:00Z',slot=7,item=53362,previous=None))
            n.record(dict(event='failure',timestamp='2026-11-10T10:01:00Z',error='Example\nproblem'))
            n.close();text=p.read_text(encoding='utf-8')
            self.assertIn('Equipped item 53362 in feet',text)
            self.assertIn('FAILURE: Example problem',text)
            with self.assertRaises(FileExistsError):Notes(p,'test',120)



if __name__=='__main__':unittest.main()
