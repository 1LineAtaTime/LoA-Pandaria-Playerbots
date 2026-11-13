import tempfile,struct,unittest
from unittest.mock import patch
from types import SimpleNamespace
from pathlib import Path
from catalogue import Table,sql_parts,sql_text,World

class CatalogueTests(unittest.TestCase):
 def test_database_preserves_empty_last_column(self):
  world=World.__new__(World);world.cmd=['unused'];world.password='fixture'
  with patch('catalogue.subprocess.run',return_value=SimpleNamespace(returncode=0,stdout='42\t\n__CONTENT_CATALOGUE_DONE__\n')):
   self.assertEqual(world.q('SELECT entry,ScriptName FROM creature_template'),[['42','']])
 def table(self,data):
  d=tempfile.TemporaryDirectory();self.addCleanup(d.cleanup);p=Path(d.name)/'test.dbc';p.write_bytes(data);return Table(p)
 def test_dbc_names_and_ids(self):
  t=self.table(struct.pack('<4s4I',b'WDBC',1,2,8,6)+struct.pack('<2I',42,1)+b'\0Name\0');self.assertEqual(t.labels(1),{42:'Name'})
 def test_wdb2_index_is_not_mistaken_for_records(self):
  b=struct.pack('<4s11I',b'WDB2',1,2,8,6,0,18273,0,17,17,1,0)+b'\0'*6+struct.pack('<2I',17,1)+b'\0Name\0'
  self.assertEqual(self.table(b).labels(1),{17:'Name'})
 def test_truncated_record_rejected(self):
  with self.assertRaises(ValueError):self.table(struct.pack('<4s4I',b'WDBC',2,2,8,1)+b'\0')
 def test_invalid_string_pointer_rejected(self):
  t=self.table(struct.pack('<4s4I',b'WDBC',1,2,8,1)+struct.pack('<2I',1,999)+b'\0')
  with self.assertRaises(ValueError):t.labels(1)
 def test_duplicate_ids_rejected(self):
  t=self.table(struct.pack('<4s4I',b'WDBC',2,1,4,1)+struct.pack('<2I',5,5)+b'\0')
  with self.assertRaises(ValueError):t.labels()
 def test_sql_quote_and_comma_boundaries(self):
  rows=list(sql_parts("(1,'A\\'s, B'),(2,'C')"));self.assertEqual(len(rows),2);self.assertEqual(list(sql_parts(rows[0][1:-1])),['1',"'A\\'s, B'"])
 def test_sql_label_unescaping(self):
  self.assertEqual(sql_text("'A\\'s'"),"A's")
 def test_stacked_statement_rejected(self):
  with self.assertRaises(ValueError):World.__new__(World).q('SELECT 1; DELETE FROM quest_template')
 def test_file_write_select_rejected(self):
  with self.assertRaises(ValueError):World.__new__(World).q("SELECT 1 INTO OUTFILE 'x'")
 def test_database_mutations_rejected(self):
  with self.assertRaises(ValueError):World.__new__(World).q('DELETE FROM quest_template')
if __name__=='__main__':unittest.main()
