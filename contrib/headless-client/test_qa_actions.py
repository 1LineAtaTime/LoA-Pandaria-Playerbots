import copy
import json
import struct
import unittest
from pathlib import Path
from unittest.mock import Mock
from qa_actions import validate_workflow, ObservedWorld, Workflow, read_values, UNIT_HEALTH
import headless
from qa_spec import load_suite


class ActionTests(unittest.TestCase):
    def test_stationary_gameobject_create_keeps_native_yzox_position(self):
        bits=headless.BitWriter()
        for _ in range(5):bits.bit(0)
        bits.bits(0,22)  # transport stop frames
        for _ in range(14):bits.bit(0)
        bits.bit(1)  # UPDATEFLAG_STATIONARY_POSITION
        movement=bits.flush()+struct.pack('<ffff',1269.86,-85.7466,1.5,-2060.72)
        reader=headless.MvReader(movement)
        decoded=headless.mv_read_create_movement(reader)
        for actual,expected in zip(decoded['pos'],(-2060.72,1269.86,-85.7466)):
            self.assertAlmostEqual(actual,expected,places=3)

    def test_spellbook_and_zero_coinage_are_observed(self):
        client=Mock();client.char_guid=1;tracker=ObservedWorld(client)
        bits=headless.BitWriter();bits.bit(0);bits.bits(2,22)
        tracker.handle(0x045a,bits.flush()+struct.pack('<II',78,34428),0)
        self.assertEqual(tracker.known_spells,{78,34428})
        tracker.observe_player_values(1,{})
        self.assertEqual(tracker.coinage,0)
        tracker.observe_player_values(1,{0x47d:4})
        self.assertEqual(tracker.coinage,4)

    def test_vehicle_questgiver_is_a_creature_actor(self):
        tracker=ObservedWorld(Mock());guid=(0xF15<<52)|(53566<<32)|1
        tracker.visible={guid:(1,2,3)};tracker.health={guid:50}
        self.assertEqual(tracker.actors(53566),[(guid,(1,2,3))])
        self.assertEqual(tracker.actors(53566,alive=True),[(guid,(1,2,3))])

    def test_unit_health_value_mask_is_decoded(self):
        raw=struct.pack('<BIII B',2,0,2,42,0)
        self.assertEqual(read_values(headless.MvReader(raw)),{UNIT_HEALTH:42})

    def test_combat_candidates_exclude_dead_and_unknown_health(self):
        tracker=ObservedWorld(Mock());a=(0xF13<<52)|(42<<32)|1;b=a+1
        tracker.visible={a:(1,2,3),b:(4,5,6)};tracker.health={a:0,b:10}
        self.assertEqual(tracker.actors(42,alive=True),[(b,(4,5,6))])
    def test_quest_details_counts_as_npc_interaction_response(self):
        client=Mock();client.auto=lambda *a:None
        db=Mock();db.q.return_value=[['3']]
        result={};wf=Workflow(client,db,1,dict(quest=2,steps=[dict(op='gossip')]),result,lambda *a:None)
        wf.selected=42;wf.tracker.handle=lambda *a:None
        client.gossip_hello.side_effect=lambda *a:client.auto(0x134C,b'')
        wf.execute()
        self.assertEqual(result['actions'][0]['response_opcodes'],['0x134c'])

    def test_quest_only_npc_uses_questgiver_hello(self):
        client=Mock();client.auto=lambda *a:None;db=Mock();db.q.return_value=[['2']]
        wf=Workflow(client,db,1,dict(quest=2,steps=[dict(op='gossip')]),{},lambda *a:None)
        wf.selected=42;wf.tracker.handle=lambda *a:None
        client.quest_hello.side_effect=lambda *a:client.auto(0x134C,b'')
        wf.execute();client.quest_hello.assert_called_once_with(42);client.gossip_hello.assert_not_called()
    def case(self):
        return dict(steps=[dict(op='accept')],postconditions=dict(quest_state='active'))

    def test_unknown_operation_rejected(self):
        c=self.case();c['steps']=[dict(op='gm_complete')]
        with self.assertRaises(ValueError):validate_workflow(c)

    def test_sql_shaped_item_rejected(self):
        c=self.case();c['steps']=[dict(op='use_item',item='1;DELETE')]
        with self.assertRaises(ValueError):validate_workflow(c)

    def test_unbounded_or_nonfinite_timeout_rejected(self):
        for value in (0,601,float('nan'),float('inf'),True):
            c=self.case();c['steps']=[dict(op='kill',entry=1,count=1,timeout=value)]
            with self.assertRaises(ValueError):validate_workflow(c)

    def test_long_combat_is_bounded_without_extending_waits(self):
        c=self.case();c['steps']=[dict(op='kill',entry=1,count=6,timeout=600)]
        validate_workflow(c)
        c['steps']=[dict(op='wait_progress',entry=1,count=1,timeout=181)]
        with self.assertRaises(ValueError):validate_workflow(c)

    def test_delivery_requires_independent_reward(self):
        c=self.case();c['steps']=[dict(op='turn_in')];c['postconditions']['quest_state']='rewarded'
        with self.assertRaises(ValueError):validate_workflow(c)

    def test_empty_postconditions_rejected(self):
        c=self.case();c['postconditions']={}
        with self.assertRaises(ValueError):validate_workflow(c)

    def test_unknown_field_cannot_smuggle_sql(self):
        c=self.case();c['steps']=[dict(op='accept',sql='UPDATE characters')]
        with self.assertRaises(ValueError):validate_workflow(c)

    def test_new_suite_validates(self):
        suite=load_suite(Path(__file__).with_name('qa-workflow-scenarios.json'))
        self.assertEqual(len(suite['scenarios']),3)

    def test_creature_and_object_namespaces_separate(self):
        tracker=ObservedWorld(Mock())
        npc=(0xF13<<52)|(42<<32)|1;obj=(0xF11<<52)|(42<<32)|2
        tracker.visible={npc:(1,2,3),obj:(3,2,1)}
        self.assertEqual(tracker.actors(42),[(npc,(1,2,3))])
        self.assertEqual(tracker.actors(42,'gameobject'),[(obj,(3,2,1))])

    def test_packet_credit_belongs_to_test_quest(self):
        client=Mock();client.auto=lambda *a:None
        result={};wf=Workflow(client,Mock(),1,dict(quest=2),result,lambda *a:None)
        wf.tracker.handle=lambda *a:None
        client.auto(0x1645,struct.pack('<HBIHI',1,0,3,6,42))
        self.assertFalse(wf.progress)
        client.auto(0x1645,struct.pack('<HBIHI',1,0,2,6,42))
        self.assertEqual(wf.progress,{42:1})

    def test_disappearance_requires_prior_observation(self):
        client=Mock();client.auto=lambda *a:None
        wf=Workflow(client,Mock(),1,dict(quest=2),{},lambda *a:None)
        with self.assertRaisesRegex(TimeoutError,'never seen'):
            wf.wait(dict(op='wait_visible',entry=42,present=False,timeout=1))

    def test_decoder_errors_cannot_prove_absence(self):
        client=Mock();client.auto=lambda *a:None
        wf=Workflow(client,Mock(),1,dict(quest=2),{},lambda *a:None)
        wf.tracker.ever_seen={(0xF13<<52)|(42<<32)|1};wf.tracker.decode_errors=1
        with self.assertRaisesRegex(TimeoutError,'incomplete'):
            wf.wait(dict(op='wait_visible',entry=42,present=False,timeout=1))


if __name__=='__main__':unittest.main()
