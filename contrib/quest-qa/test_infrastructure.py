import json
import sqlite3
import struct
import tempfile
import time
import unittest
from pathlib import Path
from unittest.mock import Mock
from catalogue_matrix import level_cases,playable_pairs
from pool import Pool
from protocol import QuestPackets


class InfrastructureTests(unittest.TestCase):
    def test_deterministic_selection_skips_persistent_body_of_same_pair(self):
        with tempfile.TemporaryDirectory() as d:
            p=Pool.__new__(Pool);p.path=Path(d)/'pool.json';p.validate=Mock()
            persistent=dict(guid=120,race=2,**{'class':1},purpose='persistent-adventure',snapshot_sha256='old')
            normal=dict(guid=5,race=2,**{'class':1},snapshot_sha256='normal')
            p.path.write_text(json.dumps(dict(characters=[persistent,normal])))
            self.assertEqual(p.ensure(2,1)['guid'],5)
            self.assertEqual(p._ensure(2,1)['guid'],5)

    def test_persistent_adventure_is_protected_from_deterministic_reset(self):
        with tempfile.TemporaryDirectory() as d:
            p=Pool.__new__(Pool);p.path=Path(d)/'pool.json';p.db=Mock()
            char=dict(guid=120,account_id=12,purpose='persistent-adventure')
            p.path.write_text(json.dumps(dict(characters=[char])))
            p.require_lease=Mock();p.validate=Mock()
            with self.assertRaisesRegex(RuntimeError,'Persistent adventure'):
                p.reset(dict(guid=120,account_id=12))
            p.db.q.assert_not_called()

    def test_full_account_allocates_another_instead_of_twelfth_character(self):
        class Selected(Exception):pass
        for count,expected in [(10,1),(11,2)]:
            p=Pool.__new__(Pool);p.state=dict(accounts=[dict(id=1)],characters=[],token='abcd')
            p.db=Mock();p.db.q.return_value=[[str(count)]];p.registry=Mock();p.registry.execute.return_value=[]
            def account(index):
                if index==1:p.state['accounts'].append(dict(id=2))
                return p.state['accounts'][index]
            def lease(a):
                self.assertEqual(a['id'],expected);raise Selected()
            p.account=account;p.lease=lease
            with self.assertRaises(Selected):p._ensure(1,1)

    def test_level_boundaries_are_unique_and_include_negative_limits(self):
        self.assertEqual(list(level_cases(1,0)),[('at_minimum',1),('at_maximum',90)])
        self.assertEqual(dict(level_cases(20,30)),dict(at_minimum=20,at_maximum=30,below_minimum=19,above_maximum=31))
        self.assertEqual([n for _,n in level_cases(30,30)],[30,29,31])

    def test_client_pairs_use_race_class_bytes_not_row_identifier(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'CharBaseInfo.dbc'
            p.write_bytes(struct.pack('<4s4I',b'WDBC',2,3,8,0)+struct.pack('<IBB2x',999,22,4)+struct.pack('<IBB2x',1000,99,1))
            self.assertEqual(playable_pairs(p),[(22,4)])
            p.write_bytes(b'WDBC')
            with self.assertRaises(struct.error):playable_pairs(p)

    def test_offer_requires_decodable_packet(self):
        p=QuestPackets();w=Mock()
        with self.assertRaises(ValueError):p.observe(w,0x134c,b'')
        self.assertFalse(p.offered)
        p.observe(w,0x134c,b'\0'*88+struct.pack('<I',14212))
        self.assertEqual(p.offered,{14212})

    def test_expired_or_foreign_lease_cannot_reset(self):
        p=Pool.__new__(Pool);p.registry=sqlite3.connect(':memory:');p.owned_leases={1:'ours'}
        p.registry.execute('CREATE TABLE leases(account_id INTEGER PRIMARY KEY,owner TEXT,expires REAL)')
        p.registry.execute('INSERT INTO leases VALUES(1,?,?)',('other',time.time()+60))
        with self.assertRaises(RuntimeError):p.require_lease(1)
        p.registry.execute('UPDATE leases SET owner=?,expires=?',('ours',time.time()-1))
        with self.assertRaises(RuntimeError):p.require_lease(1)
        p.registry.execute('UPDATE leases SET expires=?',(time.time()+60,))
        p.require_lease(1);p.heartbeat(1)
        self.assertGreater(p.registry.execute('SELECT expires FROM leases').fetchone()[0],time.time()+800)

    def test_sql_snapshot_preserves_binary_and_converts_numeric_strings(self):
        self.assertEqual(Pool.literal('N'),'NULL')
        self.assertEqual(Pool.literal('H3130','int'),'CONVERT(X\'3130\' USING utf8mb4)')
        self.assertEqual(Pool.literal('H00ff','blob'),"X'00ff'")


if __name__=='__main__':unittest.main()
