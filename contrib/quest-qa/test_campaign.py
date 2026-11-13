import unittest
from campaign import scenarios,total_xp,digest,semantic_contract


class CampaignTests(unittest.TestCase):
    def test_resume_keeps_objectives_and_rewards_fixed(self):
        c=next(c for c in scenarios() if c.get('kill'))
        self.assertEqual(semantic_contract(c),semantic_contract({**c,'return_distance':999}))
        self.assertNotEqual(semantic_contract(c),semantic_contract({**c,'xp':0}))
        changed={**c,'kill':{**c['kill'],'count':1}}
        self.assertNotEqual(semantic_contract(c),semantic_contract(changed))

    def test_contracts_expand_only_valid_pairs_and_three_faction_states(self):
        cases=scenarios()
        self.assertEqual(len(cases),23)
        self.assertEqual({c['faction'] for c in cases},{'Alliance','Horde','Neutral'})
        self.assertFalse(any(c['class_id']==6 for c in cases))
        self.assertEqual(len({c['scenario'] for c in cases}),23)

    def test_xp_carry_does_not_lose_levelup_rewards(self):
        self.assertEqual(total_xp(['2','100','0'])-total_xp(['1','0','0']),500)
        with self.assertRaises(RuntimeError):total_xp(['90','0','0'])

    def test_localized_contract_change_changes_identity(self):
        c=scenarios()[0];changed={**c,'money':c['money']+1}
        self.assertNotEqual(digest(c),digest(changed))
        self.assertEqual(digest({'a':1,'b':2}),digest({'b':2,'a':1}))


if __name__=='__main__':unittest.main()
