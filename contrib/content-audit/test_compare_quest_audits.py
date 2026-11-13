import json
import tempfile
import unittest
from pathlib import Path
from compare_quest_audits import compare, load


class ComparisonTests(unittest.TestCase):
    def audit(self, findings=None, quests=(1,)):
        return dict(summary={'version': 1}, quests=set(quests), findings=findings or [])

    def finding(self, target=42):
        return dict(quest=1, issue='missing_target_definition', role='objective_0',
                    target_type='creature', target_id=target, source='fixture', status='structural_failure')

    def test_unchanged_baseline_passes(self):
        self.assertTrue(compare(self.audit([self.finding()]), self.audit([self.finding()]))['regression_free'])

    def test_equal_counts_cannot_hide_replaced_failure(self):
        r = compare(self.audit([self.finding(42)]), self.audit([self.finding(43)]))
        self.assertFalse(r['regression_free'])
        self.assertEqual(len(r['new_findings']), 1)

    def test_removed_quest_cannot_improve_audit(self):
        self.assertFalse(compare(self.audit(quests=(1, 2)), self.audit())['regression_free'])

    def test_no_gameplay_pass_inferred_from_removed_finding(self):
        r = compare(self.audit([self.finding()]), self.audit())
        self.assertTrue(r['regression_free'])
        self.assertFalse(r['gameplay_certified'])

    def test_added_content_without_failure_is_allowed(self):
        self.assertTrue(compare(self.audit(), self.audit(quests=(1, 2)))['regression_free'])

    def test_format_mismatch_is_blocked(self):
        a = self.audit(); a['summary']['version'] = 2
        with self.assertRaises(ValueError):
            compare(self.audit(), a)

    def test_unverified_artifacts_are_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d)
            (p / 'summary.json').write_text(json.dumps(dict(version=1, read_only=True, output_hashes={'quests.csv': 'bad'})))
            (p / 'quests.csv').write_text('quest\n1\n')
            with self.assertRaisesRegex(ValueError, 'hash mismatch'):
                load(p)


if __name__ == '__main__':
    unittest.main()
