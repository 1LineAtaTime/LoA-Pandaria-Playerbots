import tempfile
import threading
import time
import unittest
from pathlib import Path
from unittest.mock import patch
from pool import metadata_lock
import published_batch
from reference_schema import safe_body


class MigrationTests(unittest.TestCase):
    def test_alias_clone_cannot_write_other_schemas(self):
        self.assertTrue(safe_body('BEGIN SET NEW.`entry`=NEW.`CreatureID`; END'))
        self.assertFalse(safe_body('BEGIN UPDATE mop_loa_characters.characters SET money=0; END'))
        self.assertFalse(safe_body('BEGIN CALL other_database.procedure(); END'))

    def test_sql_strings_and_comments_are_not_table_references(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'x.sql';path.write_text("-- FROM fake\nUPDATE `npc_text` SET text='from my wee Tarren Mill';\nDELETE FROM creature WHERE guid=1;")
            with patch.object(published_batch,'ROOT',Path(d)):
                self.assertEqual(published_batch.tables([dict(name='x.sql')]),['creature','npc_text'])

    def test_parallel_metadata_writers_wait_for_short_owner(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'pool.lock';entered=threading.Event();finished=[]
            def owner():
                with metadata_lock(path):entered.set();time.sleep(.1)
            first=threading.Thread(target=owner);first.start();self.assertTrue(entered.wait(1))
            with metadata_lock(path):finished.append(True)
            first.join();self.assertEqual(finished,[True])


if __name__=='__main__':unittest.main()
