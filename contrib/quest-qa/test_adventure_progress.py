import copy
import unittest
import json
import hashlib
import tempfile
from types import SimpleNamespace
from pathlib import Path
from unittest.mock import patch, Mock
from adventure_progress import ProgressWatchdog
from adventure_campaign import summarize_result, world_pid, validate_runtime


def state():
    return dict(character=[['3','10','100','1','0','0','0','100','0']],
                quests=[['25126','3']], objectives=[['256287','2']], rewarded=[], equipment=[])


class NativeProgressTests(unittest.TestCase):
    def test_unattended_preflight_rejects_foreign_auth_database(self):
        with tempfile.TemporaryDirectory() as d:
            runtime=Path(d);binary=b'owned QA fixture'
            (runtime/'worldserver.exe').write_bytes(binary)
            (runtime/'worldserver.conf').write_text('BindIP = "127.0.0.1"\nWorldServerPort = 8086\n')
            auth=runtime/'authserver.conf'
            auth.write_text('BindIP = "127.0.0.1"\nRealmServerPort = 3725\nLoginDatabaseInfo = "host;3306;user;private;mop_loa_qa_auth"\n')
            pool=SimpleNamespace(runtime=runtime,environment={'worldserver_sha256':hashlib.sha256(binary).hexdigest()},
                db=SimpleNamespace(connections={'Login':['host','3306','user','private','mop_loa_qa_auth']}))
            validate_runtime(pool)
            auth.write_text(auth.read_text().replace('mop_loa_qa_auth','production_auth'))
            with self.assertRaises(RuntimeError):validate_runtime(pool)

    def test_episode_success_never_claims_full_content_and_failure_is_reported(self):
        self.assertEqual(summarize_result({'reconnect_verified':True},[])[0],'EXECUTED')
        self.assertEqual(summarize_result({'reconnect_verified':False},[])[0],'BLOCKED')
        self.assertEqual(summarize_result({'reconnect_verified':True},[{'reason':'stationary'}])[0],'NEEDS_REVIEW')
        status,reason=summarize_result({'error':'socket closed'},[],server_alive=False)
        self.assertEqual(status,'BLOCKED')
        self.assertIn('worldserver exited',reason)

    def test_production_process_is_never_selected_as_qa(self):
        with tempfile.TemporaryDirectory() as d:
            runtime=Path(d)/'qa'
            with patch('adventure_campaign.subprocess.run',return_value=Mock(stdout=json.dumps([
                {'ProcessId':50,'ExecutablePath':str(Path(d)/'exports/worldserver.exe')},
                {'ProcessId':60,'ExecutablePath':str(runtime/'worldserver.exe')}]))) as call:
                self.assertEqual(world_pid(runtime),60)
            with patch('adventure_campaign.subprocess.run',return_value=Mock(stdout=json.dumps(
                {'ProcessId':50,'ExecutablePath':str(Path(d)/'exports/worldserver.exe')}))):
                with self.assertRaises(RuntimeError):world_pid(runtime)

    def test_travel_and_combat_are_not_stationary_failures(self):
        w=ProgressWatchdog();s=state();w.observe(s,0)
        for t in range(15,301,15):
            s=copy.deepcopy(s);s['character'][0][4]=str(t)
            self.assertEqual(w.observe(s,t,in_combat=t>200),[])
        self.assertEqual(w.observe(s,601,in_combat=True)[0]['reason'],'active_without_progress')

    def test_real_reward_and_equipment_progress_reset_stall(self):
        w=ProgressWatchdog();s=state();w.observe(s,0)
        self.assertEqual(w.observe(s,181)[0]['reason'],'stationary_without_progress')
        self.assertEqual(w.observe(s,190),[])
        earned=copy.deepcopy(s);earned['quests']=[];earned['rewarded']=[['25126']]
        earned['equipment']=[['9','53360','6827']]
        events=w.observe(earned,195)
        self.assertEqual([e['event'] for e in events],['quest_rewarded','equipment_changed'])
        self.assertEqual(w.observe(earned,200),[])

    def test_death_recovery_is_separate_and_not_quest_failure(self):
        w=ProgressWatchdog();s=state();w.observe(s,0)
        dead=copy.deepcopy(s);dead['character'][0][7]='0';w.observe(dead,15)
        event=w.observe(dead,200)[0]
        self.assertEqual(event['reason'],'recovery_needed')
        self.assertEqual(event['classification'],'unknown_or_insufficient_evidence')

    def test_new_objective_and_level_are_recorded(self):
        w=ProgressWatchdog();s=state();w.observe(s,0)
        new=copy.deepcopy(s);new['character'][0][0]='4';new['objectives'][0][1]='6'
        self.assertEqual([e['event'] for e in w.observe(new,15)],['level_changed','objective_changed'])


if __name__=='__main__':unittest.main()
