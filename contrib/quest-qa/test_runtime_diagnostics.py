import tempfile
import unittest
from pathlib import Path
from runtime_diagnostics import collect


class RuntimeDiagnosticsTests(unittest.TestCase):
    def test_preserves_repetitions_but_deduplicates_appender_copies(self):
        with tempfile.TemporaryDirectory() as folder:
            logs=Path(folder)/'logs';logs.mkdir()
            line='2026-11-10_16:00:00 ERROR [scripts] missing binding\n'
            (logs/'Errors.txt').write_text(line*2)
            (logs/'Server.log').write_text(line*2+'2026-11-10_16:00:01 WARN [maps] bad tile\n')
            result=collect(folder)
            self.assertEqual(result['unique_signatures'],2)
            error=next(r for r in result['findings'] if r['severity']=='ERROR')
            self.assertEqual(error['occurrences'],2)
            self.assertEqual(set(error['files']),{'Errors.txt','Server.log'})

    def test_history_and_info_are_not_current_diagnostics(self):
        with tempfile.TemporaryDirectory() as folder:
            logs=Path(folder)/'logs';logs.mkdir()
            (logs/'Server.log').write_text('2026-11-10_16:00:00 INFO [server] ready\n')
            (logs/'Errors.txt.old').write_text('2026-11-09_16:00:00 ERROR [server] old crash\n')
            self.assertEqual(collect(folder)['unique_signatures'],0)

    def test_absent_logs_are_empty_not_an_exception(self):
        with tempfile.TemporaryDirectory() as folder:
            self.assertEqual(collect(folder)['findings'],[])


if __name__=='__main__':unittest.main()
