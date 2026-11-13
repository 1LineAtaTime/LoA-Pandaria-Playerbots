import unittest
from finalize_historical_quest_compatibility import matching_assembly


class AssemblyBindingTests(unittest.TestCase):
    def test_new_assembly_replaces_old_cached_plan(self):
        current={'sql_sha256':'new'}
        self.assertIs(matching_assembly(current,{'sql_sha256':'old'},'new'),current)

    def test_repeat_finalization_uses_matching_original(self):
        original={'sql_sha256':'raw'}
        self.assertIs(matching_assembly({'identity_restore_fields':10},original,'raw'),original)

    def test_unreviewed_sql_cannot_inherit_other_plan(self):
        with self.assertRaises(RuntimeError):
            matching_assembly({'identity_restore_fields':10},{'sql_sha256':'reviewed'},'different')


if __name__=='__main__':unittest.main()
