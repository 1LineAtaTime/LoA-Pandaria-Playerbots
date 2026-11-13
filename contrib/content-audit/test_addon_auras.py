import unittest
from addon_auras import inspect


class AuraTests(unittest.TestCase):
    def test_spaces_are_not_spell_zero(self):
        r = inspect(' 29266  7 ', {29266,7})
        self.assertTrue(r['formatting_differs']); self.assertFalse(r['findings'])

    def test_explicit_zero_is_an_error(self):
        self.assertEqual(inspect('0', {0})['findings'][0]['issue'], 'invalid_spell_token')

    def test_partial_numeric_token_is_rejected(self):
        self.assertEqual(inspect('7garbage', {7})['findings'][0]['issue'], 'invalid_spell_token')

    def test_missing_spell_is_attributed(self):
        self.assertEqual(inspect('99', {7})['findings'], [dict(issue='missing_spell_definition',spell=99)])

    def test_large_list_does_not_wrap_at_uint8(self):
        self.assertFalse(inspect(' '.join(map(str,range(1,301))), set(range(1,301)))['findings'])

    def test_uint32_boundary_and_sign(self):
        for token in ['-1','4294967296','+7']:
            self.assertEqual(inspect(token,{7})['findings'][0]['issue'],'invalid_spell_token')


if __name__ == '__main__':
    unittest.main()
