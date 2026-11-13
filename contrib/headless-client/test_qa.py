import unittest,tempfile,json
from pathlib import Path
from qa_spec import load_suite
from qa_report import summarize
from qa_compare import compare

class CoverageTests(unittest.TestCase):
 def test_lost_passing_coverage_is_regression(self):
  b=dict(suite='s',gameplay=[dict(id='q',status='passed')],data_audits=[])
  a=dict(suite='s',gameplay=[dict(id='q',status='untested')],data_audits=[])
  self.assertFalse(compare(b,a)['regression_free'])
 def test_incompatible_baseline_rejected(self):
  with self.assertRaises(ValueError):compare({'suite':'a'},{'suite':'b'})
 def test_unknown_mechanics_cannot_make_release_ready(self):
  r=summarize([{'status':'passed'},{'status':'untested'}]);self.assertFalse(r['ready']);self.assertFalse(r['complete'])
 def test_blocked_is_not_a_pass(self):
  self.assertFalse(summarize([{'status':'blocked'}])['ready'])
 def test_empty_is_not_success(self):
  self.assertFalse(summarize([])['ready'])
 def test_failed_completed_run_is_not_ready(self):
  r=summarize([{'status':'failed'}]);self.assertTrue(r['complete']);self.assertFalse(r['ready'])
 def test_invalid_result_is_rejected(self):
  with self.assertRaises(ValueError):summarize([{'status':'maybe'}])
 def check_invalid(self,cases):
  with tempfile.TemporaryDirectory() as d:
   p=Path(d)/'suite.json';p.write_text(json.dumps({'version':1,'scenarios':cases}))
   with self.assertRaises(ValueError):load_suite(p)
 def test_duplicate_id_rejected_before_mutation(self):
  c={'id':'a','mode':'create','race':1,'class_id':1};self.check_invalid([c,c])
 def test_reward_cannot_be_inferred_for_delivery_pass(self):
  self.check_invalid([{'id':'a','mode':'delivery','race':1,'class_id':1,'quest':1}])
 def test_gossip_fixture_requires_typed_target_and_item(self):
  self.check_invalid([{'id':'g','mode':'gossip_item','race':1,'class_id':8,'quest':29193,'npc':'15192;DELETE'}])
 def test_nonquest_mode_cannot_smuggle_quest_identifier(self):
  self.check_invalid([{'id':'a','mode':'create','race':1,'class_id':1,'quest':'1; DELETE'}])
 def test_sql_shaped_quest_is_rejected(self):
  self.check_invalid([{'id':'a','mode':'accept','race':1,'class_id':1,'quest':'1; DELETE'}])
 def test_unknown_scenario_requires_gap_reason(self):
  self.check_invalid([{'id':'a','mode':'boss'}])
 def test_invalid_accept_probe_must_fit_packet(self):
  for value in ('1;DELETE',-1,4294967296,True):
   self.check_invalid([{'id':'a','mode':'invalid_accept','race':1,'class_id':1,'request_quest':value}])
 def test_invalid_accept_suite_validates(self):
  s=load_suite(Path(__file__).with_name('qa-invalid-quest-scenarios.json'))
  self.assertEqual(len(s['scenarios']),3)
if __name__=='__main__':unittest.main()
