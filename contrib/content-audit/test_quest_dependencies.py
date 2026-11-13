import unittest
from quest_dependencies import analyse


class DependenciesTests(unittest.TestCase):
    def run_graph(self, addons=None, objectives=None, relations=None, items=None, quests=None):
        quests = quests if quests is not None else {1: dict(LogTitle='Fixture', MinLevel=1,
                   QuestLevel=1, QuestSortID=0, AllowableRaces=0, PrevQuestId=999)}
        stores = {k: set() for k in ['quest', 'creature', 'gameobject', 'item', 'spell',
                                     'currency', 'faction', 'skill', 'title', 'mail_template']}
        stores['quest'] = set(quests)
        stores['creature'] = {42}
        return analyse(quests, addons or {}, objectives or [], relations or [],
                       items or [], stores, {})

    def test_legacy_alias_does_not_override_loader(self):
        edges, findings, _ = self.run_graph({1: {'PrevQuestID': 0}})
        self.assertEqual(edges, [])
        self.assertEqual(findings, [])

    def test_signed_previous_is_reference_to_absolute_id(self):
        edges, findings, _ = self.run_graph({1: {'PrevQuestID': -1}})
        self.assertEqual((edges[0]['target_id'], edges[0]['signed_value']), (1, -1))
        self.assertFalse(findings)

    def test_exclusive_group_is_not_quest_id(self):
        edges, findings, _ = self.run_graph({1: {'ExclusiveGroup': 99999}})
        self.assertFalse(edges)
        self.assertFalse(findings)

    def test_missing_objective_has_quest_attribution(self):
        _, findings, _ = self.run_graph(objectives=[dict(questId=1, id=8, type=1, objectId=99, amount=2)])
        self.assertEqual(findings[0]['quest'], 1)
        self.assertEqual(findings[0]['target_id'], 99)
        self.assertEqual(findings[0]['status'], 'structural_failure')

    def test_summoned_actor_without_spawn_is_not_failure(self):
        edges, findings, _ = self.run_graph(relations=[('creature_queststarter', 'creature', 'giver', [(42, 1)])])
        self.assertFalse(findings)
        self.assertTrue(edges[0]['target_present'])

    def test_orphan_relation_detected(self):
        _, findings, _ = self.run_graph(relations=[('creature_queststarter', 'creature', 'giver', [(42, 99)])])
        self.assertEqual(findings[0]['issue'], 'orphan_quest_reference')

    def test_pet_objective_not_misread_as_creature_entry(self):
        _, findings, summaries = self.run_graph(objectives=[dict(questId=1, id=8, type=12, objectId=99, amount=1)])
        self.assertFalse(findings)
        self.assertEqual(summaries[0]['unmodelled_objectives'], 1)

    def test_empty_givers_not_gameplay_failure(self):
        _, findings, summaries = self.run_graph()
        self.assertFalse(findings)
        self.assertEqual(summaries[0]['giver_links'], 0)
        self.assertEqual(summaries[0]['behavior'], 'untested_by_this_audit')

    def test_client_spell_absence_is_review(self):
        _, findings, _ = self.run_graph({1: {'SourceSpellID': 999}})
        self.assertEqual(findings[0]['status'], 'client_store_review')

    def test_unregistered_giver_requires_positive_log_evidence(self):
        quests = {1: dict(LogTitle='Fixture', MinLevel=1, QuestLevel=1, QuestSortID=0, AllowableRaces=0)}
        relations = [('creature_queststarter', 'creature', 'giver', [(42, 1)])]
        _, findings, _ = analyse(quests, {}, [], relations, [], {'creature': {42}}, {},
                                  {'creature': {42: 'npc_fixture'}}, {'npc_fixture'})
        self.assertEqual(findings[0]['status'], 'native_binding_review')

    def test_item_starts_missing_quest(self):
        _, findings, _ = self.run_graph(items=[(20, 99)])
        self.assertIn('orphan_quest_reference', [f['issue'] for f in findings])

    def test_server_spell_override_is_present(self):
        q = {1: dict(LogTitle='Fixture', MinLevel=1, QuestLevel=1, QuestSortID=0, AllowableRaces=0, RewardSpell=81040)}
        _, findings, _ = analyse(q, {}, [], [], [], {'spell': {81040}}, {})
        self.assertFalse(findings)


if __name__ == '__main__':
    unittest.main()
