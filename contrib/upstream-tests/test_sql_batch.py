import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from assemble_gilneas_batch import assemble
from sql_batch import statements, comma_values


class MigrationAssemblyTests(unittest.TestCase):
    def test_quoted_delimiters(self):
        result=statements("-- header\nINSERT INTO t VALUES (1,'it''s; -- text, yes'); /* note */ UPDATE t SET n=2;")
        self.assertEqual(len(result),2)
        self.assertIn("'it''s; -- text, yes'",result[0])
        self.assertEqual(comma_values("1,'it''s, text',NULL"),['1',"'it''s, text'",'NULL'])

    def test_unterminated_quote_fails(self):
        with self.assertRaises(ValueError):
            statements("INSERT INTO t VALUES ('broken);")

    def test_inline_comment_leaves_no_trailing_spaces(self):
        result=statements("INSERT INTO t VALUES (1,    -- inline note\n 'literal  ');")
        self.assertTrue(all(line==line.rstrip(' \t') for line in result[0].splitlines()))
        self.assertIn("'literal  '",result[0])

    def test_identity_adaptation_and_source_pin(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)
            source="INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`) VALUES (123,1,0,'it''s; -- text, yes');"
            path=root/'source.sql';path.write_text(source)
            manifest={'sources':[{'path':'source.sql','sha256_lf':hashlib.sha256(source.encode()).hexdigest()}]}
            generated=assemble(root,manifest)
            self.assertIn('(`entry`,`CreatureID`,`GroupID`,`ID`,`Text`)',generated)
            self.assertIn("(123,123,1,0,'it''s; -- text, yes')",generated)
            path.write_text(source+'\nUPDATE t SET n=4;')
            with self.assertRaises(ValueError):
                assemble(root,manifest)

    def test_committed_batch_reproduces(self):
        root=Path(__file__).resolve().parents[2]
        manifest=json.loads((root/'contrib/upstream-tests/batches/gilneas-20261105.json').read_text())
        self.assertEqual(assemble(root,manifest),(root/'sql/updates/world/2026_11_05_30_world_gilneas_progression.sql').read_text())


if __name__=='__main__':
    unittest.main()
