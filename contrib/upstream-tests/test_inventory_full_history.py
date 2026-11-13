import unittest
from inventory_full_history import canonical, blobs


class EvidenceTests(unittest.TestCase):
    def test_sql_layout_is_ignored(self):
        self.assertEqual(canonical('UPDATE `x` SET a = 1'), canonical('update `x`\n set a=1'))

    def test_text_case_and_spacing_remain_significant(self):
        self.assertNotEqual(canonical("SET a='Hello  World'"), canonical("SET a='hello World'"))

    def test_quoted_escape_remains_significant(self):
        self.assertNotEqual(canonical(r"SET a='Bob\'s'"), canonical("SET a='Bobs'"))

    def test_no_blob_request_is_nonblocking(self):
        self.assertEqual(blobs([]), {})


if __name__ == '__main__':
    unittest.main()
