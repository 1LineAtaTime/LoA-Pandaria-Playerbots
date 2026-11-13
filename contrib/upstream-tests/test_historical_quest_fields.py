import unittest
from assemble_historical_quest_fields import decode, ids, mentioned_ids, narrows_authored_mask, source_updates, source_records, FIELDS, EXTENDED_FIELDS
from deploy_historical_quest_fields import prerequisite_cycles


class HistoricalQuestTests(unittest.TestCase):
    def test_explicit_identity_only(self):
        self.assertEqual(ids('`Id` IN (12, 34)'), [12,34])
        self.assertEqual(ids('Id=12 AND MinLevel=1'), [])

    def test_sql_strings_preserve_quest_tokens(self):
        self.assertEqual(decode(r"'Bob\'s\n$N'"), "Bob's\n$N")

    def test_where_inside_text_is_not_a_predicate(self):
        rows=list(source_updates("UPDATE quest_template SET Details=' WHERE Id=9' WHERE Id=12;"))
        self.assertEqual(rows,[(1,[12],['Details'])])

    def test_global_update_is_not_deployable(self):
        self.assertEqual(list(source_updates('UPDATE quest_template SET Level=1;'))[0][1],[])

    def test_historical_column_case_is_equivalent(self):
        row=list(source_updates('UPDATE quest_template SET PrevQuestID=1,NextQuestID=2 WHERE Id=3;'))[0]
        self.assertEqual(row[2],['PrevQuestId','NextQuestId'])

    def test_sensitive_scripts_flags_and_spells_not_mapped(self):
        for field in ('Flags','SpecialFlags','RewardSpell','RewardSpellCast','SourceSpellId','RequiredRaces','RequiredClasses'):
            self.assertNotIn(field,FIELDS)
        self.assertEqual(FIELDS['PrevQuestId'],('quest_template_addon','PrevQuestID'))

    def test_positive_prerequisite_cycles(self):
        self.assertEqual(prerequisite_cycles({1:0,2:1,3:2}),set())
        self.assertEqual(prerequisite_cycles({1:2,2:1,3:2}),{(1,2)})
        self.assertEqual(prerequisite_cycles({1:-2,2:1}),set())

    def test_conditional_quest_identity(self):
        self.assertEqual(mentioned_ids("`Id`=12 AND (OfferRewardText IS NULL OR OfferRewardText='')"),[12])
        self.assertEqual(mentioned_ids("Id='7509'"),[7509])
        self.assertEqual(mentioned_ids('(Id,RewardSpell) IN ((12,45),(34,67))'),[12,34])
        self.assertEqual(mentioned_ids("Details='Id=999' AND Id=12"),[12])

    def test_file_local_quest_variable(self):
        rows=list(source_records("SET @QUEST := 12; UPDATE quest_template SET Details='x' WHERE Id=@QUEST;",True))
        self.assertEqual(rows[0][1],[12])

    def test_modern_source_item_and_emote_storage(self):
        self.assertEqual(EXTENDED_FIELDS['RequiredSourceItemId2'],('quest_template','ItemDrop2'))
        self.assertEqual(EXTENDED_FIELDS['DetailsEmote1'],('quest_details','Emote1'))
        self.assertEqual(EXTENDED_FIELDS['RewardSpellCast'],('quest_template','RewardSpell'))

    def test_authored_class_race_access(self):
        self.assertTrue(narrows_authored_mask(512,10))
        self.assertTrue(narrows_authored_mask(8,128))
        self.assertFalse(narrows_authored_mask(0,512))
        self.assertFalse(narrows_authored_mask(8,520))
        self.assertFalse(narrows_authored_mask(8,0))


if __name__=='__main__':unittest.main()
